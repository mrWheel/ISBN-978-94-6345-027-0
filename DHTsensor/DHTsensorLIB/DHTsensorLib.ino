/*
    Program  : DHTsensorLib v4
  
    Copyright (C) 2016 Willem Aandewiel

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

// -- DEBUG needs to be defined before "mySensors.h"!! 
#define DEBUG false
#include "mySensors.h"

#include "DHT.h"
#include "GSPprotocol.h"
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <avr/power.h>
#include <avr/eeprom.h>

#if defined(__AVR_ATtiny84__) 
// ATMEL ATTINY84
//                    +--\/--+
//               VCC 1|      |14 GND
//   [LED] (D10) PB0 2|      |13 AREF (D0)
//          (D9) PB1 3|      |12 PA1  (D1) [TX]->
//         RESET PB3 4|      |11 PA2  (D2)
//     INT0 (D8) PB2 5|      |10 PA3  (D3) [DHT]<->
//      PWM (D7) PA7 6|      |9  PA4  (D4) 
//      PWM (D6) PA6 7|      |8  PA5  (D5) PWM
//                    +------+
    //-- PA1 = DIL-12 = Arduino IDE D1
    #define TXDDR        DDRA
    #define TXPORT      PORTA
    #define TXPORT_BIT      1   // DIL-12
    //-- PA3 = DIL-10 = Arduino IDE D3
    #define DHTDDR       DDRB
    #define DHTPORT     PORTB
    #define DHTPORT_BIT     3   // DIL-10
    #define DHTIDE_PIN      3   // IDE D3
    //-- PB0 = DIL-2 = Arduino IDE D10
    #define LEDDDR       DDRB
    #define LEDPORT     PORTB
    #define LEDPORT_BIT     0   // DIL-2
    #define LEDIDE_PIN     10   // IDE D10
#elif defined(__AVR_ATtiny85__)
// ATMEL ATTINY85
//                    +--\/--+
//         RESET PB5 1|      |8 VCC
// <->[DHT] (D3) PB3 2|      |7 PB2 (D2)
//    [LED] (D4) PB4 3|      |6 PB1 (D1) [TX]->
//               GND 4|      |5 PB0 (D0)
//                    +------+
    //-- PB1 = DIL-6 = Arduino IDE D1
    #define TXDDR        DDRB
    #define TXPORT      PORTB
    #define TXPORT_BIT      1   // DIL-6
    //-- PB3 = DIL-2 = Arduino IDE D3
    #define DHTDDR       DDRB
    #define DHTPORT     PORTB
    #define DHTIDE_PIN      3   // IDE D3
    #define DHTPORT_BIT     3   // DIL-2
    //-- PB4 = DIL-3 = Arduino IDE D4
    #define LEDDDR       DDRB
    #define LEDPORT     PORTB
    #define LEDIDE_PIN      4   // IDE D4
    #define LEDPORT_BIT     4   // DIL-3
#else
    //-- PB2 = DIL-16 = Arduino IDE D10
    #define TXDDR        DDRB
    #define TXPORT      PORTB
    #define TXPORT_BIT      2   // UNO/Trinket-Pro D10
    //-- PD4 = DIL-6 = Arduino IDE 4
    #define DHTDDR       DDRD
    #define DHTPORT     PORTD
    #define DHTIDE_PIN      4   // IDE D4
    #define DHTPORT_BIT     4   // UNO/Trinket-Pro D4
    //-- PB5 = DIL-19 = Arduino IDE D13
    #define LEDDDR       DDRB
    #define LEDPORT     PORTB
    #define LEDIDE_PIN     13   // IDE D13
    #define LEDPORT_BIT     5   // UNO/Trinket-Pro D13
#endif

// Uncomment whatever type you're using!
#define DHTTYPE DHT11     // DHT 11
//#define DHTTYPE DHT22   // DHT 22 (AM2302, AM2321)
//#define DHTTYPE DHT21   // DHT 21 (AM2301)

DHT dht(DHTIDE_PIN, DHTTYPE);
GSPprotocol GSP(&TXPORT, TXPORT_BIT);

static SensorConfig EEPROM;

int     statusBat   = 1;
long    VccBat      = 0;

static float Temperature;
static float Humidity;

void readDHTsensor() {
    SET_OUTPUT(DHTPORT, DHTPORT_BIT);
    dht.begin();
    delay(50);  // expiriment for differend sensors
    // Read temperature as Celsius (the default)
    Temperature = dht.readTemperature();
    // Read humidity
    Humidity    = dht.readHumidity();
    if (isnan(Temperature)) {
        Sprintln(F("Failed to read Temperature!"));
        Temperature = 0;
    }
    if (isnan(Humidity)) {
        Sprintln(F("Failed to read Humidity!"));
        Humidity    = 0;
    }
    SET_INPUT(DHTPORT, DHTPORT_BIT);
    
}   // readDHTsensor()


void prepareAndSend(uint8_t sensorID, uint8_t Unit
                        , uint8_t statusBat, int32_t Value
                        , int8_t Decimals, char *Label) {
    SET_OUTPUT(TXDDR, TXPORT_BIT);
    GSP.send(sensorID, Unit, statusBat, Value, Decimals, Label);
    SET_INPUT(TXDDR, TXPORT_BIT);
            
}   // prepareAndSend()


void setup() {
    Sbegin(19200);
    delay(100);
    SET_OUTPUT(TXDDR, TXPORT_BIT);
    loadConfig();
    Sprintln(F("\nRF433 DHTsensorLib"));

}   // setup()

void loop() {
    Sflush();

    readDHTsensor();    // Temperature and Humidity
    goToSleep(1);
    // -- turn LED on for visual feedback --
    SET_OUTPUT(LEDDDR, LEDPORT_BIT);
    SET(LEDPORT,LEDPORT_BIT);
    // --- read the voltage of the battery ---
    VccBat     = readVcc();
    // -- turn LED off --
    CLEAR(LEDPORT,LEDPORT_BIT);
    SET_INPUT(LEDDDR, LEDPORT_BIT);
    // -- 3200 is a arbitrary value. Do expiriment!
    if (VccBat < 3200)  statusBat = false;
    else                statusBat = true; 
    
    Sprint(F("Temperature: "));
    Sprint(Temperature);
    Sprint(F(" *C"));
    Sprintln();
    prepareAndSend(EEPROM.sensorID, 1, statusBat
                , (int)(Temperature * 100), 2, "*C");

  // Wait one second between transmissions.
    goToSleep(1);
    Sprint(F("Humidity: "));
    Sprint(Humidity);
    Sprint(F(" %"));
    Sprintln();
    prepareAndSend(EEPROM.sensorID, 2, statusBat
                        , (int32_t)Humidity, 0, "%");

  // Wait one second between transmissions.
    goToSleep(1);
    Sprint(F("Vcc Bat : "));
    Sprint(VccBat);
    Sprint(F(" mV"));
    Sprintln();
    prepareAndSend(EEPROM.sensorID, 7, statusBat
                        , (int32_t)VccBat, 0, "mV");

    if (statusBat)  goToSleep(EEPROM.interval);
    else            goToSleep(EEPROM.interval * 2);   
     
}   // loop()

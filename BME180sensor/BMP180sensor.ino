/*
    Program  : BMP180sensor v4
    
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

#define DEBUG   true
#include "mySensors.h"

#ifdef __AVR_ATtiny85__
    // TinyWireM and tinyBMP085 must be installed as library
    #include <TinyWireM.h>
    // https://github.com/cano64/
    //  ATTiny85-ATTiny84-BMP085-Arduino-Library-FastAltitude
    #include <tinyBMP085.h>
#else
    // Adafruit_BMP1085 must be installed as library
    #include <Wire.h>
    #include <Adafruit_BMP085.h>
#endif

#include <avr/sleep.h>
#include <avr/wdt.h>
#include <avr/power.h>
#include <avr/eeprom.h>
#include <GPSprotocol.h>

#if defined(__AVR_ATtiny84__) 
//                    +--\/--+
//               VCC 1|      |14 GND
//   [LED] (D10) PB0 2|      |13 AREF (D0)
//          (D9) PB1 3|      |12 PA1  (D1) [TX]->
//         RESET PB3 4|      |11 PA2  (D2)
//     INT0 (D8) PB2 5|      |10 PA3  (D3)
//      PWM (D7) PA7 6|      |9  PA4  (D4) [SCL]<->
// <->[SDA] (D6) PA6 7|      |8  PA5  (D5) PWM
//                    +------+
    //-- PA4 = SCL (clock) PA6 = SDA (data)!
    //-- PA1 = DIL-12 = Arduino IDE D1
    #define TXDDR        DDRA
    #define TXPORT      PORTA
    #define TXPORT_BIT      1   // DIL-12/D1
    //-- PB0 = DIL-2 = Arduino IDE D10
    #define LEDDDR       DDRB
    #define LEDPORT     PORTB
    #define LEDPORT_BIT     0   // DIL-2/D10
    #define LEDIDE_PIN     10   // IDE D2
#elif defined(__AVR_ATtiny85__)
//                    +--\/--+
//         RESET PB5 1|      |8 VCC
//          (D3) PB3 2|      |7 PB2 (D2) [SCL]<->
//    [LED] (D4) PB4 3|      |6 PB1 (D1) [TX]--->
//               GND 4|      |5 PB0 (D0) [SDA]<->
//                    +------+
    //-- PB2 = SCL (clock) PB0 = SDA (data)!
    //-- PB1 = DIL-6 = Arduino IDE D1
    #define TXDDR        DDRB
    #define TXPORT      PORTB
    #define TXPORT_BIT      1   // DIL-6
    //-- PB4 = DIL-3 = Arduino IDE D4
    #define LEDDDR       DDRB
    #define LEDPORT     PORTB
    #define LEDIDE_PIN      4   // IDE D4
    #define LEDPORT_BIT     4   // DIL-3
#else
    //-- ATMega328 ----------------------
    //-- A5 = SCL (clock) A4 = SDA (data)!
    //-- PB1 = Arduino IDE D9
    #define TXDDR        DDRB
    #define TXPORT      PORTB
    #define TXPORT_BIT      1   // DIL-15
    //-- PB5 = Arduino IDE D13
    #define LEDDDR       DDRB
    #define LEDPORT     PORTB
    #define LEDIDE_PIN     13   // IDE D13
    #define LEDPORT_BIT     5   // DIL-19
#endif

#if defined(__AVR_ATtiny84__) || defined(__AVR_ATtiny85__)
    tinyBMP085 BMP;
#else
     Adafruit_BMP085 BMP;
#endif
GPSprotocol GPS(&TXPORT, TXPORT_BIT);

static SensorConfig EEPROM;

int     statusBat   = 1;
long    VccBat      = 0;

static float Temperature;
static float Pressure;
static float Humidity;

void readBMP180sensor() {
#if defined(__AVR_ATtiny84__) || defined(__AVR_ATtiny85__)
    TinyWireM.begin();   // recover from deep-sleep
#endif
    BMP.begin();    // recover from deep-sleep
    delay(100);
#if defined(__AVR_ATtiny84__) || defined(__AVR_ATtiny85__)
    Temperature = (Temperature + BMP.readTemperature10C() / 10.0) / 2.0;
    Pressure    = (Pressure + BMP.readPressure() / 100.0) / 2.0;
#else
    Temperature = (float)(Temperature + BMP.readTemperature() / 1.0) / 2.0;
    Pressure    = (float)(Pressure + BMP.readPressure() / 10000.0) / 2.0;
#endif
    
}   // readBMP180sensor()


void prepareAndSend(uint8_t sensorID, uint8_t Unit
                    , uint8_t statusBat, int32_t Value
                    , int8_t Decimals, char *Label) {
    SET_OUTPUT(TXDDR, TXPORT_BIT);
    GPS.send(sensorID, Unit, statusBat, Value, Decimals, Label);
    SET_INPUT(TXDDR, TXPORT_BIT);
            
}   // prepareAndSend()


void setup() {
    Sbegin(19200);
    delay(100);
    SET_OUTPUT(LEDDDR, LEDPORT_BIT);
    SET_OUTPUT(TXDDR,  TXPORT_BIT);
    loadConfig();
#if defined(__AVR_ATtiny84__) || defined(__AVR_ATtiny85__)
    TinyWireM.begin();   // initialize I2C lib
#endif
    delay(100);
    if (!BMP.begin()) {
#if DEBUG
        Sprintln(F("Could not find a valid BMP180 "\
                    "sensor, check wiring!"));
#endif
        blinkLed(0);
    }
    blinkLed(5);
    readBMP180sensor(); // Temperature and Humidity
    Temperature *= 2;
    Pressure *= 2;
    CLEAR(LEDPORT, LEDPORT_BIT);
    Sprintln(F("\nRF433 BMP180sensor"));

}   // setup()


void loop() {
    Sflush();

    readBMP180sensor(); // Temperature and Humidity
    
    goToSleep(1);
    // --- read the voltage of the battery ---
    VccBat     = readVcc();
    // -- 3200 is a arbitrary value. Do expiriment!
    if (VccBat < 3200)  statusBat = false;
    else                statusBat = true; 
    // -- turn LED on for visual feedback --
    SET(LEDPORT,LEDPORT_BIT);

    Sprint(F("Temperature: "));
    Sprint(Temperature);
    Sprint(F(" *C"));
    Sprintln();
    prepareAndSend(EEPROM.sensorID, 1, statusBat
                , (int32_t)(Temperature * 100), 2, "*C");

    // -- turn LED off --
    CLEAR(LEDPORT,LEDPORT_BIT);

    // Wait one second between transmissions.
    goToSleep(10);
    Sprint(F("Pressure: "));
    Sprint((Pressure * 100));
    Sprint(F(" hPa"));
    Sprintln();
    prepareAndSend(EEPROM.sensorID, 2, statusBat
                , (int32_t)(Pressure * 100), 2, "hPa");

    // Wait one second between transmissions.
    goToSleep(10);
    Sprint(F("Vcc Bat : "));
    Sprint(VccBat);
    Sprint(F(" mV"));
    Sprintln();
    prepareAndSend(EEPROM.sensorID, 7, statusBat
                , (int32_t)VccBat, 0, "Vcc");

    // -- now go into deep-sleep ----
    if (statusBat)  goToSleep(EEPROM.interval);
    else            goToSleep(EEPROM.interval * 2);   
     
}   // loop()

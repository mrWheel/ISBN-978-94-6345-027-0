/*
    Program  : BME280sensor v3
  
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

#if ARDUINO >= 100
    #include "Arduino.h"
#else
    #include "WProgram.h"
#endif

// -- DEBUG must be defined before #include "mySensors.h"
#define DEBUG true

#if defined(__AVR_ATtiny84__) || defined(__AVR_ATtiny85__)
    // TinyWireM and tinyBME085 must be installed as library
    #include <TinyWireM.h>
    #define Wire TinyWireM
#endif
// https://github.com/sparkfun/
//    SparkFun_BME280_Arduino_Library
// --------------------------------------
// SparkFunBME280.cpp needs to be edited!
// --------------------------------------
#include <SparkFunBME280.h>

#include <avr/sleep.h>
#include <avr/wdt.h>
#include <avr/power.h>
#include <avr/eeprom.h>
#include "mySensors.h"
#include "GPSprotocol.h"

#if defined(__AVR_ATtiny84__) 
// ATMEL ATTINY84
//                    +--\/--+
//               VCC 1|      |14 GND
//   [LED] (D10) PB0 2|      |13 AREF (D0)
//          (D9) PB1 3|      |12 PA1  (D1) [TX]->
//         RESET PB3 4|      |11 PA2  (D2)
//     INT0 (D8) PB2 5|      |10 PA3  (D3)
//      PWM (D7) PA7 6|      |9  PA4  (D4) [SCK]<->
// <->[SDI] (D6) PA6 7|      |8  PA5  (D5) PWM
//                    +------+
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
// ATMEL ATTINY85
//                    +--\/--+
//         RESET PB5 1|      |8 VCC
//          (D3) PB3 2|      |7 PB2 (D2) [SCK]<->
//    [LED] (D4) PB4 3|      |6 PB1 (D1) [TX]--->
//               GND 4|      |5 PB0 (D0) [SDI]<->
//                    +------+
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
    #error Only for ATtiny84 or ATtiny85 AVR chips!!
#endif

//Global sensor object
BME280 myBME280;

GPSprotocol GPS(&TXPORT, TXPORT_BIT);

static SensorConfig EEPROM;

int     statusBat   = 1;
long    VccBat      = 0;

static float Temperature;
static float Pressure;
static float Humidity;


void setupBME280()
{
    //commInterface can only be I2C_MODE
    myBME280.settings.commInterface = I2C_MODE;
    myBME280.settings.I2CAddress    = 0x77;
    myBME280.settings.runMode  = 3; //  3, Normal mode
    myBME280.settings.tStandby = 0; //  0, 0.5ms
    myBME280.settings.filter   = 1; //  0, filter off
    //tempOverSample can be:
    //  0, skipped
    //  1 through 5, 
    //      oversampling *1, *2, *4, *8, *16 respectively
    myBME280.settings.tempOverSample = 2;
    //pressOverSample can be:
    //  0, skipped
    //  1 through 5, 
    //      oversampling *1, *2, *4, *8, *16 respectively
    myBME280.settings.pressOverSample = 2;
    //humidOverSample can be:
    //  0, skipped
    //  1 through 5, 
    //      oversampling *1, *2, *4, *8, *16 respectively
    myBME280.settings.humidOverSample = 1;

    //delay(30);
    uint8_t beginReply = myBME280.begin();
    /***
    Sprint("\nmyBME280.begin(): 0x");
    Sprintln(beginReply, HEX);
    if (beginReply == 0) {
        Sprintln("\nError initializing BME280. "\
                    "Check your wiring!\n");
        blinkLed(0);
    }
    ***/
}   // setupBME280()    
    

void readBME280sensor() {
    Temperature = (Temperature + myBME280.readTempC()) / 2.0;
  //Temperature = myBME280.readTempF();
    Pressure    = myBME280.readFloatPressure() / 100.0;
    Humidity    = myBME280.readFloatHumidity();
    
}   // readBME280sensor()


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
    delay(200);
#endif    
    setupBME280();
    //readBME280sensor();
    //Temperature *= 2;
    //Pressure    *= 2;
    //Humidity    *= 2;
    blinkLed(5);
    CLEAR(LEDPORT, LEDPORT_BIT);

}   // setup()


void loop() {
    Sflush();

    readBME280sensor(); // Temperature, Pressure and Humidity
    
    goToSleep(1);
    // --- read the voltage of the battery ---
    VccBat     = readVcc();
    // -- 3200 is a arbitrary value. Do expiriment!
    if (VccBat < 3200)  statusBat = false;
    else                statusBat = true; 
    
    // -- turn LED on for visual feedback --
    SET(LEDPORT,LEDPORT_BIT);

    Sprint(F("Temperature: "));
    Sprint(Temperature, 2);
    Sprint(F(" *C"));
    Sprintln();
    prepareAndSend(EEPROM.sensorID, 1, statusBat
                , (int32_t)(Temperature * 100), 2, "*C");

    // -- turn LED off --
    CLEAR(LEDPORT,LEDPORT_BIT);

    // Wait one second between transmissions.
    goToSleep(10);
    Sprint(F("Pressure: "));
    Sprint(Pressure, 2);
    Sprint(F(" hPa"));
    Sprintln();
    prepareAndSend(EEPROM.sensorID, 2, statusBat
                , (int32_t)(Pressure * 100), 2, "hPa");

    // Wait one second between transmissions.
    goToSleep(10);
    Sprint(F("Humidity: "));
    Sprint(Humidity, 2);
    Sprint(F(" %RH"));
    Sprintln();
    prepareAndSend(EEPROM.sensorID, 3, statusBat
                , (int32_t)(Humidity * 100), 2, "%RH");

    // Wait one second between transmissions.
    goToSleep(10);
    Sprint(F("Vcc Bat : "));
    Sprint(VccBat);
    Sprint(F(" mV"));
    Sprintln();
    prepareAndSend(EEPROM.sensorID, 7, statusBat
                , (int32_t)VccBat, 0, "mV");

    // -- now go into deep-sleep ----
    if (statusBat)  goToSleep(EEPROM.interval);
    else            goToSleep(EEPROM.interval * 2);   
     
}   // loop()

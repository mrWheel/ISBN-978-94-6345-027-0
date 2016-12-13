/*
    Program  : TSL2561sensor v1
  
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

#include "Arduino.h"
#define DEBUG   true
#include "mySensors.h"

/*
 * You have to download and install the "obsolete" TSL2561
 * library from github at:
 * https://github.com/adafruit/TSL2561-Arduino-Library
 * 
 * Modify in the file TSL2651.h the line that says:
 * #include <Wire.h>
 * 
 * Change it into:
 * //---- modified for use with ATtiny84/5
 * #if defined(__AVR_ATtiny84__) || defined(__AVR_ATtiny85__)
 *      #include <TinyWireM.h>
 *      #define  Wire    TinyWireM
 * #else
 *      #include <Wire.h>
 * #endif
 */
#include "TSL2561.h"
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
// [LDR] (A7/D7) PA7 6|      |9  PA4  (D4) [SCL]<->
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
    //-- PA7 = DIL-6 = Arduino IDE D7
    #define LDRDDR       DDRA
    #define LDRPORT     PORTA
    #define LDRPORT_BIT     7   // DIL-6/D7
    #define LDRIDE_PIN      7   // IDE D7
#elif defined(__AVR_ATtiny85__)
//                    +--\/--+
//         RESET PB5 1|      |8 VCC
// [LDR] (A3/D3) PB3 2|      |7 PB2 (D2) [SCL]<->
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
    //-- PA7 = DIL-6 = Arduino IDE D7
    #define LDRDDR       DDRA
    #define LDRPORT     PORTA
    #define LDRPORT_BIT     7   // DIL-6/D7
    #define LDRIDE_PIN      7   // IDE D7
#else
    //-- ATMega328 ----------------------
    //-- A5 = SCL (clock) A4 = SDA (data)!
    //-- PB1 = Arduino IDE D9
    #define TXDDR        DDRB
    #define TXPORT      PORTB
    #define TXPORT_BIT      1   // DIL-15
    //-- PB4 = Arduino IDE D12
    #define LEDDDR       DDRB
    #define LEDPORT     PORTB
    #define LEDIDE_PIN      2   // IDE D12
    #define LEDPORT_BIT     4   // DIL-18
    //-- A1 = DIL-24 = Arduino IDE A1
    #define LDRDDR       DDRC
    #define LDRPORT     PORTC
    #define LDRPORT_BIT     1   // DIL-24/A1
    #define LDRIDE_PIN      1   // IDE A1
#endif

TSL2561 TSL(TSL2561_ADDR_FLOAT); 

GPSprotocol GPS(&TXPORT, TXPORT_BIT);

static SensorConfig EEPROM;

int     statusBat   = 1;
long    VccBat      = 0;

static float visableLux;
int          ldrIn;

bool readTSL2561sensor() {
    if (!TSL.begin()) return false; 
    delay(100);
    // set no gain (for bright situtations)
    TSL.setGain(TSL2561_GAIN_0X);
    // medium integration time (medium light)
    TSL.setTiming(TSL2561_INTEGRATIONTIME_101MS);
    
    int16_t lux = TSL.getLuminosity(TSL2561_VISIBLE);
    uint32_t lum = TSL.getFullLuminosity();
    uint16_t ir, full;
    ir = lum >> 16;
    full = lum & 0xFFFF;
    Sprint("IR: ");      Sprint(ir);        Sprint("\t");
    Sprint("Full: ");    Sprint(full);      Sprint("\t");
    Sprint("Visible: "); Sprint(full - ir); Sprint("\t");
  
    visableLux = TSL.calculateLux(full, ir);
    
    return true;
    
}   // readTSL2561sensor()


void prepareAndSend(uint8_t sensorID, uint8_t Unit
                    , uint8_t statusBat, int32_t Value
                    , int8_t Decimals, char *Label) {
    SET_OUTPUT(TXDDR, TXPORT_BIT);
    GPS.send(sensorID, Unit, statusBat, Value, Decimals, Label);
    SET_INPUT(TXDDR, TXPORT_BIT);
            
}   // prepareAndSend()


void setup(void) {
    Sbegin(19200);

    SET_OUTPUT(LEDDDR, LEDPORT_BIT);
    SET_INPUT(LDRDDR,  LDRPORT_BIT);
    loadConfig();

    if (TSL.begin()) {
        Sprintln("Found sensor");
    } else {
        Sprintln("No sensor?");
        blinkLed(0);
    }
    blinkLed(4);
    
}   // setup()


void loop(void) {
    // --- read the voltage of the battery ---
    VccBat     = readVcc();
    // -- 3300 - 3600 are arbitrary values. Do expiriment!
    if (VccBat < 3500)  statusBat = 0;
    else                statusBat = 1; 
    
    if (readTSL2561sensor()) {
        Sprint(F("TSL2561: "));
        Sprint(visableLux);
        Sprint(F(" Lux"));
        prepareAndSend(EEPROM.sensorID, 1, statusBat
                    , (int32_t)(visableLux * 1), 0, "Lux");
    }
    else {
        statusBat   = 0;
        visableLux  = 0;
    }
    
    // Wait between transmissions.
    goToSleep(5);
    SET(LEDPORT, LEDPORT_BIT);
    delay(200);
    ldrIn = analogRead(LDRIDE_PIN);
    Sprint("\tLDR: "); 
    Sprint(ldrIn);
    
    prepareAndSend(EEPROM.sensorID, 5, statusBat
                , (int32_t)(ldrIn * 1), 0, "LDR");

    CLEAR(LEDPORT, LEDPORT_BIT);

    // Wait between transmissions.
    goToSleep(5);
    Sprint(F("\tVcc Bat : "));
    Sprint(VccBat);
    Sprint(F(" mV"));
    prepareAndSend(EEPROM.sensorID, 7, statusBat
                , (int32_t)VccBat, 0, "mV");

    Sprintln();
    Sflush();
    
    // -- now go into deep-sleep ----
    if (statusBat == 1) goToSleep(EEPROM.interval);
    else                goToSleep(EEPROM.interval * 2);
     
}   // loop()

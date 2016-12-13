/*
    Program  : DHTsensorBat v3
   
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
#define DEBUG false

#include "DHT.h"
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <avr/power.h>
#include <avr/eeprom.h>
#include "mySensors.h"


#if defined(__AVR_ATtiny84__) 
// ATMEL ATTINY84
//                    +--\/--+
//               VCC 1|      |14 GND
//         (D10) PB0 2|      |13 AREF (D0)
//          (D9) PB1 3|      |12 PA1  (D1) [TX]->
//         RESET PB3 4|      |11 PA2  (D2)
//     INT0 (D8) PB2 5|      |10 PA3  (D3) [DHT]<->
//      PWM (D7) PA7 6|      |9  PA4  (D4) 
//      PWM (D6) PA6 7|      |8  PA5  (D5) PWM
//                    +------+
    //-- PA1 = DIL-12 = Arduino IDE D1
    #define TXDDR        DDRA
    #define TXPORT      PORTA
    #define TXPORT_BIT      1   // DIL-12/D1
    //-- PA3 = DIL-10 = Arduino IDE D3
    #define DHTDDR       DDRB
    #define DHTPORT     PORTB
    #define DHTPORT_BIT     3   // DIL-10/D3
    #define DHTIDEPIN       3   // IDE D3
    //-- PB0 = DIL-2 = Arduino IDE D10
    #define LEDDDR       DDRB
    #define LEDPORT     PORTB
    #define LEDPORT_BIT     0   // DIL-2/D10
    #define LEDIDEPIN      10   // IDE D2
#elif defined(__AVR_ATtiny85__)
// ATMEL ATTINY85
//                    +--\/--+
//         RESET PB5 1|      |8 VCC
// <->[DHT] (D3) PB3 2|      |7 PB2 (D2)
//          (D4) PB4 3|      |6 PB1 (D1) [TX]->
//               GND 4|      |5 PB0 (D0)
//                    +------+
    //-- PB1 = DIL-6 = Arduino IDE D1
    #define TXDDR        DDRB
    #define TXPORT      PORTB
    #define TXPORT_BIT           1   // DIL-6/D1
    //-- PB3 = DIL-2 = Arduino IDE D3
    #define DHTDDR       DDRB
    #define DHTPORT     PORTB
    #define DHTIDEPIN       3   // IDE D3
    #define DHTPORT_BIT          3   // DIL-2/D3
    //-- PB2 = DIL-7 = Arduino IDE D2
    #define LEDDDR       DDRB
    #define LEDPORT     PORTB
    #define LEDIDEPIN       2   // IDE D2
    #define LEDPORT_BIT     2   // DIL-7/D2
#else
    #if DEBUG == false
        #define Sbegin(a)   
        #define Sprint(a)   
        #define Sprintln(a) 
        #define Sflush()    
    #endif
    //-- PB2 = DIL-16 = Arduino IDE D10
    #define TXDDR        DDRB
    #define TXPORT      PORTB
    #define TXPORT_BIT      2   // UNO/Trinket-Pro D10
    //-- PD4 = DIL-6 = Arduino IDE 4
    #define DHTDDR       DDRD
    #define DHTPORT     PORTD
    #define DHTIDEPIN       4   // IDE D4
    #define DHTPORT_BIT     4   // UNO/Trinket-Pro D4
    //-- PB5 = DIL-19 = Arduino IDE D13
    #define LEDDDR       DDRB
    #define LEDPORT     PORTB
    #define LEDIDEPIN      13   // IDE D13
    #define LEDPORT_BIT     5   // UNO/Trinket-Pro D13
#endif

#define CONFIGVERSION   2
#define EEPROM_ADDR     0
#define INTERNALREF     1125300L    // default
#define SENSORID        999

// Uncomment whatever type you're using!
#define DHTTYPE DHT11     // DHT 11
//#define DHTTYPE DHT22   // DHT 22 (AM2302, AM2321)
//#define DHTTYPE DHT21   // DHT 21 (AM2301)

DHT dht(DHTIDEPIN, DHTTYPE);

enum seconds_t
{
    SLEEP_1S    = 6,
    SLEEP_8S    = 9
};

static SensorConfig EEPROM;

int     statusBat   = 1;
long    VccBat      = 0;

static float Temperature;
static float Humidity;

void send(uint8_t, uint8_t, uint8_t, int32_t, int8_t, char* );  // prototype

#if defined(__AVR_ATtiny84__) || defined(__AVR_ATtiny85__)
    void  lowPower(seconds_t period)
    {
        // --- Disable ADC ---
        ADCSRA &= ~(1 << ADEN);
        // --- and the rest ---
        power_all_disable ();
        wdt_enable(period);
        WDTCSR |= (1 << WDIE);    

        do {                         
            set_sleep_mode(SLEEP_MODE_PWR_DOWN); 
            cli();                
            sleep_enable();  
            sei();                
            sleep_cpu();          
            sleep_disable();      
            sei();                
        } while (false);

        power_all_enable();
        // --- Enable ADC ---
        ADCSRA |= (1 << ADEN);

    }   // lowPower()


    //------------------------------------------
    // Watchdog Timer interrupt service routine. 
    // This routine is required to allow 
    // automatic WDIF and WDIE bit clearance.
    //------------------------------------------
    ISR (WDT_vect)
    {
        // WDIE & WDIF cleared in hardware upon entering this ISR
        wdt_disable();
    
    }   // ISR()

    void goToSleep(unsigned int seconds) {
        while(seconds >= 8) {
            lowPower(SLEEP_8S); 
            seconds -= 8;
        }
        while (seconds >= 1) {
            lowPower(SLEEP_1S);
            seconds -= 1;
        }

    }   // goToSleep()
#else
    void goToSleep(unsigned int seconds) {
        Sflush();
        delay(seconds * 1000);
    
    }   // goToSleep()
#endif


static void loadConfig () {
    Sprintln(F("\nRead EEPROM configuration"));
    eeprom_read_block(&EEPROM, EEPROM_ADDR, sizeof EEPROM);
    if (EEPROM.configVersion != CONFIGVERSION) {
        EEPROM.sensorID     =  1;
        EEPROM.interval     = 30;
        EEPROM.internalRef  = INTERNALREF;
    }
}   // loadConfig()


void readDHTsensor() {
    dht.begin();
    delay(50);  // expiriment for differend sensors
    // Read temperature as Celsius (the default)
    Temperature = dht.readTemperature();
    //delay(50);
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
    
}   // readDHTsensor()

void prepareAndSend(uint8_t sensorID, uint8_t Unit
                        , uint8_t statusBat, int32_t Value
                        , int8_t Decimals, char *Label) {
    SET_OUTPUT(TXDDR, TXPORT_BIT);
    send(sensorID, Unit, statusBat, Value, Decimals, Label);
    SET_INPUT(TXDDR, TXPORT_BIT);
            
}   // prepareAndSend()


void setup() {
    Sbegin(19200);
    delay(100);
    loadConfig();
    Sprintln(F("\nRF433_DHTsensor"));

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
    prepareAndSend(EEPROM.sensorID, 2, statusBat
                , (int32_t)(Temperature * 100), 2, "*C");

  // Wait one second between transmissions.
    goToSleep(1);
    Sprint(F("Humidity: "));
    Sprint(Humidity);
    Sprint(F(" %"));
    Sprintln();
    prepareAndSend(EEPROM.sensorID, 3, statusBat
                , (int32_t)Humidity, 0, "%RH");

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

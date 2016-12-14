/*
    Program  : DHTsensor v3
   
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
//--- define some common macro's ------------------------
#define SET(a,b)                ((a) |=  _BV(b))
#define CLEAR(a,b)              ((a) &= ~_BV(b))
#define SET_OUTPUT(_ddr, _pin)  ((_ddr) |=  _BV(_pin))

#include "DHT.h"

#if defined(__AVR_ATtiny84__) 
    #define Sbegin(...)
    #define Sprint(...)
    #define Sprintln(...)
    #define Sflush(...)
//                    +--\/--+
//               VCC 1|      |14 GND
//          (D0) PB0 2|      |13 AREF (D10)
//          (D1) PB1 3|      |12 PA1  (D9) [TX]->
//         RESET PB3 4|      |11 PA2  (D8)
//     INT0 (D2) PB2 5|      |10 PA3  (D7) [DHT]<->
//      PWM (D3) PA7 6|      |9  PA4  (D6) 
//      PWM (D4) PA6 7|      |8  PA5  (D5) PWM
//                    +------+
    //-- PA1 = DIL-12 = Arduino IDE D9
    #define TXDDR        DDRA
    #define TXPORT      PORTA
    #define TXPORTBIT       1   // DIL-12
    //-- PA3 = DIL-10 = Arduino IDE D7
    #define DHTDDR       DDRA
    #define DHTPORT     PORTA
    #define DHTIDEPIN       3   // IDE D7
    #define DHTPORTBIT      3   // DIL-10
#elif defined(__AVR_ATtiny85__)
    #define Sbegin(...)
    #define Sprint(...)
    #define Sprintln(...)
    #define Sflush(...)
//                    +--\/--+
//    RESET (D5) PB5 1|      |8 VCC
// <->[DHT] (D3) PB3 2|      |7 PB2 (D2)
//          (D4) PB4 3|      |6 PB1 (D1) [TX]->
//               GND 4|      |5 PB0 (D0)
//                    +------+
    //-- PB1 = DIL-6 = Arduino IDE D1
    #define TXDDR        DDRB
    #define TXPORT      PORTB
    #define TXPORTBIT       1   // DIL-6
    //-- PB3 = DIL-2 = Arduino IDE D3
    #define DHTDDR       DDRB
    #define DHTPORT     PORTB
    #define DHTIDEPIN       3   // IDE D3
    #define DHTPORTBIT      3   // DIL-2
#else
    #define Sbegin(...)     {Serial.begin(__VA_ARGS__);}
    #define Sprint(...)     {Serial.print(__VA_ARGS__);}
    #define Sprintln(...)   {Serial.println(__VA_ARGS__);}
    #define Sflush(...)     {Serial.flush(__VA_ARGS__);}
    //-- PB2 = DIL-16 = Arduino IDE D10
    #define TXDDR        DDRB
    #define TXPORT      PORTB
    #define TXPORTBIT       2   // UNO/Trinket-Pro D10
    //-- PD4 = DIL-6 = Arduino IDE 4
    #define DHTDDR       DDRD
    #define DHTPORT     PORTD
    #define DHTIDEPIN       4   // IDE D4
    #define DHTPORTBIT      4   // UNO/Trinket-Pro D4
#endif

// Uncomment whatever type you're using!
#define DHTTYPE DHT11     // DHT 11
//#define DHTTYPE DHT22   // DHT 22 (AM2302, AM2321)
//#define DHTTYPE DHT21   // DHT 21 (AM2301)

DHT dht(DHTIDEPIN, DHTTYPE);

uint8_t statusBat   = 1;
uint8_t parityBit   = 0;

static float Temperature;
static float Humidity;

// -- prototype send()
void send(uint8_t, uint8_t, uint8_t, int32_t, int8_t, char* ); 

void goToSleep(unsigned int seconds) {
    Sflush();
    delay(seconds * 1000);
    
}   // goToSleep()


void readDHTsensor() {
    // Read temperature as Celsius (the default)
    Temperature = dht.readTemperature();
    // Read humidity
    Humidity    = dht.readHumidity();
    if (isnan(Temperature) || isnan(Humidity)) {
        Sprintln(F("Failed to read from DHT sensor!"));
        if (isnan(Temperature)) Temperature = 0;
        if (isnan(Humidity))    Humidity    = 0;
    }
    
}   // readDHTsensor()


void prepairAndSend(uint8_t sensorID, uint8_t Unit
                           , uint8_t statusBat, int32_t Value
                           , int8_t Decimals, char *Label) {

    send(sensorID, Unit, statusBat, Value, Decimals, Label);
    
}   // prepairAndSend()


void setup() {
    Sbegin(19200);
    SET_OUTPUT(TXDDR,  TXPORTBIT);
    dht.begin();
    delay(50);
    Sprintln(F("\nRF433_DHTsensor"));

}   // setup()

void loop() {
    readDHTsensor();    // sets Temperature and Humidity
    goToSleep(1);
    // --- for now: toggle the statusBat ---
    statusBat  = !statusBat; 
    Sprint(F("Temperature: "));
    Sprint(Temperature);
    Sprint(F(" *C"));
    Sprintln();
    prepairAndSend(123, 1, statusBat
                    , (int32_t)(Temperature * 100), 2, "*C");

  // Wait one second between transmissions.
    goToSleep(1);
    Sprint(F("Humidity: "));
    Sprint(Humidity);
    Sprint(F(" %"));
    Sprintln();
    prepairAndSend(123, 2, statusBat
                    , (int32_t)Humidity, 0, "%");

  // Wait a few seconds between measurements.
    goToSleep(20);
    
}   // loop()

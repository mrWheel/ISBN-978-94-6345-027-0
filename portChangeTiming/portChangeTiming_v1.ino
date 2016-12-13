/*
    Program : portChangeTiming v1
  
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

// define macro's
#define SET(a,b)                ((a) |=  _BV(b))
#define CLEAR(a,b)              ((a) &= ~_BV(b))
#define SET_OUTPUT(_port, _pin) ((_port) |=  _BV(_pin))

//--- PORT and Bit for UNO/Trinket Pro ----
#define _DDR_REG   DDRD
#define _PORT_REG PORTD
#define ARDUINO_PIN   4  // pin-4 UNO Board
#define AVR_PIN       4  // PD4
//--- PORT and Bit for ATtiny84 -----------
//define _DDR_REG   DDRA
//define _PORT_REG PORTA
//define ARDUINO_PIN   4  // DIL-9 ATtiny84
//define AVR_PIN       4  // PA4   ATtiny84

void setup() {
    //pinMode(ARDUINO_PIN, OUTPUT);
    SET_OUTPUT(_DDR_REG, AVR_PIN);    
}   // end_setup()

void loop() {
    SET(_PORT_REG, AVR_PIN); 
    CLEAR(_PORT_REG, AVR_PIN); 
    digitalWrite(ARDUINO_PIN, HIGH);
    
    CLEAR(_PORT_REG, AVR_PIN); 
    SET(_PORT_REG, AVR_PIN); 
    digitalWrite(ARDUINO_PIN, LOW);
    
}   // end_loop()

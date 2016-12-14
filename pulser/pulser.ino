/* 
    Program : pulser v1
  
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

#include <arduino.h>

#define SET(a,b)                ((a) |=  _BV(b))
#define CLEAR(a,b)              ((a) &= ~_BV(b))
#define TOGGLE(a,b)             ((a) ^=  _BV(b))
#define SET_OUTPUT(_port, _pin) ((_port) |=  _BV(_pin))
 
#define _DDR_REG     DDRB // Arduino UNO
#define _PORT_REG   PORTB // Arduino UNO
#define _PULSEPIN       5 // PB5 (UNO pin-13)

#define SYNC  9000

int pulseState     = LOW;

void sendPulse(int duration) {
    pulseState = !pulseState;
    TOGGLE(_PORT_REG, _PULSEPIN);
    delayMicroseconds(duration);
  
} // sendPulse()

void setup() {
    SET_OUTPUT(_DDR_REG, _PULSEPIN);

}   // setup()

void loop() {
    sendPulse(SYNC); //leadin
    for(int U=300; U<=1000; U+=100) {
        sendPulse(U);
    }  
    for(int D=1000; D>=200; D-=100) {
        sendPulse(D);
    }  
    
    delay(5000);
  
}   // loop()

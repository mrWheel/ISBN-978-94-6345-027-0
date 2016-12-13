/*
    Programma    : extClock1MHz 
   
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

    ==================================== 
    Processor      PORTB    PIN  DIL-Pin
    ATtiny84         2      PB2     5
    ATtiny85         0      PB0     5
*/
 
#include <arduino.h>

#if defined(__AVR_ATtiny84__)
    #define OAC0_PIN  2             // PB2 
#elif defined(__AVR_ATtiny85__)
    #define OAC0_PIN  0             // PB0
#endif
void setup() {
    // first calibrate 8MHz RC-oscillator
    // find value with sensorConfigurator
    //OSCCAL  = 124;
    
    CLKPR   = 0x80;         // set system clock to 8 MHz with
    CLKPR   = 0x00;         // no prescale. These 2 CLKPR
                            // instructions have to be run
                            // together in order to set clock
                            // to 8 Mhz
                            
    DDRB   |= (1<<OAC0_PIN);   // Set pin PB0/PB2 as output
    
    TCNT0   = 0;            // initialize timer counter
                            // value to 0
    TCCR0A  = 0;            // write 0 to timer 0 control
    TCCR0B  = 0;            // registers
    
    // With OCR0A = 3 and prescaler 1 ----------------
    // With F_CPU = 8 MHz, the result is 1MHz.
    // f = 8.000.000 / (2 * p=1 * (1 + 3))
    // f = 8.000.000 / 8 ==> 1000.000Hz => 1 MHz
    TCCR0A |= (1 << COM0A0); // Timer0 toggle on Compare Match
    TCCR0A |= (1 << WGM01);  // Start timer 1 in CTC mode
    TCCR0B |= (1 << CS00);   // See Prescaler table 1/1
    OCR0A   = 3;             // CTC Compare value
                             // (3 gives 1 MHz)
} // setup()
  
void loop () {}

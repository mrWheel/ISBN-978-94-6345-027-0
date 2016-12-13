/*
    Program  : GPSexample
  
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

#include "GPSprotocol.h"

// initialize GPS for PORTB PB5 (D13)
// connect a transmitter-data pin to Arduino pin-13
GPSprotocol GPS(&PORTB, 5);
int Counter = 0;

void setup() {
    Serial.begin(19200);
    delay(100);
    pinMode(13, OUTPUT);
    Serial.println(F("\nGPSdemo"));
    
}   // setup()

void loop() {
    Serial.print(F("Counter DEC: "));
    Serial.print(Counter);
    Serial.print(", BIN: ");
    Serial.print(Counter, BIN);
    Serial.println();
    //        ID, Unit, BatStatus, Payload, Decimals, Label
    GPS.send(100,           // sensorID    
               2,           // sensor Unit
               1,           // Bat.Status
               Counter,     // payload
               0,           // Dec. places payload
               "Tcks"       // Label
               );
    Counter++;
    delay(1000);
         
}   // loop()

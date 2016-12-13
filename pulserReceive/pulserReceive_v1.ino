/*  
    Program : pulserReceive v1
  
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
    
    The pulser program creates a pulse-train containing: 
    1 x SYNC 9000uS
    Pulses varying from 300uS to 1000uS and back from 
    1000uS to 300uS in steps of 100uS
    In total there will be 16 state-changes.
    The purpose of this program is to meassure the time 
    between the flanks and display them.
    
      +---//----+  +---+    +-----+      +-------+
      |  SYNC   |  |   |    |     |      |       |
    --+         +--+   +----+     +------+       +-//--
*/

#define MAX_CHANGES  50
#define SYNC       9000

int                 enableISP   = true;
int                 hasData     = false;
int                 duration;
long                lastTime;
unsigned int        changeCount = 0;
static unsigned int rawData[MAX_CHANGES];

void blinkLed(int num) {
  for(int l=0; l<num; l++) {
    digitalWrite(13, HIGH);
    delay(100);
    digitalWrite(13, LOW);
    delay(400);
  }
}

void interruptISP(void) {
    long time = micros();
    if (enableISP) {
        duration = time - lastTime;
      
        if ((duration > SYNC - 200) && 
            (duration < SYNC + 200)) {
                changeCount = 0;
                rawData[changeCount++] = duration;
        }
        else if ((duration > 280) && 
                 (duration < 1100)) {
                if (changeCount > 0 && 
                   changeCount < MAX_CHANGES) {
                      rawData[changeCount++] = duration;
                }
        }
        if (changeCount > 16) {
            hasData = true;
        }
    } // if enableISP
    lastTime = time;  
 
} // interruptISP()


void setup() {
  Serial.begin(19200);
  pinMode(13, OUTPUT);
  Serial.println("Starting program..");
  blinkLed(5);
  Serial.println("Wait for interrupt..");
  Serial.flush();
  attachInterrupt(0, interruptISP, CHANGE);
}

void loop() {
  // put your main code here, to run repeatedly:
  enableISP = true;
  if (hasData) {
      enableISP = false;
      for (int i = 0; i < changeCount; i++) {
          Serial.print(rawData[i]);
          Serial.print(" ");
      }
      Serial.print("(");
      Serial.print(changeCount);
      Serial.println(" pulses)");
      changeCount = 0;
      hasData   = false;
      enableISP = true;
  }

}

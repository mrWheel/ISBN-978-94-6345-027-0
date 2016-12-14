/*
    Program  : RF433repeater v1
  
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

#include "arduino.h"

//--- define some common macro's ------------------------
#define SET(a,b)          ((a) |=  _BV(b))
#define CLEAR(a,b)        ((a) &= ~_BV(b))
#define GET(a,b)          ((a) &   _BV(b))
#define TOGGLE(a,b)       ((a) ^=  _BV(b))
#define SET_OUTPUT(_port, _pin)  ((_port) |=  _BV(_pin))
#define SET_INPUT(_port, _pin)   ((_port) &= ~_BV(_pin))

#define _DDR_REG        DDRD
#define _PORT_REG       PORTD
#define ARDUINO_TXPIN     6      // PD6 (UNO pin-6)
#define AVR_TXPIN         6      // (Trinket-Pro pin-6)
#define INTERRUPT_PIN     1

#define MAXPULSES       300
#define SYNC_LO_LIMIT   2500
#define SYNC_HI_LIMIT   25000
#define LEDPIN          13

enum { UNKNOWN, SYNC, DATA, DONE };
enum { UNKNOWNpulse, SYNCpulse, DATApulse,INVALIDpulse };

static struct { uint8_t state; 
                uint8_t numPulses; 
                uint16_t pulse[MAXPULSES]; 
              } RF433;

static uint16_t minSync   = SYNC_LO_LIMIT; // min. sync length
static uint16_t maxSync   = SYNC_HI_LIMIT; // max. sync length
static uint16_t minPulse  = 150;               // min data-pulse
static uint16_t maxPulse  = SYNC_LO_LIMIT / 2; // max data-pulse
uint16_t        lastPulse = 0;  // puls length last pulse
byte            ledState  = 0;

//----- Interrupt Service Routine -----------------------------
void interruptSR() {
    // lastPulse is the duration between 2 interrupts in usecs
    uint8_t         pulseType = UNKNOWNpulse;
    static uint16_t lastTime;
    uint16_t        lastPulse = micros() - lastTime;
    lastTime += lastPulse;

    //--- determine the type of Pulse received -------
    if (lastPulse > minSync && lastPulse < maxSync) {
            pulseType   = SYNCpulse;    // SYNC Pulse
    }
    else if (lastPulse > minPulse && lastPulse < maxPulse)  {
            pulseType   = DATApulse;    // Genuin data Pulse
    }
    else {
            pulseType   = INVALIDpulse; // invallid Pulse
    }

    //----- enter Finite State Machine --------------------
    if (RF433.state != DONE) {
        switch(RF433.state) {
            case UNKNOWN:   
                switch(pulseType) {
                    case SYNCpulse: // SYNC received
                        RF433.state     = SYNC;
                        RF433.numPulses = 0;
                        RF433.pulse[RF433.numPulses] = 
                                                lastPulse;
                        minSync     = lastPulse - 100;
                        maxSync     = lastPulse + 100;
                        minPulse    = minSync / 55;
                        maxPulse    = minSync / 2;
                        break;

                    default:
                        RF433.state = UNKNOWN;
                        RF433.numPulses = 0;
                        RF433.pulse[RF433.numPulses]  = 0;
                        minSync         = SYNC_LO_LIMIT;
                        maxSync         = SYNC_HI_LIMIT;
                        break;
                }
                break;
                        
            case SYNC:      
                switch(pulseType) {
                    case DATApulse: // Genuine Data Pulse
                        RF433.state = DATA;
                        RF433.pulse[++RF433.numPulses] = 
                                                lastPulse;
                        break;
                    case SYNCpulse: // SYNC received
                        RF433.state     = SYNC;
                        RF433.numPulses = 0;
                        RF433.pulse[RF433.numPulses] = 
                                                lastPulse;
                        minSync     = lastPulse - 100;
                        maxSync     = lastPulse + 100;
                        minPulse    = minSync / 55;
                        maxPulse    = minSync / 2;
                        break;
                        
                    default: // SYNC or invalid pulse
                        RF433.state = UNKNOWN;
                        RF433.numPulses = 0;
                        RF433.pulse[0]  = 0;
                        minSync         = SYNC_LO_LIMIT;
                        maxSync         = SYNC_HI_LIMIT;
                        break;
                }
                break;
                    
            case DATA:  
                switch(pulseType) {
                    case DATApulse: // Genuine Data Pulse
                        RF433.state = DATA;
                        RF433.pulse[++RF433.numPulses] =
                                                lastPulse;
                        //--- sanaty check ----------------
                        if (RF433.numPulses > (MAXPULSES -1)) {
                            RF433.state     = DONE;
                        }
                        break;
                    case SYNCpulse: // SYNC received
                        RF433.state = DONE;
                        RF433.pulse[++RF433.numPulses] =
                                                lastPulse;
                        break;
                        
                    default: // invalid pulse
                        RF433.state = UNKNOWN;
                        RF433.numPulses = 0;
                        RF433.pulse[0]  = 0;
                        minSync         = SYNC_LO_LIMIT;
                        maxSync         = SYNC_HI_LIMIT;
                        break;
                }
                break;
                        
            case DONE:  // nothing to do (here)!
                break;
                    
            default:// invallid pulse length
                    RF433.state     = UNKNOWN;
                    RF433.numPulses = 0;
                    RF433.pulse[RF433.numPulses]  = 0;
                    minSync         = SYNC_LO_LIMIT;
                    maxSync         = SYNC_HI_LIMIT;
                    break;
                        
        }   // switch(RF433.state)
        
    }
    
}   // interruptSR()

//----- information about Sync and Pulse width -----
void showMetaInfo() {
    Serial.print("\nRF433 (");   
    Serial.print(RF433.numPulses);
    Serial.print(" Pulses) SYNC: [");
    Serial.print(RF433.pulse[0]);   Serial.print("]");
    Serial.print(", minSync[");     Serial.print(minSync);
    Serial.print("], maxSync[");    Serial.print(maxSync);
    Serial.print("], minPulse[");   Serial.print(minPulse);
    Serial.print("], maxPulse[");   Serial.print(maxPulse);
    Serial.print("] ");
}   // showMetaInfo()

//------- information about Pulse width -------
void showPulseInfo() {
    //--- show all pulse-length's -------------
    //--- but skip first and last SYNC --------
    for (int I = 1; I < RF433.numPulses; I++) {
        if (I % 8 == 1)   Serial.print("\n\t");
        Serial.print(RF433.pulse[I]);
        Serial.print("\t");
    }
    //--- now show last SYNC ------------------
    Serial.print("[");
    Serial.print(RF433.pulse[RF433.numPulses]);
    Serial.println("]");
}   // showPulseInfo()

//----- send a pulse ------------
void sendPulse(int pulseLength) {
    TOGGLE(_PORT_REG, AVR_TXPIN);
    delayMicroseconds(pulseLength);
    
}   // sendPulse()


void sendPulseTrain() {
    int R;
    SET(_PORT_REG, AVR_TXPIN);
    delay(500);
    //--- send some pulses to tune-in the radio ---
    for (R = 0; R < 6; R++) {
        sendPulse(100); // leadIn
    }
    //--- send puls-train a view times in a row ---
    for (R = 0; R < 5; R++) {
        //--- repeat all pulse's ------------------
        //--- except the last one (double SYNC) ---
        for (int I = 0; I < RF433.numPulses; I++) {
            sendPulse(RF433.pulse[I]);
        }
    }
    //--- end with the SYNC-pulse ----------
    sendPulse(RF433.pulse[0]);
    CLEAR(_PORT_REG, AVR_TXPIN);

}   // sendPulseTrain()


void setup() {
    Serial.begin(19200);
    pinMode(LEDPIN, OUTPUT);
    SET_OUTPUT(_DDR_REG, AVR_TXPIN); 
    minPulse    = SYNC_LO_LIMIT / 55;
    maxPulse    = SYNC_LO_LIMIT / 2;
    Serial.println("\n[RF433repeater]");
    //--- accept Interrups @INT1 (pin3) ---
    attachInterrupt(INTERRUPT_PIN, interruptSR, CHANGE);    

}   // setup()


void loop() {
    if (RF433.state == DONE) {
        //-- we dont want interrupts during processing
        detachInterrupt(INTERRUPT_PIN);     
        if (RF433.numPulses > 15) { // sanity check
            ledState = !ledState;
            digitalWrite(LEDPIN, HIGH);
            delay(200);
            sendPulseTrain();
            //showMetaInfo();
            //showPulseInfo();
            digitalWrite(LEDPIN, LOW);
        }
        //--- reset state -------------
        RF433.state     = UNKNOWN;
        RF433.pulse[0]  = 0;
        RF433.numPulses = 0;
        //--- re-attach interrups @INT1 again ---
        attachInterrupt(INTERRUPT_PIN, interruptSR, CHANGE);    
    }
    
}   // loop()

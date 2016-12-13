/*
    Program  : RFreceiver v8
  
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

//--- define some common macro's ------------------------
#define SET(a,b)                ((a) |=  _BV(b))
#define CLEAR(a,b)              ((a) &= ~_BV(b))
#define SET_OUTPUT(_ddr, _pin)  ((_ddr) |=  _BV(_pin))

//-- set DEBUG to false to suppress messages
#define DEBUG       true
#define SHOWUNKNOWN false

#if DEBUG == false 
    #define Sbegin(...)   
    #define Sprint(...)   
    #define Sprintln(...) 
    #define Sflush(...)    
#else
    #define Sbegin(...)     {Serial.begin(__VA_ARGS__); }
    #define Sprint(...)     {Serial.print(__VA_ARGS__); }
    #define Sprintln(...)   {Serial.println(__VA_ARGS__); }
    #define Sflush(...)     {Serial.flush(__VA_ARGS__); }
#endif

#define LEDPIN             13
#define _TXPORT         PORTB
#define _TXPORT_BIT         9

#define _DDR_REG         DDRB
#define _PORT_REG       PORTB
#define ARDUINO_LED_PIN     8      // PB0 (UNO pin-8)
#define IMPULS_B_LED_PIN    0      // (Trinket-Pro pin-8)
#define INTERRUPT_PIN       1

#define MAXPULSES         200
#define MAXBITS           100
#define SYNC_LO_LIMIT    2500
#define SYNC_HI_LIMIT   25000

enum { UNKNOWN, SYNC, DATA, DONE };
enum { UNKNOWNpulse, SYNCpulse, DATApulse,INVALIDpulse };
enum { PROTOCOLUNKNOWN, PROTOCOLPROCESSED };

static struct { uint8_t state; 
                uint8_t numPulses; 
                uint16_t pulse[MAXPULSES]; 
              } RF433;

uint16_t minSync   = SYNC_LO_LIMIT; // min. sync lngth
uint16_t maxSync   = SYNC_HI_LIMIT; // max. sync lngth
uint16_t minPulse  = 150;           // min data-plse
uint16_t maxPulse  = SYNC_LO_LIMIT / 2; // max data-plse
uint16_t lastPulse = 0;  // puls lngth last plse
byte     ledState  = 0;
char     bits[MAXBITS];
uint8_t  numBits;
uint8_t  parityBit;
uint8_t  PROTOCOL = PROTOCOLUNKNOWN;

void addBit(char oneBit) {
    if (numBits < MAXBITS) {
        bits[numBits++] = oneBit;
        bits[(numBits + 1)] = '\0';
    }
    
}   // addBit()

bool inRange(int Pulse, int Width, int Diff) {
    if ((Pulse > (Width - Diff)) && 
        (Pulse < (Width + Diff))) {
        return true;
    }
    return false;
    
}   // inRange()


uint32_t bits2IntRev(int S, int E) {
    uint32_t    tmpVal = 0;
    for(int p = S; p <= E; p++) {
        if (bits[p] == '1') SET(tmpVal, (p - S));
    }

    return tmpVal;
    
}   // bits2IntRev()


uint32_t bits2Int(int S, int E) {
    uint32_t    tmpVal = 0;
    for(int p = S; p <= E; p++) {
        if (bits[p] == '1') SET(tmpVal, (E - p));
    }

    return tmpVal;
    
}   // bits2Int()

//----- information about Sync and Pulse width -----
void showMetaInfo() {
    Sprint(F("\nRF433 ("));   
    Sprint(RF433.numPulses);
    Sprint(F(" Pulses) SYNC: ["));
    Sprint(RF433.pulse[0]);    Sprint(F("]"));
    Sprint(F(", minSync["));   Sprint(minSync);
    Sprint(F("], maxSync["));  Sprint(maxSync);
    Sprint(F("], minPulse[")); Sprint(minPulse);
    Sprint(F("], maxPulse[")); Sprint(maxPulse);
    Sprintln(F("] "));
}   // showMetaInfo()

//------- information about Pulse width -------
void showPulseInfo() {
    //--- show all pulse-length's -------------
    Sprint(" [");
    Sprint(RF433.pulse[0]);
    Sprint("]\t");
    for (int I = 1; I < RF433.numPulses; I++) {
        if (I % 16 == 0)   Sprint("\n\t");
        Sprint(RF433.pulse[I]);
        Sprint("\t");
    }
    //--- now show last SYNC ------------------
    Sprint(" [");
    Sprint(RF433.pulse[RF433.numPulses]);
    Sprintln("]");

}   // showPulseInfo()


bool analyseProtocol() {
    numBits = 0;
    bits[0] = '\0';
    PROTOCOL    = PROTOCOLUNKNOWN;
    if (RF433.numPulses > 20) {
        if (!PROTOCOL) analyseImpuls();
        if (!PROTOCOL) analyseEverFlourish();
        if (!PROTOCOL) analyseWX500();
        if (!PROTOCOL) analyseWS1700();
        if (!PROTOCOL) analyseKAKU();
        if (!PROTOCOL) analyseGPS();
        // -- add more protocols here --
        // if (!PROTOCOL) analyseNewProtocol();

    }

    return PROTOCOL;
    
}   // analyseProtocol()


void setup() {
    Serial.begin(19200);
    pinMode(LEDPIN, OUTPUT);
    pinMode(9, OUTPUT);
    SET_OUTPUT(_DDR_REG, IMPULS_B_LED_PIN); 
    minPulse    = SYNC_LO_LIMIT / 55;
    maxPulse    = SYNC_LO_LIMIT / 2;
    RF433.state = UNKNOWN;
    Serial.println(F("\n[RF433receiver]"));
    //--- accept Interrups @INTERRUPT_PIN ---
    pinMode(INTERRUPT_PIN, INPUT);
    attachInterrupt(INTERRUPT_PIN, interruptSR, CHANGE);    

}   // setup()


void loop() {
    if (RF433.state == DONE) {
        //-- we dont want interrupts during processing
        detachInterrupt(INTERRUPT_PIN);   
        if (RF433.numPulses > 15) { // sanity check
            ledState = !ledState;
            digitalWrite(LEDPIN, ledState);
            if (!analyseProtocol()) {
                // -- no valid PROTOCOL found -- 
                showMetaInfo();
                Sprint(F("Unkown protocol with "));
                Sprint(RF433.numPulses);
                Sprintln(F(" pulses"));
                #if SHOWUNKNOWN == true
                    showPulseInfo();
                #endif
            }
        }
        //--- reset state -------------
        RF433.state     = UNKNOWN;
        RF433.pulse[0]  = 0;
        RF433.numPulses = 0;
        //--- re-attach interrups @INT1 again ---
        attachInterrupt(INTERRUPT_PIN, interruptSR, CHANGE);    
    }
    
}   // loop()

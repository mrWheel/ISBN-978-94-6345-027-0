/*
** Library  : GPSprotocol.cpp - General Purpose Sensor protocol
** Author   : Willem Aandewiel
*/

#if (ARDUINO >= 100)
    #include "Arduino.h"
#else
    #include "WProgram.h"
#endif

#define GPSSYNC         4500
#define GPSSTART         250
#define GPSLONG         1800
#define GPSSHORT         900

// --- LeadIn to set receiver ---
void GPSsendLeadIn()
{
    for (int P = 0; P < 5; P++) {
        digitalWrite( _TXPORT_BIT, HIGH );
        delayMicroseconds(25);
        digitalWrite( _TXPORT_BIT, LOW);
        delayMicroseconds(25);
    }

} // GPSsendLeadIn()

// --- send GPSSYNC pulse ---
void GPSsendSYNC()
{
    digitalWrite( _TXPORT_BIT, HIGH );
    delayMicroseconds(GPSSTART);
    digitalWrite( _TXPORT_BIT, LOW );
    delayMicroseconds(GPSSYNC);

} // GPSsendSYNC()

// --- send integer ---
void GPSsendInt(int32_t data, int bitsWanted) {
    uint32_t mask;
    for (int b = 1; b <= bitsWanted; b++) {
        mask = 1;
        mask <<= (bitsWanted - b);
        if (data & mask) {
            parityBit++;
            digitalWrite( _TXPORT_BIT, HIGH );
            delayMicroseconds(GPSSTART);
            digitalWrite( _TXPORT_BIT, LOW );
            delayMicroseconds(GPSLONG);
        }
        else {
            digitalWrite( _TXPORT_BIT, HIGH );
            delayMicroseconds(GPSSTART);
            digitalWrite( _TXPORT_BIT, LOW );
            delayMicroseconds(GPSSHORT);
        }
    }   // for ..

} // GPSsendInt()


void GPSsendString(char* data, int charsWanted) {
    for (int c = 0; c < charsWanted; c++) {
        if (c > (int)strlen(data)) {
            GPSsendInt(0, 8);
        }
        else {
            GPSsendInt(data[c], 8);
        }
    }   // for ..

} // GPSsendString()


void GPSsend(uint8_t sensorID, uint8_t Unit
                            , uint8_t Bat, int32_t Value
                            , int8_t Decimals, char *Label ) {

    GPSsendLeadIn();   // tune receiver
    parityBit = 0;
    // repeat pulse-train 7 times
    for (int L = 0; L < 7; L++) {
        GPSsendSYNC();
        GPSsendInt(0b1100, 4);     //  4 bit - Header
        GPSsendInt(sensorID, 8);   //  8 bit
        GPSsendInt(Unit, 3);       //  3 bit
        GPSsendInt(Bat, 1);        //  1 bit - Batery status
        GPSsendInt(Value, 20);     // 20 bit - payload
        GPSsendInt(Decimals, 3);   //  3 bit
        GPSsendString(Label, 4);   // 32 bit (4x8)
        // 4 + 8 + 3 + 1 + 20 + 3 + 32 + parityBit = 72 bits
        if ((parityBit % 2) == 0) GPSsendInt(0, 1);
        else                      GPSsendInt(1, 1);
    } // for L ...

} // GPSsend()


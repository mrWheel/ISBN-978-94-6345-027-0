/*
** Library  : GPSprotocol.cpp - General Purpose Sensor protocol
** Author   : Willem Aandewiel
*/

#if (ARDUINO >= 100)
    #include "Arduino.h"
#else
    #include "WProgram.h"
#endif
#include "GPSprotocol.h"

volatile uint8_t *_TXPORT;
volatile uint8_t *_TXPORT_BIT;
uint8_t parityBit;

// --- constructor -------------
GPSprotocol::GPSprotocol(volatile uint8_t* port, uint8_t port_bit) {
    _TXPORT  = port;
    _TXPORT_BIT = port_bit;
    
}   // begin()

// --- LeadIn to set receiver ---
void GPSprotocol::sendLeadIn()
{
    for (int P = 0; P < 5; P++) {
        ( *_TXPORT ) |=  ( 1 << _TXPORT_BIT );
        delayMicroseconds(25);
        ( *_TXPORT ) &= ~( 1<< _TXPORT_BIT );
        delayMicroseconds(25);
    }

} // sendLeadIn()

// --- send SYNC pulse ---
void GPSprotocol::sendSYNC()
{
    ( *_TXPORT ) |=  ( 1 << _TXPORT_BIT );
    delayMicroseconds(GPSSTART);
    ( *_TXPORT ) &= ~( 1 << _TXPORT_BIT );
    delayMicroseconds(GPSSYNC);

} // sendSYNC()

// --- send integer ---
void GPSprotocol::sendInt(int32_t data, int bitsWanted) {
    uint32_t mask;
    for (int b = 1; b <= bitsWanted; b++) {
        mask = 1;
        mask <<= (bitsWanted - b);
        if (data & mask) {
            parityBit++;
            ( *_TXPORT ) |=  ( 1 << _TXPORT_BIT );
            delayMicroseconds(GPSSTART);
            ( *_TXPORT ) &= ~( 1 << _TXPORT_BIT );
            delayMicroseconds(GPSLONG);
        }
        else {
            ( *_TXPORT ) |=  ( 1 << _TXPORT_BIT );
            delayMicroseconds(GPSSTART);
            ( *_TXPORT ) &= ~( 1 << _TXPORT_BIT );
            delayMicroseconds(GPSSHORT);
        }
    }   // for ..

} // sendInt()


void GPSprotocol::sendString(char* data, int charsWanted) {
    for (int c = 0; c < charsWanted; c++) {
        if (c > strlen(data)) {
            sendInt(0, 8);
        }
        else {
            sendInt(data[c], 8);
        }
    }   // for ..

} // sendString()


void GPSprotocol::send(uint8_t sensorID, uint8_t Unit
							, uint8_t Bat, int32_t Value
                            , int8_t Decimals, char *Label ) {

    sendLeadIn();   // tune receiver
    parityBit = 0;
    // repeat pulse-train 7 times
    for (int L = 0; L < 7; L++) {
        sendSYNC();
        sendInt(0b1100, 4);  	//  4 bit - Header
        sendInt(sensorID, 8);	//  8 bit
        sendInt(Unit, 3);		//  3 bit
        sendInt(Bat, 1);     	//  1 bit - Batery status
        sendInt(Value, 20);		// 20 bit - payload
        sendInt(Decimals, 3);	//  3 bit
        sendString(Label, 4);	// 32 bit (4x8)
        // 4 + 8 + 3 + 1 + 20 + 3 + 32 + parityBit = 72 bits
        if ((parityBit % 2) == 0) sendInt(0, 1);
        else                      sendInt(1, 1);
    } // for L ...

} // send()


/*
** Library  : GPSprotocol.h - General Purpose Sensor protocol
** Author   : Willem Aandewiel
*/

#ifndef GPSprotocol_h
#define GPSprotocol_h

#if (ARDUINO >= 100)
    #include "Arduino.h"
#else
    #include "WProgram.h"
#endif

//--- protocol info ---
#define GPSSYNC   4500
#define GPSSTART   250
#define GPSSHORT   900
#define GPSLONG   1800

class GPSprotocol
{
  public:
    GPSprotocol(volatile uint8_t* port, uint8_t port_pin);
    void send(uint8_t sensorID, uint8_t Unit
    	                 , uint8_t Bat, int32_t Value
                         , int8_t Decimals, char *Label );
  private:
    volatile uint8_t* _TXPORT;
    uint8_t _TXPORT_BIT;
    uint8_t  parityBit;
    void sendLeadIn();
    void sendSYNC();
    void sendInt(int32_t data, int bitsWanted);
    void sendString(char* data, int charsWanted);
    
};

#endif

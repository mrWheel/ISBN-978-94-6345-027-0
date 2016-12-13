/*
 * HeaderFile   : mySensors.h
 * Author       : Willem Aandewiel
 */
#ifndef MYSENSORS_H
#define MYSENSORS_H

//--- define some common macro's ------------------------
#define SET(a,b)                ((a) |=  _BV(b))
#define CLEAR(a,b)              ((a) &= ~_BV(b))
#define GET(a,b)                ((a) &   _BV(b))
#define TOGGLE(a,b)             ((a) ^=  _BV(b))
#define SET_OUTPUT(_ddr, _pin)  ((_ddr) |=  _BV(_pin))
#define SET_INPUT(_ddr, _pin)   ((_ddr) &= ~_BV(_pin))

#ifndef WDTCSR
    #define WDTCSR  WDTCR
#endif

#ifndef DEBUG
	#define DEBUG  true
#endif

#if DEBUG == false \
	|| defined(__AVR_ATtiny84__) || defined(__AVR_ATtiny85__) 
    //#error ATtiny or DEBUG false
    #define Sbegin(...)   
    #define Sprint(...)   
    #define Sprintln(...) 
    #define Sflush(...)    
#else
	//#error No Tiny, No DEBUG or DEBUG true
    #define Sbegin(...)     {Serial.begin(__VA_ARGS__); }
    #define Sprint(...)     {Serial.print(__VA_ARGS__); }
    #define Sprintln(...)   {Serial.println(__VA_ARGS__); }
    #define Sflush(...)     {Serial.flush(__VA_ARGS__); }
#endif

// Sensor configuration
#define CONFIGVERSION   2
#define EEPROM_ADDR     0
#define INTERNALREF     1125300L    // default

typedef struct {
    uint8_t  configVersion;           
    uint8_t  sensorID;  
    uint16_t interval;      
    uint32_t internalRef;       
    uint8_t  oscillatorCal;      
} SensorConfig;

enum seconds_t {
    SLEEP_1S    = 6,  // WDP1 & WDP2
    SLEEP_2S    = 7,
    SLEEP_4S    = 8,
    SLEEP_8S    = 9   // WDP0 & WDP3
};

#endif
This is an Arduino library for the TSL2561 digital luminosity (light) sensors. 

Pick one up at http://www.adafruit.com/products/439

To download. click the DOWNLOADS button in the top right corner, rename the uncompressed folder TSL2561. Check that the TSL2561 folder contains TSL2561.cpp and TSL2561.h

Place the TSL2561 library folder your <arduinosketchfolder>/libraries/ folder. You may need to create the libraries subfolder if its your first library. Restart the IDE.

The file "TSL2561.h" is modified by Willem Aandewiel

 46     //---- modified for use with ATtiny84/5
 47     #if defined(__AVR_ATtiny84__) || defined(__AVR_ATtiny85__)
 48         #include <TinyWireM.h>
 49         #define  Wire    TinyWireM
 50     #else
 51         #include <Wire.h>
 52     #endif

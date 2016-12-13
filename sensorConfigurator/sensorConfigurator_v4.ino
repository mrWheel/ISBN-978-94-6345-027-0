/*
    Program  : sensorConfigurator v4
  
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

#include <avr/eeprom.h>
#if defined(__AVR_ATtiny85__)
// ATMEL ATTINY85
//                          +--\/--+
//               RESET PB5 1|      |8 VCC
//         ->[RX] (D3) PB3 2|      |7 PB2 (D2) INT0 [10kHz]<-
//                (D4) PB4 3|      |6 PB1 (D1)
//                     GND 4|      |5 PB0 (D0) AREF [TX]->
//                          +------+
    // DIL-7
    #define INTDDR       DDRB
    #define INTERRUPT_PORTBIT   2   // PB2
    #define INTERRUPT           0
    #include <SoftwareSerial.h>
    // RX = D3 = DIL-2
    // TX = D0 = DIL-5
    SoftwareSerial mySerial(3, 0); // RX, TX
#elif defined (__AVR_ATtiny84__)
// ATMEL ATTINY84
//                          +--\/--+
//                     VCC 1|      |14 GND
//               (D10) PB0 2|      |13 AREF (D0)
//                (D9) PB1 3|      |12 PA1  (D1)
//               RESET PB3 4|      |11 PA2  (D2)
// ->[10kHz] INT0 (D8) PB2 5|      |10 PA3  (D3) [RX]<-
//            PWM (D7) PA7 6|      |9  PA4  (D4) [TX]->
//            PWM (D6) PA6 7|      |8  PA5  (D5) PWM
//                          +------+
    // DIL-5
    #define INTDDR       DDRB
    #define INTERRUPT_PORTBIT   0   // PB2
    #define INTERRUPT           0
    #include <SoftwareSerial.h>
    // RX = D3 = DIL-10
    // TX = D4 = DIL-11
    SoftwareSerial mySerial(3, 4); // RX, TX
#else
    #define INTDDR       DDRD
    #define INTERRUPT_PORTBIT   3   // PD3
    #define INTERRUPT           1
    #define mySerial Serial
#endif
// in <ArduinoProjects>Library/mySensors
#include <mySensors.h>

#define MAXPULSES    2500

enum { UNKNOWNSTATE, SHOWMENU, READCONFIG
      , WRITECONFIG, SETSENSORID 
      , SETINTERVAL, SETVOLTAGE, CALIBRATE
     };

uint16_t    Volts  = 0;
uint16_t    VccExtern;
uint32_t    VccFactor;
char        charIn;
bool        keyPressed = true;
byte        state   = UNKNOWNSTATE;
long        showVccTime;

volatile uint32_t   pulse1000;
volatile uint16_t   pulseSkip;
volatile uint16_t   pulseCounter;
volatile uint32_t   startTime;
float               divider;
float               freqIn;

#define MENU_ENTRIES    12
#define MENU_ITEM_LEN   13

const char menuText [MENU_ENTRIES] [MENU_ITEM_LEN] PROGMEM = {
        { "Version" },
        { "sensorID" },
        { "interval" },
        { "intern.Ref" },
        { "OSCCAL" },
        { "ADC reading" },
        { "(new) Vcc" },
        { "(EEPROM) Vcc" },
        { "read EEPROM" },
        { "write EEPROM" },
        { "show Menu" },
        { "Voltmeter" },
};

#define M_VERSION       0
#define M_SENSORID      1
#define M_INTERVAL      2
#define M_INT_REF       3
#define M_OSCCAL        4
#define M_ADC           5
#define M_NEW_VCC       6
#define M_EEPROM_VCC    7
#define M_READ_EEPROM   8
#define M_WRITE_EEPROM  9
#define M_SHOW_MENU    10
#define M_VOLTMETER    11

static SensorConfig EEPROM;
static SensorConfig newConfig;

void interruptSR() {
    // pulse1000 holds the duration between the 
    // first and pulseCounter interrupts in usecs
    pulseSkip++;
    pulseCounter++;
    if (pulseSkip <= 5) { // skip first 5 pulses
        pulseCounter = 0;
        startTime = micros();
    }
    else {
        pulse1000 = micros() - startTime;
    }

}   // interruptSR()


void displayCalData() {
    mySerial.print(F("\nDone callibrating\nOSCCAL is ["));
    mySerial.print(OSCCAL);
    mySerial.print(F("] EEPROM value [")); 
    mySerial.print(EEPROM.oscillatorCal);
    mySerial.println(F("]"));
    mySerial.flush();

}   // displayCalData()

void autoCalibrate() {
    bool notComplete    = true;
    pulseCounter = 0;
    pulseSkip    = 0;
    pulse1000    = 0;
    OSCCAL      += 2;
    mySerial.println(F("\nCallibrating..."));
    mySerial.flush();
    attachInterrupt(INTERRUPT, interruptSR, RISING);
    while (notComplete) {
        if (pulseCounter >= MAXPULSES) {
            detachInterrupt(INTERRUPT);
            mySerial.begin(9600);
            delay(500);
            divider = pulseCounter * 10.0;
            freqIn = (float)(pulse1000 / divider);
            mySerial.print(F("OSCCAL ["));
            mySerial.print(OSCCAL);
            mySerial.print(F("] ["));
            mySerial.print(pulse1000);
            mySerial.print(F("] => 1 pulse ["));
            mySerial.print((float)(pulse1000 / 
                                  (pulseCounter * 1.0)), 2);
            mySerial.print(F("]us => Pulse In ["));
            mySerial.print(freqIn, 3);
            mySerial.print(F("kHz] Clock ["));
            mySerial.print((freqIn * 0.8), 3);
            mySerial.println(F("MHz]"));
            mySerial.flush();
            if (freqIn > 10.03)         OSCCAL--;
            else if (freqIn <  9.96)    OSCCAL++;
            else notComplete = false;

            pulseCounter = 0;
            pulseSkip    = 0;
            pulse1000    = 0;
            if (notComplete) {
                delay(1000);
                attachInterrupt(INTERRUPT, interruptSR, RISING);
            }
        }
    }   // while(notComplete)

    // assuming 5V compensate for low battery
    OSCCAL += 2;
    newConfig.oscillatorCal = (uint8_t)OSCCAL;

}   // autoCalibrate();


static void readConfig() {
    mySerial.println(F("\nRead EEPROM"));
    eeprom_read_block(&EEPROM, EEPROM_ADDR, sizeof EEPROM);
}

static void writeConfig() {
    mySerial.println(F("\nWrite EEPROM"));
    eeprom_write_block(&EEPROM, EEPROM_ADDR, sizeof EEPROM);
}

long readADC() {
  // Read 1.1V reference against Vcc
#if defined(__AVR_ATmega32U4__) \
            || defined(__AVR_ATmega1280__) \
            || defined(__AVR_ATmega2560__)
    ADMUX = _BV(REFS0)  | _BV(MUX4) \
                        | _BV(MUX3) \
                        | _BV(MUX2) \
                        | _BV(MUX1);
#elif defined (__AVR_ATtiny24__) \
            || defined(__AVR_ATtiny44__) \
            || defined(__AVR_ATtiny84__)
    ADMUX = _BV(MUX5) | _BV(MUX0);
#elif defined (__AVR_ATtiny25__) \
            || defined(__AVR_ATtiny45__) \
            || defined(__AVR_ATtiny85__)
    ADMUX = _BV(MUX3) | _BV(MUX2);
#else
    ADMUX = _BV(REFS0) | _BV(MUX3) \
                       | _BV(MUX2) \
                       | _BV(MUX1);
#endif

    delayMicroseconds(500); // Wait for Vref to settle
    ADCSRA |= _BV(ADSC); // Start conversion
    while (bit_is_set(ADCSRA, ADSC)); // measuring

    long result = ADC;
    return result;

}   // readADC()


long calcARef(long extVcc) {
    long ADCval  = readADC();
    long newARef = ((extVcc * ADCval) / 1023) * 1000;

    return newARef;

}   // calcARef()


long readVcc(uint32_t internalRef) {
    long ADCval   = readADC();
    long Vcc = ((internalRef * 1023) / ADCval) / 1000;

    return Vcc; // Vcc in millivolts

}   // readVcc()


int strlenProgStr (const char * str)
{
    char c;
    int  sLen = 0;
    if (!str)   return 0;
    while ((c = pgm_read_byte(str++))) {
        sLen++;
    }
    return sLen;

} // strlenProgStr

void printProgStr (const char * str)
{
    char c;
    if (!str)   return;
    while ((c = pgm_read_byte(str++))) {
        mySerial.print (c);
    }
    
} // printProgStr


uint16_t readNumber(int textNo, int iLen) {
    char    cIn;
    uint8_t iPos = 0;
    int32_t iVal = 0;
    // --- clear the buffer --
    while (mySerial.available() > 0) mySerial.read();

    mySerial.print(F("\n New value "));
    printProgStr ((const char *) &menuText [textNo]);

    mySerial.print(F(": "));
    mySerial.flush();
    keyPressed    = true;
    while (keyPressed) {
        if (mySerial.available() > 0) {
            cIn = mySerial.read();
            if (iPos < iLen && (   cIn >= 48
                                && cIn <= 57)) {
                iPos++;
                iVal = (iVal * 10) + (cIn - '0');
            }
            if ((cIn == 13) && iPos > 0) {
                keyPressed = false;
            }
        }   // if chars in buffer
    } // while (!keyPressed)
    mySerial.println(iVal);
    keyPressed  = true;
    return iVal;

}   // readNumber()


void addMenuItem(char mId, int textNo, uint32_t mValue) {
    mySerial.print(F("   "));
    mySerial.print(mId);
    for (int S = strlenProgStr((const char *) 
                        &menuText [textNo]); S < 15; S++) {
        if (mId == ' ') mySerial.print(F(" "));
        else            mySerial.print(F("."));
    }
    printProgStr ((const char *) &menuText [textNo]);
    if (mValue != 999) {
        mySerial.print(F(" ["));
        mySerial.print(mValue);
        mySerial.print(F("]"));
    }
    mySerial.println();

}   // addMenuItem()


void setup() {
    mySerial.begin(9600);
    delay(100);
    readConfig();
    if (EEPROM.configVersion != CONFIGVERSION) {
          EEPROM.configVersion  = CONFIGVERSION;
          EEPROM.sensorID       = 1;
          EEPROM.interval       = 30;
          EEPROM.internalRef    = INTERNALREF;
          EEPROM.oscillatorCal  = OSCCAL;
    }
    newConfig = EEPROM;
    OSCCAL    = EEPROM.oscillatorCal;

    SET_INPUT(INTDDR, INTERRUPT_PORTBIT);
    autoCalibrate();

}   // setup()

void loop() {

    if (keyPressed) {
        mySerial.println();
        addMenuItem(' ' , M_VERSION 
                        , newConfig.configVersion);
        addMenuItem('1' , M_SENSORID
                        , newConfig.sensorID);
        addMenuItem('I' , M_INTERVAL
                        , newConfig.interval);
        addMenuItem('C' , M_OSCCAL
                        , newConfig.oscillatorCal);
        addMenuItem('V' , M_NEW_VCC
                        , readVcc(newConfig.internalRef));
        addMenuItem(' ' , M_EEPROM_VCC
                        , readVcc(EEPROM.internalRef));
        addMenuItem(' ' , M_ADC,     readADC());
        addMenuItem(' ' , M_INT_REF
                        , newConfig.internalRef);
        addMenuItem('R' , M_READ_EEPROM, 999);
        addMenuItem('W' , M_WRITE_EEPROM, 999);
        addMenuItem('S' , M_SHOW_MENU, 999);

        mySerial.print(F("\nPress key:"));
        keyPressed = false;
    }
    while (mySerial.available() > 0) {
        // -- read the incoming char:
        charIn = mySerial.read();
        // -- clear rest of the buffer
        while (mySerial.available() > 0) mySerial.read();

        switch (charIn) {
            case '1':   keyPressed   = true;
                        state = SETSENSORID;
                        break;
            case 'c':
            case 'C':   keyPressed   = true;
                        state = CALIBRATE;
                        break;
            case 's':
            case 'S':   keyPressed   = true;
                        state = SHOWMENU;
                        break;
            case 'i':
            case 'I':   keyPressed   = true;
                        state = SETINTERVAL;
                        break;
            case 'r':
            case 'R':   keyPressed   = true;
                        state = READCONFIG;
                        break;
            case 'v':
            case 'V':   keyPressed   = true;
                        state = SETVOLTAGE;
                        break;
            case 'w':
            case 'W':   keyPressed   = true;
                        state = WRITECONFIG;
                        break;
            default:    state = UNKNOWNSTATE;
        }
    }

    switch (state) {
        case SETSENSORID:
            newConfig.sensorID =
                         readNumber(M_SENSORID, 4);
            if (   newConfig.sensorID < 1
                || newConfig.sensorID > 255) {
                    newConfig.sensorID = 1;
            }
            break;
        case CALIBRATE:
            autoCalibrate();
            delay(500);
            displayCalData();
            break;
        case SHOWMENU:
            break;
        case READCONFIG:
            readConfig();
            if (EEPROM.configVersion == CONFIGVERSION) {
                newConfig = EEPROM;
                OSCCAL    = EEPROM.oscillatorCal;
            }
            else EEPROM.internalRef = INTERNALREF;
            break;
        case SETINTERVAL:
            newConfig.interval =
                        readNumber(M_INTERVAL, 4);
            break;
        case SETVOLTAGE:
            VccExtern = readNumber(M_VOLTMETER, 4);
            VccFactor = readVcc(-1);
            newConfig.internalRef =
                        (long)calcARef(VccExtern);
            break;
        case WRITECONFIG:
            newConfig.configVersion = CONFIGVERSION;
            EEPROM = newConfig;
            writeConfig();
            break;

    }   // switch(state)
    state   = UNKNOWNSTATE;
}   // loop()

/*
    Program  : ClockGenerator10kHz v2
  
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

#include <SoftwareSerial.h>

//--- define some common macro's ------------------------
#define SET(a,b)                ((a) |=  _BV(b))
#define SET_OUTPUT(_ddr, _pin)  ((_ddr) |=  _BV(_pin))
#define PB1    1

SoftwareSerial mySerial(10, 11); // RX, TX
char    chIn;

void start10kHz() {
    //-------------------------------------------------
    // With OCR1A = 7 and no prescaler ----------------
    // With F_CPU = 16 MHz, the result is 1 MHz.
    // f = F_CPU / (2 * prescaler * (1+OCR1A))
    // f = 16.000.000 / (2 * 1 * (1 + 7))
    // f = 16.000.000 / 16 ==> 1.000.000Hz => 1MHz
    // With OCR1A = 249 and no prescaler --------------
    // With F_CPU = 16 MHz, the result is 32kHz.
    // f = F_CPU / (2 * prescaler * (1+OCR1A))
    // f = 16.000.000 / (2 * 1 * (1 + 249))
    // f = 16.000.000 / 500 ==> 32.000Hz => 32kHz
    // With OCR1A = 99 and prescaler 8 ----------------
    // With F_CPU = 16 MHz, the result is 10kHz.
    // f = 16.000.000 / (2 * 8 * (1 + 99))
    // f = 16.000.000 / 1600 ==> 10.000Hz => 10kHz
    //-------------------------------------------------
    // Set Timer 1 CTC mode with no prescaling. 
    // OC1A toggles on compare match
    //
    // TCCR1A : Timer/Counter1 Control Register A
    // bit       7        6        5        4      3     2      1      0 
    //      +--------+--------+--------+--------+-----+-----+-------+-------+
    // 0X80 | COM1A1 | COM1A0 | COM1B1 | COM1B0 |  -  |  -  | WGM11 | WGM10 |
    //      +--------+--------+--------+--------+-----+-----+-------+-------+
    // Bit 7:6 - COM1A1:0: Compare Output Mode for Channel A
    // Bit 5:4 - COM1B1:0: Compare Output Mode for Channel B

    // Set Bit 6 : Toggle OC1A on Compare Match
    TCCR1A = ( (1 << COM1A0));

    // TCCR1B : Timer/Counter1 Control Register B
    // bit      7       6      5      4       3       2      1      0 
    //      +-------+-------+-----+--------+------+------+------+------+
    // 0X81 | ICNC1 | ICES1 |  -  | WGM13 | WGM12 | CS12 | CS11 | CS10 |
    //      +-------+-------+-----+--------+------+------+------+------+
    // Bit 4:3 - WGM13:2: Waveform Generation Mode
    // Bit 2:0 - CS12:0: Clock Select

    // Set Bit 3 : Clear Timer on Compare Match
    // Set Bit 0 : No Prescaling
    //TCCR1B = ((1 << WGM12) | (1 << CS10)); // 16MHz
    // Set Bit 1 : clock/8
    TCCR1B = ((1 << WGM12) | (1 << CS11)); // 2MHz

    // TIMSK1 : Timer/Counter1 Interrupt Mask Register
    // bit     7     6      5      4     3       2        1      0 
    //      +-----+-----+-------+-----+-----+--------+--------+-------+
    // 0X6F |  -  |  -  | ICIE1 |  -  |  -  | OCIE1B | OCIE1A | TOIE1 |
    //      +-----+-----+-------+-----+-----+--------+--------+-------+
    // Bit 5 - ICIE1: Timer/Counter1, Input Capture Interrupt Enabled
    // Bit 2 - OCIE1B: Timer/Counter1, Output Compare B Match Interrupt Enabled
    // Bit 1 - OCIE1A: Timer/Counter1, Output Compare A Match Interrupt Enabled
    // Bit 0 - TOIE1: Timer/Counter1, Overflow Interrupt Enabled

    // Clear all bits
    TIMSK1 = 0; // disable compare match register A for timer1

    // This value determines the output frequency
    // Count's ever 250th ClockCycle for 32kHz
    // OCR1A = 249;   
    // Count's ever 100th ClockCycle for 10kHz
    OCR1A = 99;   
    
}   // start10kHz()

void stop10kHz() {
    TCCR1B = 0;
    OCR1A  = 0;
    
}   // stop10kHz)

void ATtinyInfo() {
    Serial.println(F(" ATMEL ATTINY84"));
    Serial.println(F("                           +--V--+ "));
    Serial.println(F("                     VCC  1|     |14  GND"));
    Serial.println(F("               (D10) PB0  2|     |13  AREF (D0)"));
    Serial.println(F("                (D9) PB1  3|     |12  PA1 (D1)"));
    Serial.println(F("               RESET PB3  4|     |11  PA2 (D2)"));
    Serial.println(F(" ->[10kHz] INT0 (D8) PB2  5|     |10  PA3 (D3) [RX]<-"));
    Serial.println(F("            PWM (D7) PA7  6|     |9   PA4 (D4) [TX]->"));
    Serial.println(F("            PWM (D6) PA6  7|     |8   PA5 (D5) PWM"));
    Serial.println(F("                           +-----+\n"));
    Serial.println(F(" ATMEL ATTINY85"));
    Serial.println(F("                           +--V--+ "));
    Serial.println(F("               RESET PB5  1|     |8  VCC"));
    Serial.println(F("         ->[RX] (D3) PB3  2|     |7  PB2 (D2) INT0 [10kHz]<-"));
    Serial.println(F("                (D4) PB4  3|     |6  PB1 (D1)"));
    Serial.println(F("                     GND  4|     |5  PB0 (D0) AREF [TX]->"));
    Serial.println(F("                           +-----+\n"));
    
}   // ATtinyInfo()


void connectInfo() {
    Serial.println(F("\nConnect this Arduno to an ATtiny85/84: \n"));
    Serial.println(F(" ATtiny84             |    Arduino UNO    |     ATtiny85"));
    Serial.println(F(" ----------------     + ------------------+     ---------------"));
    Serial.println(F(" DIL-5 [10kHz in] <-- | [10kHz out] pin 9 | --> DIL-7 [10kHz in]"));
    Serial.println(F(" DIL-9 [TX]       <-- |    [RX] pin 10    | --> DIL-5 [TX]"));
    Serial.println(F(" DIL-10 [RX]      <-- |    [TX] pin 11    | --> DIL-2 [RX]"));
    Serial.println(F(" DIL-1            <-- |       [5V]        | --> DIL-8"));
    Serial.println(F(" DIL-14           <-- |       [GND]       | --> DIL-4"));
    Serial.println(F("\n\npress 'D' for this info, 'S' for tiny Menu\n"));

}   // connectInfo()

void setup() {
    Serial.begin(19200);
    // set the data rate for the SoftwareSerial port
    mySerial.begin(9600);
    delay(200);

    SET_OUTPUT(DDRB, PB1);
    start10kHz();
    Serial.println(F("\n10kHz clock started"));
    ATtinyInfo();
    connectInfo();
    mySerial.write('D');

}   // setup()


void loop()
{
    if (mySerial.available()) {
        Serial.write(mySerial.read());
    }
    if (Serial.available()) {
        chIn = Serial.read();
        if (chIn == 'd' || chIn == 'D') {
            connectInfo();
            Serial.flush();
            delay(100);
        }
        mySerial.write(chIn);
    }

}

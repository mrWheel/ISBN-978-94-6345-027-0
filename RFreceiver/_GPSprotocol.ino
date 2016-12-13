/*
 * Program  : GPS protocol handler
 * Author   : Willem Aandewiel
 */

void processGPS() {
    uint8_t  Header     = bits2Int(0, 3);
    uint16_t ID         = bits2Int(4, 11);
    uint16_t Unit       = bits2Int(12, 14);
    uint8_t  Bat        = bits2Int(15, 15);
    int32_t  ValueInt   = bits2Int(16, 35);
    uint8_t  Decimal    = bits2Int(36, 38);
    char     Label[6];
    Label[0]   = bits2Int(39, 46);
    Label[1]   = bits2Int(47, 54);
    Label[2]   = bits2Int(55, 62);
    Label[3]   = bits2Int(63, 70);
    Label[4]   = '\0';
  //uint8_t parityBit   = bits2Int(69, 69);
    float   decValue    = 0;
    int     tmpDec      = Decimal;
    int     pow10       = 1;
    if (Header == 12) { // Header has to be 'b1100'
        if (bits2Int(16,16) == 1) ValueInt -= 1048576;
        while (tmpDec > 0) {pow10 *= 10; tmpDec--;}
        decValue = (float)ValueInt / pow10;
        Sprint(F("[GPS] "));           Sprint(bits);
        Sprint(F(":: Header: "));      Sprint(Header);
        Sprint(F(",\tID: "));          Sprint(ID);
        Sprint(F(", Unit: "));         Sprint(Unit);
        Sprint(F(", Bat: "));          Sprint(Bat);
        Sprint(F(", Value: "));        Sprint(decValue);
        Sprint(F(" "));                Sprint(Label);
      //Sprint(F(", ParityBit: "))     Sprint(paratyBit);
        Sprintln();
#if !DEBUG
        Serial.print(F("ID: "));       Serial.print(ID);
        Serial.print(F(", Unit: "));   Serial.print(Unit);
        Serial.print(F(", "));         Serial.print(decValue);
        Serial.print(F(" "));          Serial.println(Label);
#endif
    }
    
}   // processGPS()

bool analyseGPS() {
    parityBit   = 0;
    if (inRange(RF433.pulse[0], 4500, 500) ) {
        for(int I=1; I<(RF433.numPulses -1); I+=2) {
            if (inRange(RF433.pulse[I], 250, 150)) {
                if (inRange(RF433.pulse[I+1],900,300)) {
                    addBit('0');
                }
                else if (inRange(RF433.pulse[I+1],1800,400)) {
                    addBit('1');
                    parityBit++;
                }
                else return false;
            }
        }   // for I ..
    }   // inRange SYNC
    if ( (numBits == 72) ) {
        if ((parityBit % 2) == 0) {
            processGPS();
            PROTOCOL = PROTOCOLPROCESSED;
            return PROTOCOL;
        }
        Sprintln(F(" Parity error"));
    }
        
    return false;
    
}   // analyseGPS()


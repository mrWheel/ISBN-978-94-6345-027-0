/*
 * Program  : alecto WS1700 protocol handler
 * Author   : Willem Aandewiel
 */

void processWS1700() {
    uint8_t  Header     = bits2Int(0, 3);
    uint16_t ID         = bits2Int(4, 11);
    uint8_t  Bat        = bits2Int(12, 12);
    uint8_t  TXmode     = bits2Int(13, 13);
    uint8_t  Channel    = bits2Int(14, 15);
    int32_t  TempInt    = bits2Int(16, 27);
    uint8_t  Humidity   = bits2Int(28, 35);
    float   decTemp    = 0;
    if (Header == 5) { // Header has to be 'b0101'
        if (bits2Int(16,16))    TempInt -= 4096;
        decTemp = (float)TempInt / 10.0;
        Sprint(F("[WS1700] "));         Sprint(bits);
        Sprint(F(":: Header: "));       Sprint(Header);
        Sprint(F(",\tID: "));           Sprint(ID);
        Sprint(F(",\tBat: "));          Sprint(Bat);
        Sprint(F(", TXmode: "));        Sprint(TXmode);
        Sprint(F(", Channel: "));       Sprint(Channel);
        Sprint(F(",\tTemp: "));         Sprint(decTemp,1);
        Sprint(F(",\tHumidity: "));     Sprint(Humidity);
        Sprintln();
#if !DEBUG
        Serial.print(F("ID:  "));       Serial.print(ID);
        Serial.print(F(",\tTemp.: "));  Serial.print(decTemp,1);
        Serial.print(F(",\tHumi.: "));  Serial.println(Humidity);
#endif
    }
    
}   // processWS1700()

bool analyseWS1700() {
    if (inRange(RF433.pulse[0], 8900, 500) ) {
        for(int I=1; I<(RF433.numPulses -1); I+=2) {
            if (inRange(RF433.pulse[I], 700, 150)) {
                if (inRange(RF433.pulse[I+1],1900,300)) {
                    addBit('0');
                }
                else if (inRange(RF433.pulse[I+1],3900,400)) {
                    addBit('1');
                    parityBit++;
                }
                //else return false;
            }
        }   // for I ..
    }   // inRange SYNC
    if ( (numBits == 36) ) {
        processWS1700();    
        PROTOCOL = PROTOCOLPROCESSED; 
        return PROTOCOL;
    }
        
    return false;
    
}   // analyseWS1700()


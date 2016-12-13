/*
 * Program  : KAKU protocol handler
 * Author   : Willem Aandewiel
 */

void processKAKU() {
    uint32_t ID     = bits2Int(0, 25);
    uint8_t All     = bits2Int(26, 26);
    uint8_t State   = bits2Int(27, 27);
    uint8_t Unit    = bits2Int(28, 31);
    Sprint(F("[KAKU] "));      Sprint(bits);
    Sprint(F(":: ID: "));      Sprint(ID);
    Sprint(F(",\tUnit: "));    Sprint(Unit);
    Sprint(F(",\tAll: "));     Sprint(All);
    Sprint(F(",\tState: "));   Sprintln(State);
#if !DEBUG
    Serial.print(F("Remote: ")); Serial.print(Unit);
    Serial.print(F(",\tState: "));  
    Serial.println((State == 1) ? "On" : "Off");
#endif
    
}   // processKAKU()

bool analyseKAKU() {
    if (inRange(RF433.pulse[0], 10000, 1000) ) {
        if ( !(inRange(RF433.pulse[1], 250, 100) && 
               inRange(RF433.pulse[2], 2500, 500) ) ) {
            return false;
        }
        for(int I=3; I<(RF433.numPulses -1); I+=4) {
            if (inRange(RF433.pulse[I], 250, 110) && 
                inRange(RF433.pulse[I+1], 1250, 400)) {
                addBit('1');
            }
            else if (inRange(RF433.pulse[I], 250, 100) && 
                     inRange(RF433.pulse[I+3], 1250, 400)) {
                addBit('0');
            }
            else return false;
        }
    }
    if (inRange(RF433.pulse[RF433.numPulses], 10000, 1000) && 
                                numBits == 32) {
        processKAKU();       
        PROTOCOL = PROTOCOLPROCESSED; 
        return PROTOCOL;
    }
    return false;
    
}   // analyseKAKU()



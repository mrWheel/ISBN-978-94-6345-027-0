/*
 * Program  : EverFlourish protocol handler
 * Author   : Willem Aandewiel
 */

void processEverFlourish() {
    uint8_t Unit    = bits2Int(0, 4);
    uint8_t ID      = bits2Int(5, 9);
    uint8_t Check   = bits2Int(10, 10);
    uint8_t State   = bits2Int(11, 11);
    if (Check == 0) { // Check must be Zero??
        Sprint(F("[EverFlourish] "));  Sprint(bits);
        Sprint(F(":: Unit: "));        Sprint(Unit);
        Sprint(F(",\tID: "));          Sprint(ID);
        Sprint(F(",\tCheck: "));       Sprint(Check);
        Sprint(F(",\tState: "));       Sprintln(State);
#if !DEBUG
        Serial.print(F("Remote: ")); Serial.print(ID);
        Serial.print(F(",\tState: "));  
        Serial.println((State == 1) ? "Off" : "On");
#endif
    }
    
}   // processEverFlourish()

bool analyseEverFlourish() {
    if (inRange(RF433.pulse[0], 11500, 1000) ) {
        for(int I=1; I<(RF433.numPulses -1); I+=4) {
            //---- 1st Short, 3e Short == '0'
            if (inRange(RF433.pulse[I], 350, 100) && 
                inRange(RF433.pulse[I + 2], 350, 100)) {
                addBit('1');
            }
            //---- 1st Long, 3e Long == '0'
            else if (inRange(RF433.pulse[I], 1000, 200) && 
                     inRange(RF433.pulse[I + 2], 1000, 200)) {
                addBit('0');
            }
            //---- 1st Short, 3e Long == '0'
            else if (inRange(RF433.pulse[I], 350, 100) && 
                     inRange(RF433.pulse[I + 2], 1000, 200)) {
                addBit('0');
            }
            else return false;
        }
    }
    if (inRange(RF433.pulse[RF433.numPulses], 11500, 1000) && 
                                numBits == 12) {
        processEverFlourish();
        PROTOCOL = PROTOCOLPROCESSED; 
        return PROTOCOL; 
    }
    return false;
    
}   // analyseEverFlourish()


/*
 * Program  : IMPULS protocol handler
 * Author   : Willem Aandewiel
 */

void processImpuls() {
    uint8_t Unit    = bits2Int(0, 4);
    uint8_t ID      = bits2Int(5, 9);
    uint8_t State   = bits2Int(10, 10);
    uint8_t Check   = bits2Int(11, 11);
    if (Check == State) return; // Check is inverse State
    Sprint(F("[Impuls] "));        Sprint(bits);
    Sprint(F(":: Unit: "));        Sprint(Unit);
    Sprint(F(",\tID: "));          Sprint(ID);
    Sprint(F(",\tState: "));       Sprint(State);
    Sprint(F(",\tCheck: "));       Sprint(Check);
    Sprintln();
#if !DEBUG
    Serial.print(F("Remote: ")); Serial.print(ID);
    Serial.print(F(",\tState: "));  
    Serial.println((State == 1) ? "On" : "Off");
#endif
    if (Unit == 31 && ID == 23) {
        if (State)  SET(_PORT_REG, IMPULS_B_LED_PIN);    
        else        CLEAR(_PORT_REG, IMPULS_B_LED_PIN);
    }
}   // processImpuls()


bool analyseImpuls() {
    if (inRange(RF433.pulse[0], 5500, 1000) ) {
        for(int I=1; I<(RF433.numPulses -1); I+=4) {
            //--[SxSx]--------------------------------
            if (inRange(RF433.pulse[I], 200, 100) && 
                inRange(RF433.pulse[I+2], 200, 100)) {
                addBit('0');
            }
            //--[LxLx]-------------------------------------
            else if (inRange(RF433.pulse[I], 550, 100) && 
                     inRange(RF433.pulse[I+2], 550, 100)) {
                addBit('1');
            }
            //--[SxLx]-------------------------------------
            else if (inRange(RF433.pulse[I], 200, 100) && 
                     inRange(RF433.pulse[I+2], 550, 100)) {
                addBit('1');
            }
            else return false;
        }
    }
    if (inRange(RF433.pulse[RF433.numPulses], 5500, 1000) && 
                                        numBits == 12) {
        processImpuls();
        PROTOCOL = PROTOCOLPROCESSED; 
        return PROTOCOL; 
    }
    return false;
    
}   // analyseImpulse()


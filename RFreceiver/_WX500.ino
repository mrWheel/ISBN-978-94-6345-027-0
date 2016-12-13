/*
 * Program  : alecto WS1700 protocol handler
 * Author   : Willem Aandewiel
 */

void processWX500() {
    uint16_t ID         = bits2IntRev(0, 7);
    uint8_t  Bat        = bits2IntRev(8, 8);
    uint8_t  Type       = bits2IntRev(9, 10);
    uint8_t  TXmode     = bits2IntRev(11, 11);
    //uint8_t  HumOnes    = bits2IntRev(24,27);
    //uint8_t  HumTens    = bits2IntRev(28,31);
    uint8_t  Humidity   = (bits2IntRev(28, 31) * 10) + bits2IntRev(24,27);
    //uint32_t intTemp    = (uint32_t)(bits2IntRev(12, 23));
    float    decTemp    = (float)(bits2IntRev(12, 23) / 10.0);
      if (Type != 3) { // Can be temp/humi
        if ((int)bits2IntRev(23,23) == 1) {   // TempInt -= 4096;
            decTemp -= 409.6;
        }
        Sprint(F("[WX500] "));      Sprint(bits);
        if ((ID != 0) && Humidity != 0) {
            Sprint(F(":: ID: "));       Sprint(ID); 
            Sprint(F(",\tBat: "));      Sprint(Bat);
            Sprint(F(",\tType "));      Sprint(Type);
            Sprint(F(", TXmode: "));    Sprint(TXmode);
            Sprint(F(",\tTemp: "));     Sprint(decTemp,1);
            Sprint(F(",\tHumidity: ")); Sprint(Humidity);
            /**
            delay(300);
            Sprint(" Translate to GPS protocol ");
            GPSsend(ID, 2, Bat,(int32_t)(Humidity), 0, "%RH" );
            GPSsend(ID, 1, Bat,(int32_t)(decTemp * 100), 2, "*C" );
            ***/
        }
        Sprintln();
#if !DEBUG
        Serial.print(F("ID:  "));           Serial.print(ID);
        Serial.print(F(",\tTemp.: "));      Serial.print(decTemp,1);
        Serial.print(F(",\tHumidity: "));   Serial.println(Humidity);
#endif
    }
    
}   // processWX500()

bool analyseWX500() {
    if (inRange(RF433.pulse[0], 8900, 250) && (RF433.numPulses == 74)) {
        //Sprint("WX500 - numPulses: "); Sprintln(RF433.numPulses);
        //for(int I=1; I<(RF433.numPulses -1); I++) {
        //    Sprint(RF433.pulse[I]); Sprint(" ");
        //}
        //Sprintln();
        for(int I=1; I<(RF433.numPulses -1); I+=2) {
            if (RF433.pulse[I] < 650) {
                if (RF433.pulse[I+1] > 3125) {
                    addBit('1');
                    parityBit++;
                }
                else {
                    addBit('0');
                }
            }
        }   // for I ..
    }   // inRange SYNC
    if ( (numBits == 36) ) {
        processWX500();    
        PROTOCOL = PROTOCOLPROCESSED; 
        return PROTOCOL;
    }
        
    return false;
    
}   // analyseWX500()


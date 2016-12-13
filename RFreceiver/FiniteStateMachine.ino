/*
 * Program  : FiniteStateMachine
 * Author   : Willem Aandewiel
 */

//----- Interrupt Service Routine ------[FSM]-----------------
void interruptSR() {
    // lastPulse is the duration between 2 interrupts in μsecs
    uint8_t         pulseType = UNKNOWNpulse;
    static uint16_t lastTime;
    uint16_t        lastPulse = micros() - lastTime;
    lastTime += lastPulse;

    //--- determine the type of Pulse received -------
    if (lastPulse > minSync && lastPulse < maxSync) {
            pulseType   = SYNCpulse;    // SYNC Pulse
    }
    else if (lastPulse > minPulse && lastPulse < maxPulse)  {
            pulseType   = DATApulse;    // Genuin data Pulse
    }
    else {
            pulseType   = INVALIDpulse; // invallid Pulse
    }

    //----- enter Finite State Machine --------------------
    if (RF433.state != DONE) {
        switch(RF433.state) {
            case UNKNOWN:   
                switch(pulseType) {
                    case SYNCpulse: // SYNC received
                        RF433.state     = SYNC;
                        RF433.numPulses = 0;
                        RF433.pulse[RF433.numPulses] = 
                                                lastPulse;
                        minSync     = lastPulse - 100;
                        maxSync     = lastPulse + 100;
                        minPulse    = minSync / 55;
                        maxPulse    = minSync / 2;
                        break;

                    default:
                        RF433.state = UNKNOWN;
                        RF433.numPulses = 0;
                        RF433.pulse[RF433.numPulses]  = 0;
                        minSync         = SYNC_LO_LIMIT;
                        maxSync         = SYNC_HI_LIMIT;
                        break;
                }
                break;
                        
            case SYNC:      
                switch(pulseType) {
                    case DATApulse: // Genuine Data Pulse
                        RF433.state = DATA;
                        RF433.pulse[++RF433.numPulses] = 
                                                lastPulse;
                        break;
                    case SYNCpulse: // SYNC received
                        RF433.state     = SYNC;
                        RF433.numPulses = 0;
                        RF433.pulse[RF433.numPulses] = 
                                                lastPulse;
                        minSync     = lastPulse - 100;
                        maxSync     = lastPulse + 100;
                        minPulse    = minSync / 55;
                        maxPulse    = minSync / 2;
                        break;
                        
                    default: // SYNC or invalid pulse
                        RF433.state = UNKNOWN;
                        RF433.numPulses = 0;
                        RF433.pulse[0]  = 0;
                        minSync         = SYNC_LO_LIMIT;
                        maxSync         = SYNC_HI_LIMIT;
                        break;
                }
                break;
                    
            case DATA:  
                switch(pulseType) {
                    case DATApulse: // Genuine Data Pulse
                        RF433.state = DATA;
                        RF433.pulse[++RF433.numPulses] =
                                                lastPulse;
                        //--- sanaty check ----------------
                        if (RF433.numPulses > (MAXPULSES -1)) {
                            RF433.state     = DONE;
                        }
                        break;
                    case SYNCpulse: // SYNC received
                        RF433.state = DONE;
                        RF433.pulse[++RF433.numPulses] =
                                                lastPulse;
                        break;
                        
                    default: // invalid pulse
                        RF433.state = UNKNOWN;
                        RF433.numPulses = 0;
                        RF433.pulse[0]  = 0;
                        minSync         = SYNC_LO_LIMIT;
                        maxSync         = SYNC_HI_LIMIT;
                        break;
                }
                break;
                        
            case DONE:  // nothing to do (here)!
                break;
                    
            default:// invallid pulse length
                    RF433.state     = UNKNOWN;
                    RF433.numPulses = 0;
                    RF433.pulse[RF433.numPulses]  = 0;
                    minSync         = SYNC_LO_LIMIT;
                    maxSync         = SYNC_HI_LIMIT;
                    break;
                        
        }   // switch(RF433.state)
        
    }
    
}   // interruptSR()


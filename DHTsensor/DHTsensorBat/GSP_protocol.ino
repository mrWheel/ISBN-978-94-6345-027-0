/*
** Module   : GSP_protocol - General Sensor Protocol
** Author   : Willem Aandewiel
*/

//--- protocol info ---
#define SYNC   4500
#define START   250
#define SHORT   900
#define LONG   1800

static int parityBit;

// --- LeadIn to set receiver ---
void sendLeadIn()
{
    for (int P = 0; P < 5; P++) {
        SET(TXPORT, TXPORT_BIT);
        delayMicroseconds(25);
        CLEAR(TXPORT, TXPORT_BIT);
        delayMicroseconds(25);
    }

} // sendLeadIn()

// --- send SYNC pulse ---
void sendSYNC()
{
    SET(TXPORT, TXPORT_BIT);
    delayMicroseconds(START);
    CLEAR(TXPORT, TXPORT_BIT);
    delayMicroseconds(SYNC);

} // sendSYNC()

// --- send integer ---
void sendInt(int32_t data, int bitsWanted) {
    Sprint(" ("); Sprint(data); Sprint(")");
    uint32_t mask;
    for (int b = 1; b <= bitsWanted; b++) {
        mask  = 1;
        mask <<= (bitsWanted - b);
        if (data & mask) {
            parityBit++;
            Sprint("1");
            SET(TXPORT, TXPORT_BIT);
            delayMicroseconds(START);
            CLEAR(TXPORT, TXPORT_BIT);
            delayMicroseconds(LONG);
        }
        else {
            Sprint("0");
            SET(TXPORT, TXPORT_BIT);
            delayMicroseconds(START);
            CLEAR(TXPORT, TXPORT_BIT);
            delayMicroseconds(SHORT);
        }
    }   // for ..

} // sendInt()


void sendString(char* data, int charsWanted) {
    for (int c = 0; c < charsWanted; c++) {
        if (c > strlen(data)) {
            sendInt(0, 8);
        }
        else {
            sendInt(data[c], 8);
            Sprint("("); Sprint(data[c]); Sprint(")");
        }
    }   // for ..

} // sendString()


void send(uint8_t sensorID, uint8_t Unit
          , uint8_t Bat, int32_t Value
          , int8_t Decimals, char *Label ) {

    sendLeadIn();   // tune receiver
    parityBit = 0;
    // repeat pulse-train 5 times
    for (int L = 0; L < 5; L++) {
        Sprint("Bits [");
        sendSYNC();
        sendInt(0b1100, 4);  // Header
        sendInt(sensorID, 8);
        sendInt(Unit, 3);
        sendInt(Bat, 1);     // Batery status
        sendInt(Value, 20);
        sendInt(Decimals, 3);
        sendString(Label, 4);
        if ((parityBit % 2) == 0) sendInt(0, 1);
        else                      sendInt(1, 1);
        Sprintln(" ]");
    } // for L ...

} // send()



/*
 * Module   : sensorHelpers
 * Author   : Willem Aandewiel
 */

#if __AVR_ATtiny85__
    void  lowPower(seconds_t period)
    {
        // --- Disable ADC ---
        ADCSRA &= ~(1 << ADEN);
        // --- and the rest ---
        power_all_disable ();
        wdt_enable(period);
        WDTCSR |= (1 << WDIE);    

        do {                         
            set_sleep_mode(SLEEP_MODE_PWR_DOWN); 
            cli();                
            sleep_enable();  
            sei();                
            sleep_cpu();          
            sleep_disable();      
            sei();                
        } while (false);

        power_all_enable();
        // --- Enable ADC ---
        ADCSRA |= (1 << ADEN);

    }   // lowPower()


    //------------------------------------------
    // Watchdog Timer interrupt service routine. 
    // This routine is required to allow 
    // automatic WDIF and WDIE bit clearance.
    //------------------------------------------
    ISR (WDT_vect)
    {
        // WDIE & WDIF cleared in hardware upon entering this ISR
        wdt_disable();
    
    }   // ISR()

    void goToSleep(unsigned int seconds) {
        while(seconds >= 8) {
            lowPower(SLEEP_8S); 
            seconds -= 8;
        }
        while (seconds >= 1) {
            lowPower(SLEEP_1S);
            seconds -= 1;
        }

    }   // goToSleep()
#else
    void goToSleep(unsigned int seconds) {
        Sflush();
        delay(seconds * 1000);
    
    }   // goToSleep()
#endif

void blinkLed(int mode) {
    if (mode == 0) {
        while(1) {   // never stop blinking
            SET(LEDPORT,LEDPORT_BIT);
            delay(50);
            CLEAR(LEDPORT,LEDPORT_BIT);
            delay(1450);
        }
    }
    for(int i = 0; i< mode; i++)  {
        SET(LEDPORT,LEDPORT_BIT);
        delay(200);
        CLEAR(LEDPORT,LEDPORT_BIT);
        delay(200);
    }

} // blinkLed()


static void loadConfig () {
    //Sprintln(F("\nRead EEPROM configuration"));
    eeprom_read_block(&EEPROM, EEPROM_ADDR, sizeof EEPROM);
    if (EEPROM.configVersion != CONFIGVERSION) {
        EEPROM.sensorID      = 11;
        EEPROM.interval      = 40;
        EEPROM.internalRef   = INTERNALREF;
        EEPROM.oscillatorCal = OSCCAL;
    }
    OSCCAL = EEPROM.oscillatorCal;
    
}   // loadConfig()



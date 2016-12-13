De Atmel Microprocessoren hebben een paar eigenschappen 
die aandacht nodig hebben. Eén daarvan is de nauwkeurigheid 
van de interne clock (RC-oscillator) en de andere is de 
nauwkeurigheid van de interne referentie spanning van 1,1 Volt.
Iets anders dat we in dit hoofdstuk behandelen is het opslaan 
van de Sensor-module en chip specifieke eigenschappen in het 
EEPROM geheugen van de chip. Hierdoor hoeven we deze gegevens 
per chip maar één maal vast te leggen en kunnen we de AVR 
daarna steeds opnieuw programmeren zonder ons hier verder nog 
zorgen over te maken.

Dit programma werkt alleen in samenwerking met het programma
"ClockGenerator10kHz"!

Het headerfile "mySensor.h" moet onder de Arduino IDE map
"/Library/mySensor/" worden geplaats (zie het hoofdstuk
"Atmel AVR Microprocessoren tunen")

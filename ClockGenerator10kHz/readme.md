Volgens Atmel is de afwijking van de interne RC-oscillator 
ongeveer ± 1%. Maar af-fabriek is de nauwkeurigheid slechts 
±10%, wat voor de timing van draadloze Sensor-modules helaas 
té onnauwkeurig is. De AVR heeft echter een speciaal register 
waarin de kalibratie gegevens van de fabriek ligt opgeslagen 
en die programmatisch kan worden aangepast. Dit register heet 
OSCCAL. Door OSCCAL groter te maken gaat de interne clock 
sneller lopen, maken we hem kleiner dan gaat de interne clock 
langzamer lopen.
Met de nodige meetapparatuur zoals een frequentie counter of 
een oscilloscoop is het dus goed mogelijk om de interne 
RC-oscillator te fine-tunen tot op een nauwkeurigheid van 
±1%. Omdat niet iedereen de beschikking heeft over dit soort 
(specialistische en dure) meet instrumenten gaan we gebruik 
maken van de nauwkeurigheid van een Arduino boardje. 
De originele boardjes zijn uitgerust met een crystal van 
16MHz of 20MHz die een nauwkeurigheid heeft van ongeveer ±0,1%. 

Dit programma doet zijn werk alleen in combinatie met het programma
"sensorConfigurator" zoals beschreven in het hoofdstuk "Atmel AVR Microprocessoren tunen"

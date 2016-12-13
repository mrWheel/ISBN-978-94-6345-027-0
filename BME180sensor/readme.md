Onze eerste Sensor-module maakt gebruik van een sensor 
(DHT11/DHT12) die met de AVR communiceert via een Single-
Wire Two-Way Interface. Anders gezegd: alle communicatie 
tussen de AVR en de sensor loopt over één draadje. Wil je 
meerdere van dit soort sensoren op de AVR aansluiten, dan 
heb je voor iedere sensor een aparte I/O pin nodig. Er 
zijn ook sensoren die via een I2C (Inter-Integrated 
Circuit) bus met twee draadjes werken, waarbij op dezelfde 
twee draadjes meerdere sensoren kunnen worden aangesloten 
(vandaar de term ‘bus’). I2C werkt op basis van twee bus 
lijnen, namelijk SDA (serial data) en SCL (serial clock). 
Over de SDA-lijn wordt de data verzonden en over de SCL-
lijn wordt een kloksignaal verzonden. Op deze twee lijnen 
kunnen meerdere apparaten worden aangesloten omdat ieder 
apparaat (in ons geval voornamelijk sensoren) een eigen 
adres heeft.

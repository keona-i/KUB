/*
 * Die jeweiligen Senosren MHZ19B und BME/BMP280, sind mit den Esp32 LoraWan verbunden.
 * Die Sensordaten werden via TTN CayenneLPP nach den Gatway versendet und so weitergeleitet auf die TTN- website.
 * 
 * Für die Heltec Esp32 LoraWan board:
 * https://resource.heltec.cn/download/package_heltec_esp32_index.json
 * 
 *
 * 
 * Im Code:
 * 
 * Alle benötigte Bibliotheken werden eingebunden. 
 * 
 * ->  Heltec ESP32 Dev-Boards by Heltec Automation
 * ->  Lora   = LoRa by Sandeep Mistry 
 * 
 * ->  TTN    = TTN_esp32 by Francois Riotte
 * ->  CayenneLPP = CayenneLPP by Electronic Cats
 * 
 * ->  BME280 = Adafruit_BME280 by Adafruits
 * ->  I2C    =  SoftWire by Steve Marple
 * ->  Adafruit = Adafruit Unified Sensor by Adafruit
 * 
 * ->  MHZ19  = MH-Z19 by Jonathan Dempsey
 * ->  MHZ19  = MH-Z CO2 Sensors by Tobias Schürg, Andreas Horn
 * 
 * ->  Neopixel = Adafruit NeoPixel by Adafruit
 * 
 *  
 * Pins werden difiniert (sehe Steckplan: KUB).
 * 
 * 
 * _________________________________________________________________________________________________________________________________________________________________________
 * 
 * Durch die devEui, appEui und appKey die auf der TTN-Website (Profil > Application > "Projekt" -Application-name ) zufinden sind.
 * Sind wie Schlüssel die bei jedes Device anderes vergeben werden. So wird sichergestellt das nur der Lorawan der gerade benutz wird  
 * mit dem gleichen "Schlüssel-Nummer" angesprochen wird.
 * 
 * Startet man den Esp32-LoraWan sucht dies eine Verbindung mit dem nächstgelegen Gatway.
 * Ist die Verbindung vorhanden werden die Sensor Daten mit Payloads versendet.
 * Die 16 Bytes wurden aufgeteilt auf die 3 Sensor-Werte (PPM, Luftfeutigkeit & Temperature) mit jeweils high Bytes und low Bytes.
 * Dabei bekommen die jeweiligen Sensordaten ein eigene Nummer die so an TTN gesendet werden.
 * Mit den z.B. lpp.addaddTemperature(2, Temp), werden die HEX-Zahlen decodiert und in TTN mit Dezimal und ganze Zahlen angezeigt.
 * 
 * 
 * Die PPM (Co2)-Werte sind mit dem void co2Warung(int CO2) verbunden. Das heißt liest der MHZ19B Sensor einen Bestimmten Wert, 
 * wird dies in einer Farbe angezeigt und dient als Wahnrung: 
 *  - Ist der PPM-Wert unter 850, bleibt es grün ( Grün steht für gute Luftqualität, es muss Nicht gelüftet werden).
 *  - Ist der PPM-Wert unter 1400, bleibt es gelb ( Gelb steht für die Luftqualität geht noch, es muss nicht ungebingt gelüftet werden).
 *  - Ist der PPM-Wert unter 1800, bleibt es magenta ( magenta steht für die Luftqualität ist Schelcht, soll dringend gelüftet werden).
 *  - Ist der PPM-Wert über 1800, wird es rot ( rot steht für die Luftqualität extrem Schelcht, soll dringend gelüftet werden und nicht in dem raum aufgehalten werden).
 *  Wird eine diese Werte erreicht, leuchten die NeoPixel (RGB-LED's) auf und zeigen sich in die jeweiligen Farbe die angeben werden.
 *  
 *  
 * 
 */

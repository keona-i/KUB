//Bibliothek für die The Things Network verbindung 

#include <TTN_esp32.h>
#include "TTN_CayenneLPP.h"
#include <CayenneLPP.h>
/***************************************************************************
   Geh auf die TTN console und registere ein Device. 
   Kopiere die devEui, appEui & appKey und fügen sie HIER ein.
    
 ****************************************************************************/
const char* devEui = "00FAA2624C0846ED"; // Change to TTN Device EUI
const char* appEui = "70B3D57ED003938E"; // Change to TTN Application EUI
const char* appKey = "214A19CD94BA39754D3389CFF2C0470C"; // Chaneg to TTN Application Key

TTN_esp32 ttn ;
TTN_CayenneLPP lpp;
/*****************************************************************************/
//BME
//Bibliothek für die BME/BMP280 Sensor

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#define I2C_SDA 33
#define I2C_SCL 32

#define SEALEVELPRESSURE_HPA (1013.25)

TwoWire I2CBME = TwoWire(0);
Adafruit_BME280 bme;
unsigned long delayTime;
/****************************************************************/
//Co2 sensor
//Bibliothek für den MH-Z19B Sensor

#include <Arduino.h>
#include "MHZ19.h"

#define RX_PIN 12
#define TX_PIN 13

#define BAUDRATE 9600

MHZ19 myMHZ19;

unsigned long getDataTimer = 0;

int CO2;
/****************************************************************/
//Led
//Bibliothek für den Neopixel 

#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif

#define  PIN    25
#define  NUMPIXELS 9


Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
#define DELAYVAL 500

/****************************************************************/
//Lora:
// 

void message(const uint8_t* payload, size_t size, int rssi)
{
  Serial.println("-- MESSAGE");
  Serial.print("Received " + String(size) + " bytes RSSI=" + String(rssi) + "db");
  for (int i = 0; i < size; i++)
  {
    Serial.print(" " + String(payload[i]));
    // Serial.write(payload[i]);
  }
  Serial.println();
}



void setup()
{
  //Co2-Code:
  //Startet den MHZ19B Sensor
  Serial.begin(115200);
  Serial1.begin (BAUDRATE, SERIAL_8N1, RX_PIN , TX_PIN);
  myMHZ19.begin(Serial1);
  
  myMHZ19.autoCalibration();
  
  
  //Lora:
  //Baut die TTN verbindung auf 
  Serial.begin(115200);
  Serial.println("Starting");
  ttn.begin();
  ttn.onMessage(message); // Declare callback function for handling downlink
  // messages from server
  ttn.join(devEui, appEui, appKey);
  Serial.print("Joining TTN ");
  while (!ttn.isJoined())
  {
    Serial.print(".");
    delay(500);
  }
  Serial.println("\njoined !");
  ttn.showStatus();

  
  //BME
  //Startet den BME Sensor
  Serial.println(F("BME280 test"));
  I2CBME.begin(I2C_SDA, I2C_SCL, 100000);
  bool status;
  // default settings
  // (you can also pass in a Wire library object like &Wire2)
  status = bme.begin(0x76, &I2CBME);
  if (!status)
  {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1);
  }
  Serial.println("-- Default Test --");
  delayTime = 1000;
  Serial.println();
  
  #if defined(__AVR_ATtiny85__) && (F_CPU == 16000000)
  clock_prescale_set(clock_div_1);
  #endif
  pixels.begin();

  
}

void loop() {
  //Co2-Code:
  //Frägt und liest die MHZ19B Sensor-Daten ab.
  if (millis() - getDataTimer >= 2000)
  {
    int CO2 = myMHZ19.getCO2();
    /* note: getCO2() default is command "CO2 Unlimited". This returns the correct CO2 reading even
      if below background CO2 levels or above range (useful to validate sensor). You can use the
      usual documented command with getCO2(false) */
    CO2 = myMHZ19.getCO2();                             // Request CO2 (as ppm)
    Serial.print("CO2 (ppm): ");
    Serial.println(CO2);
    
    getDataTimer = millis();

    //Nimmt sich die Temperatur- und Luftfeuchtigkeitsdaten von printValue().
    int Temp = bme.readTemperature();
    int Humi = bme.readHumidity();


    //sendet die Werte vom MHZ19B & BME Sensoren anch TTN, in ganzen Zahlen (decodiert es geleich).
    lpp.reset();
    lpp.addLuminosity(1, CO2); // Für den CO2-Wert ist Luminosity besser,da einen höheren Zahlen-Wert (0-5000) nach TTN versenden werden kann.
    lpp.addTemperature(2, Temp); // Für Temperatur-Wert. 
    lpp.addRelativeHumidity(1, (bme.readHumidity()));// Für die Luftfeuchtigkeit-Wert.
    ttn.sendBytes(lpp.getBuffer(), lpp.getSize()); // verschickt die Bytes an TTN.


    
    // Die 16 Bytes werden auf die 3 Sensor-Wert aufgeteilt mit high bytes und low bytes 
    // Die datenpackete werden, dann nach TTN versendet.
     
    byte payload[6];
    payload[0] = highByte(CO2);
    payload[1] = lowByte(CO2);
    payload[2] = highByte(Temp);
    payload[3] = lowByte(Temp);
    payload[4] = highByte(Humi);
    payload[5] = lowByte(Humi);
    
    if (ttn.sendBytes(payload, 6))
    {
      Serial.printf("CO2: %d ppm - TTN_CayenneLPP: %02X502X\n", CO2, payload[0], payload[1]); // CO2-Wert bekommt eine eigende "Payload-Nummer" 
      Serial.printf("Temperatur: %d C° - TTN_CayenneLPP: %02X%02X\n", Temp, payload[2], payload[3]); // Temperatur bekommt eine eigende "Payload-Nummer"
      Serial.printf("Humidity: %d - TTN_CayenneLPP: %02X602X\n", Humi, payload[4], payload[5]); // Luftfeuchtigkeit bekommt eine eigende "Payload-Nummer"
    }
    
    delay(10000);

        
    //bme
    //Zeigt die Werte von printValues() an.
    printValues();
    delay(delayTime);
    
    // led
    //Die LED´s werden mit den CO2-PPM-Wert verbunden.
    co2Warung(CO2);

     
  
  }
}
 
void printValues() 
{
  //Liest die Termeratur-Wert vom BME-Sensor ab.
  Serial.print("Temperature = ");
  Serial.print(bme.readTemperature());
  Serial.println(" *C");
  
  /*// Convert temperature to Fahrenheit
    Serial.print("Temperature = ");
    Serial.print(1.8 * bme.readTemperature() + 32);
    Serial.println(" *F");*/

  //Liest die Druck-Wert vom BME-Sensor ab.
  Serial.print("Pressure = ");
  Serial.print(bme.readPressure());
  Serial.println(" hPa");
  
  /*Serial.print("Approx. Altitude = ");
  Serial.print(bme.readAltitude(SEALEVELPRESSURE_HPA));
  Serial.println(" m");*/
  
  //Liest die Luftfeuchtigkeits-Wert vom BME-Sensor ab.
  Serial.print("Humidity = ");
  Serial.print(bme.readHumidity());
  Serial.println(" %");
  
  Serial.println();
  
}



void co2Warung(int CO2) 
{
  if (CO2 < 850) {
    //setPixels(pixels, 0, 0, 0);
    //wenn der Wert unter 850 ppm ist dan wird es..
    setPixels(pixels, 0, 255, 0); // Grün
    Serial.println(" grün ");
    Serial.print(CO2);
  } else if (CO2 < 1400) {
    //wenn der Wert unter 1400 ppm ist dan wird es..
    setPixels(pixels, 255, 230, 0); // gelb
    Serial.println("gelb");
    Serial.print(CO2);
  } else if (CO2 < 1800) {
    //wenn der Wert unter 1800 ppm ist dan wird es..
    setPixels(pixels, 140, 0, 84);// magenta 
    Serial.println("magenta");
    Serial.print(CO2);
  } else {
    //wenn der Wert über 1800 ppm ist dan wird es..
    setPixels(pixels, 255, 0, 0); //Rot
    Serial.println("Rot");
    Serial.print(CO2);
  }
}
 
void setPixels(Adafruit_NeoPixel& pixel, int r, int g, int b)  // Damit die Neopixel in aller Farben aufleuchten können (rot/grün/blau = wird gemischt)
{
  int c = pixel.Color(r, g, b);
  for (uint16_t i = 0; i < pixel.numPixels(); i++)  //das alle Pixel angesprochen werden 
  {
    pixel.setPixelColor(i, c);
    pixel.setBrightness(50);//helligkeit
  }
  pixel.show(); // Das die farben werden auf den Neopixel angezeigt und leuchten in der Farbe.
}

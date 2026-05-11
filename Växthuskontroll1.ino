
#include <Adafruit_SHT31.h>
#include <Arduino.h>
#include <U8g2lib.h>

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif

// Please UNCOMMENT one of the contructor lines below
// U8g2 Contructor List (Frame Buffer)
// The complete list is available here: https://github.com/olikraus/u8g2/wiki/u8g2setupcpp
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);
// End of constructor list

Adafruit_SHT31 sht31 = Adafruit_SHT31();

#define uS_TO_S_FACTOR 1000000ULL /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  40          /* Time ESP32 will go to sleep (in seconds) */

int Open = 20;
int Close = 21;
int Moist_sens = 0;  // The input pin for the Moisture sensor 
int Moist_pot =1; //Pin connected to potentiometer to set minimum moisture
int TemppotPin = 2; // Pin connected to potentiometer to set target temp
int Watervalve = 7; //Pin that opens watervalve
int dryLED = 6; //Red LED indicating dry soil

int MoistsensorValue = 0; // variable to store the value coming from the sensor

void setup() {
  Serial.begin(9600);                                                                                                                       
  delay(100);
  u8g2.begin();

  pinMode (Moist_pot, INPUT);
  pinMode(Moist_sens, INPUT);
  pinMode(Open, OUTPUT);
  pinMode(Close, OUTPUT);

  digitalWrite(Open, HIGH);
  digitalWrite(Close, HIGH);

  // initialize the OLED object
  Serial.println("SHT31 test");
  if (!sht31.begin(0x44)) { // Set to 0x45 for alternate i2c addr
    Serial.println("Couldn't find SHT31");
    while (1) delay(1);
  }
}

void loop() {
  float t = sht31.readTemperature();
  float h = sht31.readHumidity();

    // getting moisture value
  //float Moisture = readSoilMoisture(Moist_sens); 
    // read the value from the Moistsensor: 
  int MoistsensorValue = analogRead(Moist_sens);
  // coverting value in Scale of 0 to 100
  MoistsensorValue = map(MoistsensorValue,1023,0,0,100);

  // Read moisture target potentiometer and map it to minimum soil moisture
  int MoistpotValue = analogRead(Moist_pot);
  float Minmoist = map(MoistpotValue, 0, 4095, 0, 100);

  Serial.print("Moisture : ");
  Serial.print(MoistsensorValue);
  Serial.println("%");
  Serial.print ("Min Moisture");
  Serial.print (Minmoist);
  
  u8g2.clearBuffer();	// clear the internal display memory
  u8g2.drawStr(10, 30, ("Soil moisture = " + String(MoistsensorValue) + " %").c_str());

  // Read temp target potentiometer value and map it to temperature range
  int TemppotValue = analogRead(TemppotPin);
  float Maxtemp = map(TemppotValue, 0, 4095, 10, 30);
  float Mintemp = Maxtemp - 4;
  
  if (!isnan(t)) { // check if 'is not a number'
    
    Serial.print("Set Temp °C = ");Serial.print (Maxtemp);
    Serial.print("Temp °C = "); Serial.print(t); Serial.print("\t\t");

    u8g2.setFont(u8g2_font_crox1cb_tr);	// choose a suitable font
    u8g2.drawStr(0,25,"Reading...");	// write something to the internal memory
    u8g2.sendBuffer();					// transfer internal memory to the display
    delay(2000);   
    u8g2.drawStr(010, 10, ("Temp. = " + String(t) + " °C").c_str());
    u8g2.setFont(u8g2_font_crox2cb_tf);	// choose another font
    u8g2.drawStr(10, 20, ("Target = " + String(Maxtemp)).c_str());
    u8g2.sendBuffer();
  } 
  else {
    Serial.println("Failed to read temperature");
    u8g2.drawStr(0,10,"Temp failure.");
  }
  if (!isnan(h)) { // check if 'is not a number'
    Serial.print("Hum. % = "); Serial.println(h);
    u8g2.drawStr(15, 50, (String(h) + " %").c_str());
  } 
  else {
    Serial.println("Failed to read humidity" );
    u8g2.drawStr(0,25,"Hum failure.");
   
  }
   u8g2.sendBuffer();

  if (MoistsensorValue < MoistpotValue) { digitalWrite, (Watervalve, LOW);
  digitalWrite, (dryLED, LOW);
  delay(4000);
  digitalWrite, (Watervalve, HIGH);
  }
  else {
  digitalWrite, (dryLED, HIGH);
  }
  if (t > Maxtemp) {
    digitalWrite(Open, LOW);
    delay(500);
    digitalWrite(Open, HIGH);
  }
  if (t < Mintemp) {
    digitalWrite(Close, LOW);
    delay(500);
    digitalWrite(Close, HIGH);
  }
  delay(10000);
}

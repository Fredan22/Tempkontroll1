//This project is meant to monitor the temperature, humidity and soil moisture
//in a tiny greenhouse. Then monitor the result on an Oled display.
//Also if the temperature is above, or below the target temperature,
//open or close the windows by a geared DC motor.
//and if the soil is more dry than target moisture, open a watervalve.
//Target temp and soil moisture is set via two potentiometers.
//To save energy, I use deep sleep mode with timer mode wake-up
//All this is powered from a solar panel and a 12v battery

//#include "esp32-hal-bt-mem.h"
#include <Adafruit_SHT31.h>
#include <Arduino.h>
#include <esp_sleep.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

#define i2c_Address 0x3c //initialize with the I2C addr 0x3C Typically eBay OLED's
                         // e.g. the one with GM12864-77 written on it
//#define i2c_Address 0x3d //initialize with the I2C addr 0x3D Typically Adafruit OLED's

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET -1   //   QT-PY / XIAO
Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
Adafruit_SHT31 sht31 = Adafruit_SHT31();

int Open = 20; //Output to activate relay for opening
int Close = 21; //Output to activate relay for closing
int Moist_sens = 0;  //Analog input pin for the soil moisture sensor 
int Moist_pot =1; //Pin connected to potentiometer to set minimum moisture
int Temp_pot = 2; //Pin connected to potentiometer to set target temp
int Watervalve = 10; //Pin that opens watervalve
int dryLED = 7; //Red LED indicating dry soil

void setup() {
  Serial.begin(9600);                                                                                                                       
  delay(100); 
  
  // initialize the OLED object
  display.begin(i2c_Address, true); // Address 0x3C default

  display.display();
  delay(1000);
  display.clearDisplay();  // Clear the display buffer.

  pinMode (Moist_pot, INPUT);
  pinMode(Moist_sens, INPUT);
  pinMode(Open, OUTPUT);
  pinMode(Close, OUTPUT);
  pinMode(Watervalve, OUTPUT);
  pinMode(dryLED, OUTPUT);

  digitalWrite(Open, HIGH);
  digitalWrite(Close, HIGH);
  digitalWrite (Watervalve, HIGH);

  Serial.println("SHT31 test");
  if (!sht31.begin(0x44)) { // Set to 0x45 for alternate i2c addr
    Serial.println("Couldn't find SHT31");
    while (1) delay(1);
  }

  #define uS_TO_S_FACTOR 1000000ULL /* Conversion factor for micro seconds to seconds */
  #define TIME_TO_SLEEP  40          /* Time ESP32 will go to sleep (in seconds) */
   // Set the timer to wake up after some time
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR); 
  
 }

void loop() {
  float t = sht31.readTemperature();
  float h = sht31.readHumidity();

    // getting moisture value
  //float Moisture = readSoilMoisture(Moist_sens); 
    // read the value from the Moistsensor: 
  int MoistsensorValue = analogRead(Moist_sens);
  // coverting value in Scale of 0 to 100
  int Act_moist = map(MoistsensorValue,1231,2987,100,0);

  // Read moisture target potentiometer and map it to minimum soil moisture
  int MoistpotValue = analogRead(Moist_pot);
  int Minmoist = map(MoistpotValue, 0, 4095, 0, 100);

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  display.print("Reading.....");
  display.display();
  delay(2000);
  display.clearDisplay();

  Serial.print("Moisture : "); Serial.print(Act_moist); Serial.println("%");
  Serial.print("Moist sensor reading"); Serial.println(MoistsensorValue); 
  Serial.print ("Min Moisture"); Serial.println (Minmoist);
  
  // Read temp target potentiometer value and map it to temperature range
  int TemppotValue = analogRead(Temp_pot);
  int Maxtemp = map(TemppotValue, 0, 4095, 10, 30);
  int Mintemp = Maxtemp - 4;

  
  
  if (!isnan(t)) { // check if 'is not a number'
    
    Serial.print("Set Temp °C = ");Serial.println (Maxtemp);
    Serial.print("Temp °C = "); Serial.print(t); Serial.println("\t\t");

   display.setCursor(15, 0);
   display.print("Targ temp= ");
   display.print(Maxtemp);
   display.println(" C");
  } 
  else {
    Serial.println("Failed to read temperature");
    display.println("Temp failure.");
  }
  if (!isnan(h)) { // check if 'is not a number'
    Serial.print("Hum. % = "); Serial.println(h);
    display.setCursor(5, 12);
    display.print("Humidity "); display.print(h); display.println (" %");
    display.setCursor(0, 25);
    display.print("Soil moist."); display.print(Act_moist); display.print(" % /");display.println(Minmoist);
    display.setCursor(25, 40);
    display.println("Actual temp. ");
    display.setCursor(25, 50);display.setTextSize(2); display.print(t); display.print(" C");
    display.display();

  } 
  else {
    Serial.println("Failed to read humidity" );
    //u8g2.drawStr(0,25,"Hum failure.");
    display.println("hum failure");
    display.display();

  }
   //u8g2.sendBuffer();

  if (Act_moist < Minmoist) { 
  digitalWrite (Watervalve, LOW);
  digitalWrite (dryLED, HIGH);

  delay(4000);
  digitalWrite (Watervalve, HIGH);
  }
  else {
  digitalWrite(dryLED, LOW);
  }
  if (t > Maxtemp) {
    digitalWrite(Open, LOW);
    delay(200);
    digitalWrite(Open, HIGH);
  }
  if (t < Mintemp) {
    digitalWrite(Close, LOW);
    delay(200);
    digitalWrite(Close, HIGH);
  }
  delay(10000);
    
  esp_deep_sleep_start();
}

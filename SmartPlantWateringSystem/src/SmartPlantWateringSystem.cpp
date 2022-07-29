/******************************************************/
//       THIS IS A GENERATED FILE - DO NOT EDIT       //
/******************************************************/

#include "Particle.h"
#line 1 "/Users/Layla2/Documents/IoT/SmartPlantWateringSystem-/SmartPlantWateringSystem/src/SmartPlantWateringSystem.ino"
/*
 * Project SmartPlantWateringSystem
 * Description: Dust Sensor/BME/ display to screen/ publish moisture?
 * Author: Micalah Miller
 * Date: 07/29/2022
 */
#include "Adafruit_SSD1306.h"
#include "Adafruit_BME280.h"

void setup();
void loop();
void takeAndDisplayReadings();
void drawText();
#line 10 "/Users/Layla2/Documents/IoT/SmartPlantWateringSystem-/SmartPlantWateringSystem/src/SmartPlantWateringSystem.ino"
const int BMEADDRESS = 0x76;
const int SCREENADDRESS = 0x3C;   //has an address not assigned
const int OLEDRESET = D4;
byte i, count;
int currentTime, lastTime;

Adafruit_BME280 bme;   //declare temperature sensor 
Adafruit_SSD1306 display(OLEDRESET);   //declare OLED


void setup() {

Time.zone(-6);
Particle.syncTime();   //for MDT

pinMode(A3, INPUT);  //Pin for mositure reading

display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  //apparently 0x3C is for the screen width and height, must go into library to alter this
display.display();  
display.clearDisplay();

bme.begin(0x76);  //initiazlie temp, pressure, and humidity sensor
}

void loop() {

  
  currentTime =  millis();
  if (currentTime - lastTime > 10000){
    display.clearDisplay();
    drawText();
    takeAndDisplayReadings();
    lastTime = millis();
  }
}

void takeAndDisplayReadings(){     // take and convert temperature and pressure readings and display them on the screen
  float tempC;
  float pressPA;
  float humidRH;
  float roomTempF;
  float pressureHG;
  int moisture;  //plant moisture
  String DateTime = Time.timeStr();
  String TimeOnly = DateTime.substring(11,19);  //only want to display the time

  Serial.printf("before readings\n");
 tempC= bme.readTemperature();
 pressPA = bme.readPressure();
 humidRH = bme.readHumidity();
 Serial.printf("humidity: %0.2f \n", humidRH);
 roomTempF = (tempC*1.8)+32;  // convert to Celcius to Farenheit degrees
 pressureHG =  (pressPA)*(1/3386.39); //convert from Pascals to units of mercury
 moisture = analogRead(A3);
 Serial.printf("moisture: %i", moisture);
 //display.clearDisplay();
 //Serial.printf("cleared");
 display.printf("Temp: %0.2f \nPressure: %0.2f \nHumidity: %0.2f \n" ,roomTempF, pressureHG,humidRH);
 display.printf("Moist:%i \nTime: %s" , moisture, TimeOnly.c_str());
 display.display();
}
void drawText(){
  Serial.printf("text should be displaying");
  display.setTextSize(1);  //change depending on what you're displaying
  display.setTextColor(WHITE);  //no other option
  display.setCursor(0,0);
}
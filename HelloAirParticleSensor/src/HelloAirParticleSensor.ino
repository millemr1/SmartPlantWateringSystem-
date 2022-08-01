/*
 * Project HelloAirParticleSensor
 * Description: Get readings from air particle sensor 
 * Author: Micalah Miller
 * Date: 7/30/2022
 */
 #include "Grove_Air_Quality_Sensor.h"


//Grove_Air_quality_Sensor grove_Air_quality_Sensor;
AirQualitySensor airQualitySensor(A5);  
// I think this is right  but might also just be Airquality sensor as the class
 
int currentQuality = -1;

void setup() {
  Serial.begin(9600);
  //airQualitySensor.init(); 

 Serial.printf("Waiting sensor to init...");
  delay(20000);
  
  if (airQualitySensor.init()) {
    Serial.printf("Sensor ready.");
  }
  else {
    Serial.printf("Sensor ERROR!");
  }
}

void loop() {
   currentQuality = airQualitySensor.slope();  //get readning from the sensor

  if(currentQuality >= 0){  //something valid
    Serial.printf("Air Quality READING: %i \n" , currentQuality);
  }
}
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
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT/Adafruit_MQTT.h" 
#include "Adafruit_MQTT/Adafruit_MQTT_SPARK.h"
#include "Adafruit_SSD1306.h"
#include "Adafruit_BME280.h"
#include "credentials.h"

void setup();
void loop();
void takeAndDisplayReadings();
void drawText();
void turnPumpOn();
bool IsButtonOnDashPressed();
void MQTT_connect();
#line 14 "/Users/Layla2/Documents/IoT/SmartPlantWateringSystem-/SmartPlantWateringSystem/src/SmartPlantWateringSystem.ino"
TCPClient TheClient; 

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details. 
Adafruit_MQTT_SPARK mqtt(&TheClient,AIO_SERVER,AIO_SERVERPORT,AIO_USERNAME,AIO_KEY); 

Adafruit_MQTT_Subscribe mqttObjWaterManually = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/ButtontoPump");  

SYSTEM_MODE(SEMI_AUTOMATIC);


const int BMEADDRESS = 0x76;
const int SCREENADDRESS = 0x3C;   //has an address not assigned
const int OLEDRESET = D4;
const int RELAYPIN = 11;
byte i, count;
int currentTime, lastTime;

Adafruit_BME280 bme;   //declare temperature sensor 
Adafruit_SSD1306 display(OLEDRESET);   //declare OLED


void setup() {

Serial.begin(9600);
  waitFor(Serial.isConnected, 15000); 

 //Connect to WiFi without going to Particle Cloud
  WiFi.connect();
  while(WiFi.connecting()) {
    Serial.printf(".");
  }
mqtt.subscribe(&mqttObjWaterManually);

Time.zone(-6);
Particle.syncTime();   //for MDT

pinMode(A3, INPUT);  //Pin for mositure reading
pinMode(RELAYPIN, OUTPUT);

display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  //apparently 0x3C is for the screen width and height, must go into library to alter this
display.display();  
display.clearDisplay();

bme.begin(0x76);  //initiazlie temp, pressure, and humidity sensor
}

void loop() {
  IsButtonOnDashPressed();
  currentTime =  millis();
  if (currentTime - lastTime > 10000){
    display.clearDisplay();
    drawText();
   // turnPumpOn();
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
void turnPumpOn(){  // turns pump on for a few seconds
  //int currentTime = millis();
  //int lastTime;
  digitalWrite(RELAYPIN, HIGH);   // make this more functional later
  delay(250);
  digitalWrite(RELAYPIN, LOW);
  //if (currentTime - lastTime > 250){   //built in timer to not overwater my plant
    //lastTime = millis();
  

  //lastTime = millis()
}
 
bool IsButtonOnDashPressed(){
  float buttonState;
  bool isButtonState;
   Adafruit_MQTT_Subscribe * subscription;
  while(subscription = mqtt.readSubscription(1000)){
    if(subscription == &mqttObjWaterManually){
      buttonState = atof((char *)mqttObjWaterManually.lastread);
      Serial.printf("Received %0.2f from Adafruit.io feed /ButtontoPump \n", buttonState);     
    }
  }
    if(buttonState == 1.00){
      isButtonState = true;
      Serial.printf("button is on \n");
      delay(500);
    }
    else if (buttonState == 0.00){
      isButtonState = false;
       Serial.printf("button is off \n");
       delay(500);
     }
    return isButtonState;
}

// Function to connect and reconnect as necessary to the MQTT server.
void MQTT_connect() {
  int8_t ret;
 
  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }
 
  Serial.print("Connecting to MQTT... ");
 
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       Serial.printf("%s\n",(char *)mqtt.connectErrorString(ret));
       Serial.printf("Retrying MQTT connection in 5 seconds..\n");
       mqtt.disconnect();
       delay(5000);  // wait 5 seconds
  }
  Serial.printf("MQTT Connected!\n");
}


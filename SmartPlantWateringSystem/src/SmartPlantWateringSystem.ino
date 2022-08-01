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
#include "math.h"
#include "credentials.h"

TCPClient TheClient; 

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details. 
Adafruit_MQTT_SPARK mqtt(&TheClient,AIO_SERVER,AIO_SERVERPORT,AIO_USERNAME,AIO_KEY); 

Adafruit_MQTT_Subscribe mqttObjWaterManually = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/buttontopump");  
Adafruit_MQTT_Publish mqttObjTempData = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/tempdataplant");
Adafruit_MQTT_Publish mqttObjPressData = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/plantpressure");
Adafruit_MQTT_Publish mqttObjHumidity = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/HumidityPlant");
Adafruit_MQTT_Publish mqttObjMoisture = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/moisture-plant");
Adafruit_MQTT_Publish mqttObjDust = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/moisture-plant");




SYSTEM_MODE(SEMI_AUTOMATIC);


const int BMEADDRESS = 0x76;
const int SCREENADDRESS = 0x3C;   //has an address not assigned
const int OLEDRESET = D4;
const int RELAYPIN = 11;
int DUSTPIN = D5;

int currentTime, lastTime;
byte i, count;
bool pressed;

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
pinMode(DUSTPIN, INPUT);


display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  //apparently 0x3C is for the screen width and height, must go into library to alter this
display.display();  
display.clearDisplay();

bme.begin(0x76);  //initiazlie temp, pressure, and humidity sensor
}

void loop() {
  MQTT_connect();
  pressed = IsButtonOnDashPressed(); //check if button is pressed from the dashboard
  if(pressed){
    Serial.print("Pump is on");
    turnPumpOn();
  }
  currentTime =  millis();
  if (currentTime - lastTime > 10000){
    display.clearDisplay();
    drawText();
   // turnPumpOn();
    takeDisplayPublishEVData();
    checkMoistureandWater();
    takeandPublishDust();
    lastTime = millis(); 
  }
}
void takeDisplayPublishEVData(){
  float roomTempF;;
  float pressureHG;
  float tempC;
  float pressPA;
  float humidRH;
  int analogMoisture;  //plant moisture
  int moisturePercent;
  String DateTime = Time.timeStr();
  String TimeOnly = DateTime.substring(11,19);  //only want to display the time

  Serial.printf("before readings\n");
 tempC= bme.readTemperature();
 pressPA = bme.readPressure();
 Serial.printf("humidity: %0.2f \n", humidRH);
 humidRH = bme.readHumidity();
 roomTempF = (tempC*1.8)+32;  // convert to Celcius to Farenheit degrees
 pressureHG =  (pressPA)*(1/3386.39) + 5; //convert from Pascals to units of mercury adding 5 to compensate for ABQ altitude
 
  analogMoisture= analogRead(A3);
  moisturePercent = map(analogMoisture, 2700, 1500, 1, 100);
 
 Serial.printf(" Analog moisture: %i \n Percent: %i \n", analogMoisture, moisturePercent );
 //display.clearDisplay();
 //Serial.printf("cleared");
 display.printf("Temp: %0.2f \nPressure: %0.2f \nHumidity: %0.2f \n" ,roomTempF, pressureHG,humidRH);
 display.printf("Moist: %i \nTime: %s" , moisturePercent, TimeOnly.c_str());
 display.display();
  
  if(mqtt.Update()) {
      mqttObjTempData.publish(roomTempF);
      mqttObjPressData.publish(pressureHG);
      mqttObjHumidity.publish(humidRH);
      mqttObjMoisture.publish(moisturePercent);
      Serial.printf("Publishing Temp: %0.2f \nPressure: %0.2f \nHumidity: %0.2f \n", roomTempF, pressureHG,humidRH); 
      Serial.printf(" Publishing: Analog moisture: %i \n Percent: %i \n", analogMoisture, moisturePercent);
  }
}
void drawText(){
  Serial.printf("text should be displaying");
  display.setTextSize(1);  //change depending on what you're displaying
  display.setTextColor(WHITE);  //no other option
  display.setCursor(0,0);
}
void takeandPublishDust(){
static unsigned long startTime;  //because it kept giving me warnings about comparisons 
static unsigned long duration;
unsigned long sampleTime = 30000;  //
static int lowPulseOccupancy = 0;  // gotten from example code I am wondering why we start it at 0
static float ratio;
static float concentration;

  duration = pulseIn(DUSTPIN, LOW); //waits for pin to go from high to low start timeing
  Serial.printf("Duration: %i \n", duration);
  lowPulseOccupancy = lowPulseOccupancy + duration; // duration from low to high + duration
  
  if((millis() - startTime) > sampleTime ){
    ratio = lowPulseOccupancy/(sampleTime * 10.0); // gets the ratio of dust in air over 30 seconds and converts it into a percentage?
    concentration = 1.1*pow(ratio, 3)-3.8*pow(ratio, 2)+(520*ratio)+0.62;
    Serial.printf("LPO : %i \n ratio: %f \n concentration: %f \n", lowPulseOccupancy, ratio, concentration);
    lowPulseOccupancy = 0;
    startTime = millis ();
  }
  if(mqtt.Update()) {
      mqttObjDust.publish(concentration);
      Serial.printf("Publishing Concentration: %0.2f", concentration);
  }
}
void turnPumpOn(){  // turns pump on for a few seconds
  digitalWrite(RELAYPIN, HIGH);   // make this more functional later
  delay(250);
  digitalWrite(RELAYPIN, LOW);
}
 
bool IsButtonOnDashPressed(){
  float buttonState;
  bool isButtonState;
   Adafruit_MQTT_Subscribe * subscription;
  while(subscription = mqtt.readSubscription(200)){
    if(subscription == &mqttObjWaterManually){
      buttonState = atof((char *)mqttObjWaterManually.lastread);
      Serial.printf("Received %0.2f from Adafruit.io feed /ButtontoPump \n", buttonState);     
    }
  }
    if(buttonState == 1.00){
      isButtonState = true;
      Serial.printf("button is on \n");
    }
    else if (buttonState == 0.00){
      isButtonState = false;
       Serial.printf("button is off \n");
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
void checkMoistureandWater(){
  int analogMoisture = analogRead(A3);  //plant moisture A3 is the Pin
  int moisturePercent;
  moisturePercent = map(analogMoisture, 2700, 1500, 1, 100);
  if (moisturePercent < 15){
    turnPumpOn();
    Serial.printf("Watering Plant");
  }
}




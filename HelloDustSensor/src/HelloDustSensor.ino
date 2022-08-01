/*
 * Project HelloDustSensor
 * Description:
 * Author:
 * Date:
 */

 #include "math.h"
int DUSTPIN = D5;
unsigned long startTime;  //because it kept giving me warnings about comparisons 
unsigned long duration;
unsigned long sampleTime = 30000;  //
int lowPulseOccupancy = 0;  // gotten from example code I am wondering why we start it at 0
float ratio;
float concentration;

void setup() {

  Serial.begin(9600);
  pinMode(DUSTPIN, INPUT);
  startTime =  millis();

}
void loop() { 
  duration = pulseIn(DUSTPIN, LOW); //waits for pin to go from high to low start timeing
  lowPulseOccupancy = lowPulseOccupancy + duration; // duration from low to high + duration
  
  if((millis() - startTime) > sampleTime ){
    ratio = lowPulseOccupancy/(sampleTime * 10.0); // gets the ratio of dust in air over 30 seconds and converts it into a percentage?
    concentration = 1.1*pow(ratio, 3)-3.8*pow(ratio, 2)+520*ratio+0.62;
    Serial.printf("LPO : %i \n ratio: %f \n concentration: %f \n", lowPulseOccupancy, ratio, concentration);
    lowPulseOccupancy = 0;
    startTime = millis ();
  }
}
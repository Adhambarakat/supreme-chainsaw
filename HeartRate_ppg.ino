#include <heartRate.h>
#include <MAX30105.h>
#include <spo2_algorithm.h>
#include <Wire.h>
 MAX30105 sensor;
 int beats[10];
 float beat = 0;
 int avgbpm = 0;
 int i = 0;
 int arrayindex =0; 
 int x =0;




void setup() {
  Serial.begin(115200);
  sensor.begin(Wire , I2C_SPEED_FAST);
  sensor.setup();
  
 

  
  
}

void loop() {
   int IR = sensor.getIR();
  delay(10);
  if(checkForBeat(IR) == true){
    Serial.println("beat detected");
    float delta = millis()- beat;
    beat = millis();
    int bpm = 60/(delta/1000);
    beats[arrayindex++] = bpm; //adding values to the array
    for(i=0; i<10; i=i+1)
      avgbpm+=beats[i];
      avgbpm/=i+1;

    Serial.print("BPM ="); 
    Serial.println(bpm);
    Serial.print("avgbpm =");
    Serial.println(avgbpm,10);  
    Serial.print("G=");
    Serial.println(sensor.getGreen());
    if(i == 9){
    Serial.println('yes');
    }
      
     
    }

   
 
    
    
  }

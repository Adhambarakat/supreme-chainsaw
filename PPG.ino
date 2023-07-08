#include <Wire.h>
#include "MAX30105.h"

MAX30105 particleSensor;

long startTime;
long samplesTaken = 0; //Counter for calculating the Hz or read rate

byte interruptPin = 3; //Connect INT pin on breakout board to pin 3

int G[100];

void setup()
{
  pinMode(interruptPin, INPUT);

  Serial.begin(115200);
  Serial.println("Initializing...");

  // Initialize sensor
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) //Use default I2C port, 400kHz speed
  {
    Serial.println("MAX30105 was not found. Please check wiring/power. ");
    while (1);
  }

  //Let's configure the sensor to run fast so we can over-run the buffer and cause an interrupt
  byte ledBrightness = 0x7F; //Options: 0=Off to 255=50mA
  byte sampleAverage = 1; //Options: 1, 2, 4, 8, 16, 32
  byte ledMode = 3; //Options: 1 = Red only, 2 = Red + IR, 3 = Red + IR + Green
  byte sampleRate = 400; //Options: 50, 100, 200, 400, 800, 1000, 1600, 3200
  int pulseWidth = 69; //Options: 69, 118, 215, 411
  int adcRange = 4096; //Options: 2048, 4096, 8192, 16384

  particleSensor.setup(ledBrightness, sampleAverage, ledMode, sampleRate, pulseWidth, adcRange); //Configure sensor with these settings

  particleSensor.enableAFULL(); //Enable the almost full interrupt (default is 32 samples)
  
  particleSensor.setFIFOAlmostFull(3); //Set almost full int to fire at 29 samples

  startTime = millis();
}

void loop()
{
  particleSensor.check(); //Check the sensor, read up to 3 samples

  while (particleSensor.available()) //do we have new data?
  {
    samplesTaken++;



    Serial.print("] G[");
    Serial.print(particleSensor.getGreen());

   
 

    Serial.println();

    particleSensor.nextSample(); //We're finished with this sample so move to next sample
  }
}

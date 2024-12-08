#include <Arduino.h>
#include <Arduino_LPS22HB.h>
#include <SD.h>

#include "Arduino_BMI270_BMM150.h"
#include "sampler.h"

namespace
{
  // Time interval for data collection
  unsigned long previousMillis = 0;            // Store the last time the data collection event occurred
  const unsigned long intervalInMillis = 1000; // Interval at which to trigger the data collection event (milliseconds)

  bool printResults = true;

  Sampler *sampler;
} // namespace

void setup()
{
  // Start serial
  Serial.begin(9600);
  while (!Serial)
    ;
  Serial.println("Serial started");

  // Start IMU
  if (!IMU.begin())
  {
    Serial.println("Failed to initialized IMU!");
    while (1)
      ;
  }
  IMU.setContinuousMode();
  float accSampleRate = IMU.accelerationSampleRate();
  Serial.println("IMU acc sampling rate: ");
  Serial.println(accSampleRate);

  // Start pressure sensor
  if (!BARO.begin())
  {
    Serial.println("Failed to initialize pressure sensor!");
    while (1)
      ;
  }

  // Start sd card
  if (!SD.begin(A0))
  {
    Serial.println("Failed to initialize SD card!");
    while (1)
      ;
  }

  sampler = new Sampler(256, accSampleRate, 10);
}

void loop()
{
  unsigned long currentMillis = millis();
  bool isTimeForDataCollection = previousMillis == 0 || currentMillis - previousMillis >= intervalInMillis;

  if (isTimeForDataCollection)
  {
    if (printResults)
    {
      Serial.println("Collecting acc data...");
    }

    previousMillis = currentMillis;
    sampler->sampleData(printResults);

    if (printResults)
    {
      Serial.println("Acc data collected");
    }
  }
}

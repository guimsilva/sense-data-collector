#include <Arduino.h>
#include <Arduino_LPS22HB.h>
#include <SPI.h>
#include <SD.h>

#include "Arduino_BMI270_BMM150.h"
#include "sampler.h"

namespace
{
  // Time interval for data collection
  unsigned long previousMillis = 0;    // Store the last time the data collection event occurred
  const unsigned long interval = 2000; // Interval at which to trigger the data collection event (milliseconds)

  // Pressure sensor
  float current_pressure = 0.0f;
  float new_pressure = 0.0f;
  float altitude = 0.0f;

  bool isLogging = false;

  Sampler *sample = nullptr;
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

  sample = new Sampler(512, 3);
}

void loop()
{
  unsigned long currentMillis = millis();
  bool isTimeForDataCollection = currentMillis - previousMillis >= interval;

  if (isTimeForDataCollection)
  {
    if (isLogging)
    {
      Serial.println("Collecting vibration data...");
    }

    previousMillis = currentMillis;
    sample->sampleData(isLogging);

    if (isLogging)
    {
      Serial.println("Vibration data collected");
    }
  }

  // while (1)
  //   ;
}

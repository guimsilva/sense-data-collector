#include <Arduino.h>
#include <Arduino_LPS22HB.h>

#include "Arduino_BMI270_BMM150.h"
#include "..\include\vibration.h"

namespace
{
  unsigned long previousMillis = 0;    // Store the last time the data collection event occurred
  const unsigned long interval = 2000; // Interval at which to trigger the data collection event (milliseconds)

  // Pressure sensor
  float current_pressure = 0.0f;
  float new_pressure = 0.0f;
  float altitude = 0.0f;

  // Vibration instance
  Vibration *vibration = nullptr;

  bool isLogging = true;
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

  vibration = new Vibration(512, 512, 2);
}

void loop()
{
  unsigned long currentMillis = millis();
  bool isTimeForDataCollection = currentMillis - previousMillis >= interval;

  // getAltitude();

  if (isTimeForDataCollection)
  {
    if (isLogging)
    {
      Serial.println("Collecting vibration data...");
    }

    previousMillis = currentMillis;
    vibration->sampleVibration();
    vibration->computeVibrationFFT(isLogging);

    if (isLogging)
    {
      Serial.println("Vibration data collected");
    }
  }

  // while (1)
  //   ;
}

void getAltitude()
{
  new_pressure = BARO.readPressure();
  if (new_pressure != current_pressure)
  {
    current_pressure = new_pressure;
    altitude = 44330 * (1 - pow(current_pressure / 101.325, 1 / 5.255));
  }

  Serial.print("Altitude according to kPa is = ");
  Serial.print(altitude);
  Serial.println(" m");
  Serial.println();
}

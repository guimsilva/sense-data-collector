#include <Arduino.h>
#include <Arduino_LPS22HB.h>
#include <SD.h>

#include "vibration_utils.h"

namespace
{
  // Pressure sensor
  float current_pressure = 0.0f;
  float new_pressure = 0.0f;
  float altitude = 0.0f;
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

  // // Start SD card
  // if (!SD.begin(A0))
  // {
  //   Serial.println("Failed to initialize SD card!");
  //   while (1)
  //     ;
  // }
}

void loop()
{
  // getAltitude();

  sampleVibration();
  computeVibrationFFT();

  while (1)
    ;

  // delay(2000);
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

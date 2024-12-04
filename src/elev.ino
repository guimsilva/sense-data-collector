#include <Arduino.h>
#include <Arduino_LPS22HB.h>
#include <SPI.h>
#include <SD.h>
#include <ArduinoBLE.h>

#include "vibration_utils.h"
#include "ble_utils.h"

namespace
{
  unsigned long previousMillis = 0; // Store the last time the data collection event occurred
  const long interval = 2000;       // Interval at which to trigger the data collection event (milliseconds)

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

  // Start BLE
  if (!BLE.begin())
  {
    Serial.println("Failed to initialized BLE!");
    while (1)
      ;
  }
  bleSetup();
}

void loop()
{
  unsigned long currentMillis = millis();
  bool isTimeForDataCollection = currentMillis - previousMillis >= interval;

  // getAltitude();

  if (isTimeForDataCollection)
  {
    previousMillis = currentMillis;
    sampleVibration();
    computeVibrationFFT(false, LOG_VIA_BLUETOOTH);
  }

  bleComms();

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

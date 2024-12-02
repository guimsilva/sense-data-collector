/* Copyright 2020 The TensorFlow Authors. All Rights Reserved.
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at
    http://www.apache.org/licenses/LICENSE-2.0
Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/
#include <Arduino.h>
#include <Arduino_LPS22HB.h>

#include "imu_provider.h"
#include "arduinoFFT.h"

// x = 512 samples and sampling frequency y = 512 will result in 1 second of sampling (x / y = sec)
#define SAMPLES 512            // Must be a power of 2
#define SAMPLING_FREQUENCY 512 // Hz. Determines maximum frequency that can be analysed by the FFT.

// Used for printing FFT results
#define SCL_INDEX 0x00
#define SCL_TIME 0x01
#define SCL_FREQUENCY 0x02
#define SCL_PLOT 0x03

namespace
{
  // Pressure sensor
  float current_pressure = 0.0f;
  float new_pressure = 0.0f;
  float altitude = 0.0f;

  // IMU sensor
  float acc_x, acc_y, acc_z;

  // FFT
  unsigned int sampling_period_us;
  unsigned long microseconds;

  /*
  These are the input and output vectors
  Input vectors receive computed results from FFT
  */
  double vReal[SAMPLES];
  double vImag[SAMPLES];

  /* Create FFT object */
  ArduinoFFT<double> FFT = ArduinoFFT<double>(vReal, vImag, SAMPLES, SAMPLING_FREQUENCY);
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
  SetupIMU();

  // Start pressure sensor
  if (!BARO.begin())
  {
    Serial.println("Failed to initialize pressure sensor!");
    while (1)
      ;
  }
}

void loop()
{
  // getAltitude();
  // Serial.print("Altitude according to kPa is = ");
  // Serial.print(altitude);
  // Serial.println(" m");
  // Serial.println();

  /* Sampling */
  for (int i = 0; i < SAMPLES; i++)
  {
    microseconds = micros();
    if (IMU.accelerationAvailable())
    {
      IMU.readAcceleration(acc_x, acc_y, acc_z);
    }
    vReal[i] = sqrt(acc_x * acc_x + acc_y * acc_y + acc_z * acc_z);
    vImag[i] = 0;

    while (micros() < (microseconds + sampling_period_us))
      ; // wait for next sample
  }

  /* Print the results of the simulated sampling according to time */
  Serial.println("Data:");
  PrintVector(vReal, SAMPLES, SCL_TIME);
  FFT.windowing(FFTWindow::Hamming, FFTDirection::Forward); /* Weigh data */
  Serial.println("Weighed data:");
  PrintVector(vReal, SAMPLES, SCL_TIME);
  FFT.compute(FFTDirection::Forward); /* Compute FFT */
  Serial.println("Computed Real values:");
  PrintVector(vReal, SAMPLES, SCL_INDEX);
  Serial.println("Computed Imaginary values:");
  PrintVector(vImag, SAMPLES, SCL_INDEX);
  FFT.complexToMagnitude(); /* Compute magnitudes */
  Serial.println("Computed magnitudes:");
  PrintVector(vReal, (SAMPLES >> 1), SCL_FREQUENCY);
  double x = FFT.majorPeak();
  Serial.println(x, 6);

  while (1)
    ; /* Run Once */
  // delay(2000); /* Repeat after delay */
}

void getAltitude()
{
  new_pressure = BARO.readPressure();
  if (new_pressure != current_pressure)
  {
    current_pressure = new_pressure;
    altitude = 44330 * (1 - pow(current_pressure / 101.325, 1 / 5.255));
  }
}

void PrintVector(double *vData, uint16_t bufferSize, uint8_t scaleType)
{
  for (uint16_t i = 0; i < bufferSize; i++)
  {
    double abscissa;
    /* Print abscissa value */
    switch (scaleType)
    {
    case SCL_INDEX:
      abscissa = (i * 1.0);
      break;
    case SCL_TIME:
      abscissa = ((i * 1.0) / SAMPLING_FREQUENCY);
      break;
    case SCL_FREQUENCY:
      abscissa = ((i * 1.0 * SAMPLING_FREQUENCY) / SAMPLES);
      break;
    }
    Serial.print(abscissa, 6);
    if (scaleType == SCL_FREQUENCY)
      Serial.print("Hz");
    Serial.print(" ");
    Serial.println(vData[i], 4);
  }
  Serial.println();
}

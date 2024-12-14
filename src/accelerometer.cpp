#include <Arduino.h>

#include "accelerometer.h"
#include "Arduino_BMI270_BMM150.h"

Accelerometer::Accelerometer(SamplerConfig *_samplerConfig)
    : samplerConfig(_samplerConfig),
      vRealX(new double[_samplerConfig->accOptions->accNumSamples]),
      vRealY(new double[_samplerConfig->accOptions->accNumSamples]),
      vRealZ(new double[_samplerConfig->accOptions->accNumSamples])
{
    if (samplerConfig->samplerOptions->logLevel >= LogLevel::Info)
    {
        Serial.println("Initializing accelerometer");
    }

    accX = 0.0;
    accY = 0.0;
    accZ = 0.0;

    // Start IMU
    if (!IMU.begin())
    {
        Serial.println("Failed to initialize IMU!");
        while (1)
            ;
    }
    IMU.setContinuousMode();

    if (samplerConfig->accOptions->accSamplingFrequency == 0)
    {
        samplerConfig->accOptions->accSamplingFrequency = IMU.accelerationSampleRate();
    }
    if (samplerConfig->accOptions->accSamplingFrequency == 0)
    {
        Serial.println("accSamplingFrequency is 0");
        while (1)
            ;
    }
    samplerConfig->accOptions->accSamplingLengthMs = round(static_cast<double>(samplerConfig->accOptions->accNumSamples) / samplerConfig->accOptions->accSamplingFrequency * 1000);
    if (samplerConfig->accOptions->accSamplingLengthMs == 0)
    {
        Serial.println("accSamplingLengthMs is 0");
        while (1)
            ;
    }

    samplingPeriodUs = round(1000000 * (1.0 / samplerConfig->accOptions->accSamplingFrequency));

    if (_samplerConfig->samplerOptions->logLevel >= LogLevel::Info)
    {
        Serial.println("Accelerometer initialized\n");
    }
}

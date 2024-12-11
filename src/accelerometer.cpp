#include <Arduino.h>

#include "options.h"
#include "accelerometer.h"
#include "Arduino_BMI270_BMM150.h"

Accelerometer::Accelerometer(SamplerOptions *_samplerOptions)
    : samplerOptions(_samplerOptions),
      vRealX(new double[_samplerOptions->accNumSamples]),
      vRealY(new double[_samplerOptions->accNumSamples]),
      vRealZ(new double[_samplerOptions->accNumSamples])
{
    if (samplerOptions->logLevel >= LogLevel::Info)
    {
        Serial.println("Initializing accelerometer");
    }

    accX = 0.0;
    accY = 0.0;
    accZ = 0.0;

    // Start IMU
    if (!IMU.begin())
    {
        Serial.println("Failed to initialized IMU!");
        while (1)
            ;
    }
    IMU.setContinuousMode();

    if (samplerOptions->accSamplingFrequency == 0)
    {
        samplerOptions->accSamplingFrequency = IMU.accelerationSampleRate();
    }
    if (samplerOptions->accSamplingFrequency == 0)
    {
        Serial.println("accSamplingFrequency is 0");
        while (1)
            ;
    }
    samplerOptions->accSamplingLengthMs = round(static_cast<double>(samplerOptions->accNumSamples) / samplerOptions->accSamplingFrequency * 1000);
    if (samplerOptions->accSamplingLengthMs == 0)
    {
        Serial.println("accSamplingLengthMs is 0");
        while (1)
            ;
    }

    samplingPeriodUs = round(1000000 * (1.0 / samplerOptions->accSamplingFrequency));

    if (samplerOptions->logLevel >= LogLevel::Info)
    {
        Serial.println("Accelerometer initialized with options (AccNumSamples, AccSamplingFrequency, AccSamplingLengthMs):");
        Serial.println(samplerOptions->accNumSamples);
        Serial.println(samplerOptions->accSamplingFrequency);
        Serial.println(samplerOptions->accSamplingLengthMs);
        Serial.println();
    }
}

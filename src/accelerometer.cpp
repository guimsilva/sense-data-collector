#include <Arduino.h>

#include "options.h"
#include "accelerometer.h"
#include "Arduino_BMI270_BMM150.h"

Accelerometer::Accelerometer(SampleDataPoint *_sample, SamplerOptions *_options)
    : options(_options),
      sample(_sample),
      vRealX(new double[_options->accNumSamples]),
      vRealY(new double[_options->accNumSamples]),
      vRealZ(new double[_options->accNumSamples])
{
    if (options->logLevel >= LogLevel::Info)
    {
        Serial.println("Initializing accelerometer");
    }

    // Start IMU
    if (!IMU.begin())
    {
        Serial.println("Failed to initialized IMU!");
        while (1)
            ;
    }
    IMU.setContinuousMode();

    if (options->accSamplingFrequency == 0)
    {
        _options->accSamplingFrequency = IMU.accelerationSampleRate();
    }

    samplingPeriodUs = round(1000000 * (1.0 / options->accSamplingFrequency));

    if (options->logLevel >= LogLevel::Info)
    {
        float accSampleRate = IMU.accelerationSampleRate();
        Serial.println("Accelerometer initialized with options (AccNumSamples, AccSamplingFrequency):");
        Serial.println(options->accNumSamples);
        Serial.println(options->accSamplingFrequency);
        Serial.println();
    }
}

void Accelerometer::sampleAcceleration()
{
    if (options->logLevel >= LogLevel::Info)
    {
        Serial.println("Sampling acc data...");
    }
    for (int i = 0; i < options->accNumSamples; i++)
    {
        microseconds = micros();
        if (IMU.accelerationAvailable())
        {
            IMU.readAcceleration(accX, accY, accZ);
            if (options->logLevel >= LogLevel::Verbose)
                Serial.println("Read acceleration data! >>> ");
        }
        else
        {
            if (options->logLevel >= LogLevel::Verbose)
                Serial.println("Failed to read acceleration data! <<< ");

            accX = 0.0;
            accY = 0.0;
            accZ = 0.0;
        }

        if (options->logLevel >= LogLevel::Verbose)
        {
            Serial.print("Acceleration read: ");
            Serial.print(accX);
            Serial.print(" ");
            Serial.print(accY);
            Serial.print(" ");
            Serial.println(accZ);
        }

        vRealX[i] = accX;
        vRealY[i] = accY;
        vRealZ[i] = accZ;

        while (micros() < (microseconds + samplingPeriodUs))
            ; // wait for next sample
    }

    // Add the frequencies to `sample.frequencies` variables
    sample->timestamp = millis();
    for (int i = 0; i < options->accNumSamples; i++)
    {
        double frequencyX = vRealX[i];
        double frequencyY = vRealY[i];
        double frequencyZ = vRealZ[i];
        sample->frequenciesX[i] = frequencyX;
        sample->frequenciesY[i] = frequencyY;
        sample->frequenciesZ[i] = frequencyZ;

        // Reset the vReal arrays
        vRealX[i] = 0.0;
        vRealY[i] = 0.0;
        vRealZ[i] = 0.0;
    }

    if (options->logLevel >= LogLevel::Info)
    {
        Serial.println("Acc data sampled\n");
    }
}

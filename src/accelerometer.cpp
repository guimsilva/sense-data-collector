#include <Arduino.h>
#include <ArduinoJson.h>

#include "accelerometer.h"
#include "Arduino_BMI270_BMM150.h"

Accelerometer::Accelerometer(SampleDataPoint *_sample, SamplerOptions *_options)
    : options(_options),
      sample(_sample),
      nummSamples(_options->accNumSamples),
      samplingFrequency(_options->samplingFrequency),
      vRealX(new double[_options->accNumSamples]),
      vRealY(new double[_options->accNumSamples]),
      vRealZ(new double[_options->accNumSamples])
{
    // Start IMU
    if (!IMU.begin())
    {
        Serial.println("Failed to initialized IMU!");
        while (1)
            ;
    }
    IMU.setContinuousMode();

    samplingPeriodUs = round(1000000 * (1.0 / samplingFrequency));

    if (options->logLevel >= LogLevel::Info)
    {
        Serial.println("Accelerometer initialized");
        float accSampleRate = IMU.accelerationSampleRate();
        Serial.println("IMU acc real sampling rate: ");
        Serial.println(accSampleRate);

        // Double check values from constructor
        Serial.print("Acc samples: ");
        Serial.println(nummSamples);
        Serial.print("Sampling max frequency: ");
        Serial.println(samplingFrequency);
        Serial.println();
    }
}

void Accelerometer::sampleAcceleration()
{
    if (options->logLevel >= LogLevel::Info)
    {
        Serial.print("Sampling acc data for ");
        Serial.print(nummSamples);
        Serial.println(" samples...");
    }
    for (int i = 0; i < nummSamples; i++)
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
    for (int i = 0; i < nummSamples; i++)
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
        Serial.println("Acc data sampled");
    }
}

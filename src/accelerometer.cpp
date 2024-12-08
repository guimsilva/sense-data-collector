#include <Arduino.h>
#include <ArduinoJson.h>

#include "accelerometer.h"
#include "Arduino_BMI270_BMM150.h"

Accelerometer::Accelerometer(SampleDataPoint *_sample, int16_t _samples, int16_t _samplingFrequency)
    : samples(_samples),
      samplingFrequency(_samplingFrequency),
      vRealX(new double[_samples]),
      vRealY(new double[_samples]),
      vRealZ(new double[_samples])
{
    sample = _sample;
    samplingPeriodUs = round(1000000 * (1.0 / samplingFrequency));

    // Double check values from constructor
    Serial.print("Acc samples: ");
    Serial.println(samples);
    Serial.print("Sampling frequency: ");
    Serial.println(samplingFrequency);
    Serial.println();
}

void Accelerometer::sampleAcceleration(bool printResults)
{
    if (printResults)
    {
        Serial.print("Sampling acc data for ");
        Serial.print(samples);
        Serial.println(" samples...");
    }
    for (int i = 0; i < samples; i++)
    {
        microseconds = micros();
        if (IMU.accelerationAvailable())
        {
            IMU.readAcceleration(accX, accY, accZ);
            Serial.println("Read acceleration data! >>> ");
        }
        else
        {
            Serial.println("Failed to read acceleration data! <<< ");
            accX = 0.0;
            accY = 0.0;
            accZ = 0.0;
        }

        // if (printResults)
        // {
        //     Serial.println("Reading acceleration...");
        // }

        // if (printResults)
        // {
        //     Serial.print("Acceleration read: ");
        //     Serial.print(accX);
        //     Serial.print(" ");
        //     Serial.print(accY);
        //     Serial.print(" ");
        //     Serial.println(accZ);
        // }

        // vReal[i] = sqrt(accX * accX + accY * accY + accZ * accZ);
        // vRealX[i] = sqrt(accX * accX);
        // vRealY[i] = sqrt(accY * accY);
        // vRealZ[i] = sqrt(accZ * accZ);
        vRealX[i] = accX;
        vRealY[i] = accY;
        vRealZ[i] = accZ;

        while (micros() < (microseconds + samplingPeriodUs))
            ; // wait for next sample
    }

    // Add the frequencies to sample.frequencies variable
    sample->timestamp = millis();
    for (int i = 0; i < samples; i++)
    {
        double frequencyX = vRealX[i];
        double frequencyY = vRealY[i];
        double frequencyZ = vRealZ[i];
        sample->frequenciesX[i] = frequencyX;
        sample->frequenciesY[i] = frequencyY;
        sample->frequenciesZ[i] = frequencyZ;
    }

    // Reset the vReal arrays
    for (int i = 0; i < samples; i++)
    {
        vRealX[i] = 0.0;
        vRealY[i] = 0.0;
        vRealZ[i] = 0.0;
    }

    if (printResults)
    {
        Serial.println("Acc data sampled");
    }
}

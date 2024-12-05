#include <Arduino.h>
#include <ArduinoJson.h>

#include "vibration.h"
#include "Arduino_BMI270_BMM150.h"
#include "arduinoFFT.h"

/**
 * Sampling data and preparation for FFT conversion
 **/
void Vibration::sampleVibration(bool printResults)
{
    if (printResults)
    {
        Serial.println("Sampling vibration data for " + String(samples) + " samples...");
    }
    for (int i = 0; i < samples; i++)
    {
        microseconds = micros();
        if (IMU.accelerationAvailable())
        {
            if (printResults)
            {
                Serial.println("Reading acceleration...");
            }
            IMU.readAcceleration(accX, accY, accZ);
            if (printResults)
            {
                Serial.println("Acceleration read");
            }
            vReal[i] = sqrt(accX * accX + accY * accY + accZ * accZ);
            vImag[i] = 0;
        }
        else
        {
            if (printResults)
            {
                Serial.println("Acceleration not available");
            }
        }

        while (micros() < (microseconds + samplingPeriodUs))
            ; // wait for next sample
    }
    if (printResults)
    {
        Serial.println("Vibration data sampled");
    }

    computeVibrationFFT(printResults);
}

/**
 * Compute FFT and print results
 **/
void Vibration::computeVibrationFFT(bool printResults)
{
    if (printResults)
    {
        Serial.println("Computing Vibration FFT...");
    }
    /* Print the results of the simulated sampling according to time */
    if (printResults)
    {
        Serial.println("Data:");
        printFFTVector(vReal, samples, sclTime);
    }
    FFT.windowing(FFTWindow::Hamming, FFTDirection::Forward); /* Weigh data */
    if (printResults)
    {
        Serial.println("Weighed data:");
        printFFTVector(vReal, samples, sclTime);
    }
    FFT.compute(FFTDirection::Forward); /* Compute FFT */
    if (printResults)
    {
        Serial.println("Computed Real values:");
        printFFTVector(vReal, samples, sclIndex);
        Serial.println("Computed Imaginary values:");
        printFFTVector(vImag, samples, sclIndex);
    }
    FFT.complexToMagnitude(); /* Compute magnitudes */
    if (printResults)
    {
        Serial.println("Computed magnitudes:");
        printFFTVector(vReal, (samples >> 1), sclFrequency);
    }
    dominantFrequency = FFT.majorPeak();
    if (printResults)
    {
        Serial.println(dominantFrequency, 6);
    }

    // Add the computed frequencies to vibrationSamples variable
    sample->timestamp = millis();
    sample->dominantFrequency = dominantFrequency;
    for (int i = 0; i < samples; i++)
    {
        sample->frequencies[i] = vReal[i];
    }
}

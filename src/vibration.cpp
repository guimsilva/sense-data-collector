#include <Arduino.h>
#include <ArduinoJson.h>
// #include <SPI.h>
// #include <SD.h>

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

        while (micros() < (microseconds + sampling_period_us))
            ; // wait for next sample
    }
    if (printResults)
    {
        Serial.println("Vibration data sampled");
    }
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
    VibrationSample sample;
    sample.timestamp = millis();
    sample.dominantFrequency = dominantFrequency;
    for (int i = 0; i < samples; i++)
    {
        sample.frequencies[i] = vReal[i];
    }

    Serial.print("Adding sample to buffer. Buffer size: ");
    Serial.println(vibrationSamplesBufferSize);
    for (int i = 0; i < vibrationSamplesBufferSize; i++)
    {
        if (vibrationSamples[i].timestamp == 0)
        {
            vibrationSamples[i] = sample;
            Serial.print("Sample added at index: ");
            Serial.println(i);
            break;
        }
    }

    // Log data and reset the buffer when it's full
    if (vibrationSamples[vibrationSamplesBufferSize - 1].timestamp != 0)
    {
        Serial.println("Buffer full. Saving to file and resetting buffer");
        saveVibrationSamplesToFile(printResults);
        for (int i = 0; i < vibrationSamplesBufferSize; i++)
        {
            vibrationSamples[i] = VibrationSample();
        }
    }
    else
    {
        Serial.println("Buffer not full yet");
    }
}

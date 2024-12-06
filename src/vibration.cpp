#include <Arduino.h>
#include <ArduinoJson.h>

#include "vibration.h"
#include "Arduino_BMI270_BMM150.h"
#include "arduinoFFT.h"

Vibration::Vibration(SampleDataPoint *_sample, int16_t _samples, int16_t _samplingFrequency)
    : samples(_samples),
      samplingFrequency(_samplingFrequency),
      vReal(new double[_samples]), vImag(new double[_samples]), dominantFrequency(0.0)
{
    sample = _sample;
    samplingPeriodUs = round(1000000 * (1.0 / samplingFrequency));

    /* Create FFT object */
    FFT = ArduinoFFT<double>(vReal, vImag, _samples, samplingFrequency);
}

void Vibration::printFFTVector(double *vData, uint16_t bufferSize, uint8_t scaleType)
{
    for (uint16_t i = 0; i < bufferSize; i++)
    {
        double abscissa;
        /* Print abscissa value */
        switch (scaleType)
        {
        case sclIndex:
            abscissa = (i * 1.0);
            break;
        case sclTime:
            abscissa = ((i * 1.0) / samplingFrequency);
            break;
        case sclFrequency:
            abscissa = ((i * 1.0 * samplingFrequency) / samples);
            break;
        }
        Serial.print(abscissa, 6);
        if (scaleType == sclFrequency)
            Serial.print("Hz");
        Serial.print(" ");
        Serial.println(vData[i], 4);
    }
    Serial.println();
}

void Vibration::sampleVibration(bool printResults)
{
    if (printResults)
    {
        Serial.print("Sampling vibration data for ");
        Serial.print(samples);
        Serial.println(" samples...");
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
                Serial.print("Acceleration read: ");
                Serial.print(accX);
                Serial.print(" ");
                Serial.print(accY);
                Serial.print(" ");
                Serial.println(accZ);
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

    // Print the sample values
    Serial.println("Sample half of the values values 1:");
    for (int i = 0; i < samples / 2; i++)
    {
        Serial.print(sample->frequencies[i]);
        Serial.print(" ");
    }
    Serial.print("dominantFrequency: ");
    Serial.println(sample->dominantFrequency, 6);

    while (1)
        ;
}

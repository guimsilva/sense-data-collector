#ifndef VIBRATION_UTILS_H
#define VIBRATION_UTILS_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <SPI.h>
#include <SD.h>

#include "Arduino_BMI270_BMM150.h"
#include "arduinoFFT.h"

// x = 512 samples and sampling frequency y = 512 will result in 1 second of sampling (x / y = sec)
#define SAMPLES 512            // Must be a power of 2
#define SAMPLING_FREQUENCY 512 // Hz. Determines maximum frequency that can be analysed by the FFT.

// Used for printing FFT results
#define SCL_INDEX 0x00
#define SCL_TIME 0x01
#define SCL_FREQUENCY 0x02
#define SCL_PLOT 0x03

// IMU sensor
float acc_x, acc_y, acc_z;

// FFT sampling period and time
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

const int vibrationSamplesBufferSize = 10;
VibrationSample vibrationSamples[vibrationSamplesBufferSize];
StaticJsonDocument<10240> doc;

struct VibrationSample
{
    VibrationSample()
        : dominantFrequency(0.0),
          timestamp(0)
    {
        for (int i = 0; i < SAMPLES; ++i)
        {
            frequencies[i] = 0.0;
        }
    }

    double frequencies[SAMPLES];
    double dominantFrequency;
    unsigned long timestamp;
};

/**
 * Sampling data and preparation for FFT conversion
 **/
void sampleVibration()
{
    for (int i = 0; i < SAMPLES; i++)
    {
        microseconds = micros();
        if (IMU.accelerationAvailable())
        {
            IMU.readAcceleration(acc_x, acc_y, acc_z);
            vReal[i] = sqrt(acc_x * acc_x + acc_y * acc_y + acc_z * acc_z);
            vImag[i] = 0;
        }

        while (micros() < (microseconds + sampling_period_us))
            ; // wait for next sample
    }
}

void printFFTVector(double *vData, uint16_t bufferSize, uint8_t scaleType)
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

void saveVibrationSamplesToFile()
{
    doc.clear();
    JsonArray samples = doc.createNestedArray("samples");
    for (int i = 0; i < vibrationSamplesBufferSize; i++)
    {
        JsonObject sample = samples.createNestedObject();
        sample["timestamp"] = vibrationSamples[i].timestamp;
        sample["dominantFrequency"] = vibrationSamples[i].dominantFrequency;
        JsonArray frequencies = sample.createNestedArray("frequencies");
        for (int j = 0; j < SAMPLES; j++)
        {
            frequencies.add(vibrationSamples[i].frequencies[j]);
        }
    }

    File file = SD.open("/vibration_samples_" + String(millis()) + ".json", FILE_WRITE);
    if (!file)
    {
        Serial.println("Failed to open file for writing");
        return;
    }
    serializeJson(doc, file);
    file.close();
}

/**
 * Compute FFT and print results
 **/
void computeVibrationFFT(bool printResults = true)
{
    /* Print the results of the simulated sampling according to time */
    if (printResults)
    {
        Serial.println("Data:");
        printFFTVector(vReal, SAMPLES, SCL_TIME);
    }
    FFT.windowing(FFTWindow::Hamming, FFTDirection::Forward); /* Weigh data */
    if (printResults)
    {
        Serial.println("Weighed data:");
        printFFTVector(vReal, SAMPLES, SCL_TIME);
    }
    FFT.compute(FFTDirection::Forward); /* Compute FFT */
    if (printResults)
    {
        Serial.println("Computed Real values:");
        printFFTVector(vReal, SAMPLES, SCL_INDEX);
        Serial.println("Computed Imaginary values:");
        printFFTVector(vImag, SAMPLES, SCL_INDEX);
    }
    FFT.complexToMagnitude(); /* Compute magnitudes */
    if (printResults)
    {
        Serial.println("Computed magnitudes:");
        printFFTVector(vReal, (SAMPLES >> 1), SCL_FREQUENCY);

        double dominantFrequency = FFT.majorPeak();
        Serial.println(dominantFrequency, 6);
    }

    // Add the computed frequencies to vibrationSamples variable
    VibrationSample sample;
    sample.timestamp = millis();
    sample.dominantFrequency = FFT.majorPeak();
    for (int i = 0; i < SAMPLES; i++)
    {
        sample.frequencies[i] = vReal[i];
    }
    bool sampleAdded = false;
    for (int i = 0; i < vibrationSamplesBufferSize; i++)
    {
        if (vibrationSamples[i].timestamp == 0)
        {
            vibrationSamples[i] = sample;
            sampleAdded = true;
            break;
        }
    }
    // if no sample added, save the samples to a file and reset the buffer
    if (!sampleAdded)
    {
        saveVibrationSamplesToFile();
        // reset the buffer
        for (int i = 0; i < vibrationSamplesBufferSize; i++)
        {
            vibrationSamples[i] = VibrationSample();
        }
    }
}

#endif // VIBRATION_UTILS_H

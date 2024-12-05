#ifndef SAMPLER_H
#define SAMPLER_H

#include <Arduino.h>
#include <ArduinoJson.h>
// #include <SPI.h>
// #include <SD.h>
#include "sample.h"
#include "vibration.h"
#include "barometer.h"

class Sampler
{
private:
    // Vibration samples
    int16_t vibrationSamples;
    // The sample data point reference
    SampleDataPoint *sample;
    // The sample data point buffer
    SampleDataPoint *samples;

    const int16_t samplesBufferSize; // Number of complete samples to be saved before writing to file
    JsonDocument jsonDoc;

    // Vibration instance
    Vibration *vibration;

    // Barometer instance
    Barometer *barometer;

    void saveSamplesToFile(bool printResults = true)
    {
        if (printResults)
        {
            Serial.println("Saving vibration samples to file");
        }

        jsonDoc.clear();
        JsonArray jsonSamples = jsonDoc["samples"].to<JsonArray>();
        for (int i = 0; i < samplesBufferSize; i++)
        {
            if (samples[i].timestamp == 0)
            {
                break;
            }
            JsonObject jsonSample = jsonSamples.add<JsonObject>();
            jsonSample["timestamp"] = samples[i].timestamp;
            jsonSample["dominantFrequency"] = samples[i].dominantFrequency;
            JsonArray frequencies = jsonSample["frequencies"].to<JsonArray>();
            for (int j = 0; j < vibrationSamples; j++)
            {
                frequencies.add(samples[i].frequencies[j]);
            }
        }

        if (printResults)
        {
            serializeJsonPretty(jsonDoc, Serial);
        }

        // File file = SD.open("/vibration_samples_" + String(millis()) + ".json", FILE_WRITE);
        // if (!file)
        // {
        //     Serial.println("Failed to open file for writing");
        //     return;
        // }
        // serializeJson(jsonDoc, file);
        // file.close();
        // jsonDoc.clear();

        if (printResults)
        {
            Serial.println("Vibration samples saved to file");
        }
    }

public:
    Sampler(int16_t _vibrationSamples = 512, int16_t _samplesBufferSize = 10)
        : vibrationSamples(_vibrationSamples),
          samplesBufferSize(_samplesBufferSize),
          sample(new SampleDataPoint(_vibrationSamples)),
          samples(new SampleDataPoint[_samplesBufferSize])
    {
        vibration = new Vibration(sample, 512, 512);
        barometer = new Barometer(sample);
    }

    void sampleData(bool printResults = true)
    {
        Serial.println("Collecting vibration data...");

        vibration->sampleVibration(printResults);

        Serial.println("Collecting barometer data...");

        barometer->sampleBarometer(printResults);

        Serial.print("Adding sample to buffer. Buffer size: ");
        Serial.println(samplesBufferSize);

        // Buffer should always have space for a new sample because it's reset when it's full, right below
        for (int i = 0; i < samplesBufferSize; i++)
        {
            if (samples[i].timestamp == 0)
            {
                samples[i] = *sample;
                Serial.print("Sample added at index: ");
                Serial.println(i);
                break;
            }
        }

        // Log data and reset the buffer when it's full
        if (samples[samplesBufferSize - 1].timestamp != 0)
        {
            Serial.println("Buffer full. Saving to file and resetting buffer");
            saveSamplesToFile(printResults);
            for (int i = 0; i < samplesBufferSize; i++)
            {
                samples[i] = SampleDataPoint();
            }
        }
        else
        {
            Serial.println("Buffer not full yet");
        }
    }
};

#endif // SAMPLER_H

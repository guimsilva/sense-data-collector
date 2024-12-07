#include <Arduino.h>
#include <ArduinoJson.h>
#include <SPI.h>
#include <SD.h>

#include "sampler.h"

void Sampler::duplicateSample(SampleDataPoint *newSample)
{
    newSample->timestamp = sample->timestamp;
    newSample->temperature = sample->temperature;
    newSample->pressure = sample->pressure;
    newSample->altitude = sample->altitude;
    newSample->movingStatus = sample->movingStatus;
    newSample->movingSpeed = sample->movingSpeed;
    newSample->dominantFrequency = sample->dominantFrequency;
    for (int j = 0; j < vibrationSamples; j++)
    {
        newSample->frequencies[j] = sample->frequencies[j];
    }
}

void Sampler::resetSample()
{
    sample->timestamp = 0;
    sample->temperature = 0.0;
    sample->pressure = 0.0;
    sample->altitude = 0.0;
    sample->movingStatus = 0;
    sample->movingSpeed = 0;
    sample->dominantFrequency = 0.0;
    for (int j = 0; j < vibrationSamples; j++)
    {
        sample->frequencies[j] = 0.0;
    }
}

Sampler::Sampler(int16_t _vibrationSamples, int16_t _samplesBufferSize)
    : vibrationSamples(_vibrationSamples),
      samplesBufferSize(_samplesBufferSize),
      sample(new SampleDataPoint(_vibrationSamples)),
      samples(new SampleDataPoint[_samplesBufferSize])
{
    vibration = new Vibration(sample, vibrationSamples, 512);
    barometer = new Barometer(sample);

    for (int i = 0; i < samplesBufferSize; i++)
    {
        samples[i] = SampleDataPoint(_vibrationSamples);
    }
}

void Sampler::saveSamplesToFile(bool printResults)
{
    Serial.println("Saving samples to file");

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

    // serializeJsonPretty(jsonDoc, Serial);
    // while (1)
    //     ;

    char filename[12];
    snprintf(filename, sizeof(filename), "%lu.txt", millis() % 100000000);
    File file = SD.open(filename, FILE_WRITE);
    if (!file)
    {
        Serial.println("Failed to open file for writing");
        return;
    }

    serializeJson(jsonDoc, file);

    file.close();
    jsonDoc.clear();

    Serial.println("Samples saved to file");
}

void Sampler::sampleData(bool printResults)
{
    Serial.println("Collecting vibration data...");
    vibration->sampleVibration(printResults);
    Serial.println("Vibration data collected");

    Serial.println("Collecting barometer data...");
    barometer->sampleBarometer(printResults);
    Serial.println("Barometer data collected");

    Serial.print("Adding sample to buffer. Buffer size: ");
    Serial.println(samplesBufferSize);

    // Buffer should always have space for a new sample because it's reset when it's full, right below
    for (int i = 0; i < samplesBufferSize; i++)
    {
        if (samples[i].timestamp == 0)
        {
            SampleDataPoint *newSample(new SampleDataPoint(vibrationSamples));
            duplicateSample(newSample);
            samples[i] = *newSample;
            resetSample();

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

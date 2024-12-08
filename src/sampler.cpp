#include <Arduino.h>
#include <ArduinoJson.h>
#include <SPI.h>
#include <SD.h>

#include "sampler.h"

void Sampler::copySample(SampleDataPoint *newSample)
{
    newSample->timestamp = sample->timestamp;
    newSample->temperature = sample->temperature;
    newSample->pressure = sample->pressure;
    newSample->altitude = sample->altitude;
    newSample->movingStatus = sample->movingStatus;
    newSample->movingSpeed = sample->movingSpeed;
    for (int j = 0; j < accNumSamples; j++)
    {
        newSample->frequenciesX[j] = sample->frequenciesX[j];
        newSample->frequenciesY[j] = sample->frequenciesY[j];
        newSample->frequenciesZ[j] = sample->frequenciesZ[j];
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
    for (int j = 0; j < accNumSamples; j++)
    {
        sample->frequenciesX[j] = 0.0;
        sample->frequenciesY[j] = 0.0;
        sample->frequenciesZ[j] = 0.0;
    }
}

Sampler::Sampler(SamplerOptions *_options)
    : options(_options),
      accNumSamples(_options->accNumSamples),
      samplesBufferSize(_options->samplesBufferSize),
      sample(new SampleDataPoint(_options->accNumSamples)),
      samples(new SampleDataPoint[_options->samplesBufferSize])
{
    accelerometer = new Accelerometer(sample, options);
    barometer = new Barometer(sample, options);

    for (int i = 0; i < samplesBufferSize; i++)
    {
        samples[i] = SampleDataPoint(options->accNumSamples);
    }

    if (options->saveToSdCard)
    {
        if (!SD.begin(A0))
        {
            Serial.println("Failed to initialize SD card!");
            while (1)
                ;
        }
        if (options->logLevel >= LogLevel::Info)
            Serial.println("SD card initialized");
    }
}

void Sampler::saveSamplesToFile()
{
    if (options->logLevel >= LogLevel::Info)
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
        jsonSample["temperature"] = samples[i].temperature;
        jsonSample["pressure"] = samples[i].pressure;
        jsonSample["altitude"] = samples[i].altitude;
        jsonSample["movingStatus"] = samples[i].movingStatus;
        jsonSample["movingSpeed"] = samples[i].movingSpeed;

        JsonArray frequenciesX = jsonSample["frequenciesX"].to<JsonArray>();
        JsonArray frequenciesY = jsonSample["frequenciesY"].to<JsonArray>();
        JsonArray frequenciesZ = jsonSample["frequenciesZ"].to<JsonArray>();
        for (int j = 0; j < accNumSamples; j++)
        {
            frequenciesX.add(samples[i].frequenciesX[j]);
            frequenciesY.add(samples[i].frequenciesY[j]);
            frequenciesZ.add(samples[i].frequenciesZ[j]);
        }
    }

    if (options->logLevel >= LogLevel::Verbose)
        serializeJsonPretty(jsonDoc, Serial);

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

    if (options->logLevel >= LogLevel::Info)
        Serial.println("Samples saved to file");
}

void Sampler::sampleData()
{
    accelerometer->sampleAcceleration();
    barometer->sampleBarometer();

    // while (1)
    //     ;

    if (options->logLevel >= LogLevel::Info)
    {
        Serial.print("Adding sample to buffer. Buffer size: ");
        Serial.println(samplesBufferSize);
    }

    // Buffer should always have space for a new sample because it's reset when it's full, right below
    for (int i = 0; i < samplesBufferSize; i++)
    {
        if (samples[i].timestamp == 0)
        {
            SampleDataPoint *newSample(new SampleDataPoint(accNumSamples));
            copySample(newSample);
            samples[i] = *newSample;
            resetSample();

            if (options->logLevel >= LogLevel::Verbose)
            {
                Serial.print("Sample added at index: ");
                Serial.println(i);
            }
            break;
        }
    }

    // Log data and reset the buffer when it's full
    if (samples[samplesBufferSize - 1].timestamp != 0)
    {
        if (options->logLevel >= LogLevel::Info)
            Serial.println("Buffer full. Saving to file and resetting buffer");

        saveSamplesToFile();

        for (int i = 0; i < samplesBufferSize; i++)
        {
            samples[i] = SampleDataPoint();
        }
    }
    else
    {
        if (options->logLevel >= LogLevel::Info)
            Serial.println("Buffer not full yet");
    }
}

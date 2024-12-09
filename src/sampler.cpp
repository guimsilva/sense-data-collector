#include <Arduino.h>
#include <ArduinoJson.h>
#include <SPI.h>
#include <SD.h>

#include "sampler.h"

Sampler::Sampler(SamplerOptions *_options)
    : options(_options),
      sample(new SampleDataPoint(_options->accNumSamples)),
      samples(new SampleDataPoint[_options->samplesBufferSize])
{
    if (options->logLevel >= LogLevel::Info)
        Serial.println("\nInitializing sampler\n");

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

    accelerometer = new Accelerometer(sample, options);
    barometer = new Barometer(sample, options);

    previousMillis = 0;
    currentMillis = 0;
    isTimeForDataCollection = false;

    if (options->logLevel >= LogLevel::Info)
    {
        Serial.println("Sampler initialized with options (Interval, AccNumSamples, AccSamplingFrequency, SamplesBufferSize, SaveToSdCard):");
        Serial.println(options->intervalInMillis);
        Serial.println(options->accNumSamples);
        Serial.println(options->accSamplingFrequency);
        Serial.println(options->samplesBufferSize);
        Serial.println(options->saveToSdCard);
        Serial.println();
    }
}

void Sampler::copySample(SampleDataPoint *newSample)
{
    newSample->timestamp = sample->timestamp;
    newSample->temperatureC = sample->temperatureC;
    newSample->pressureKpa = sample->pressureKpa;
    newSample->altitudeMeters = sample->altitudeMeters;
    newSample->movingStatus = sample->movingStatus;
    newSample->movingSpeed = sample->movingSpeed;
    for (int j = 0; j < options->accNumSamples; j++)
    {
        newSample->frequenciesX[j] = sample->frequenciesX[j];
        newSample->frequenciesY[j] = sample->frequenciesY[j];
        newSample->frequenciesZ[j] = sample->frequenciesZ[j];
    }
}

void Sampler::resetSample(SampleDataPoint *_sample)
{
    _sample->timestamp = 0;
    _sample->temperatureC = 0.0;
    _sample->pressureKpa = 0.0;
    _sample->altitudeMeters = 0.0;
    _sample->movingStatus = 0;
    _sample->movingSpeed = 0;
    for (int j = 0; j < options->accNumSamples; j++)
    {
        _sample->frequenciesX[j] = 0.0;
        _sample->frequenciesY[j] = 0.0;
        _sample->frequenciesZ[j] = 0.0;
    }
}

void Sampler::saveSamplesToFile()
{
    if (options->logLevel >= LogLevel::Info)
        Serial.println("Saving samples to file");

    jsonDoc.clear();
    JsonArray jsonSamples = jsonDoc["samples"].to<JsonArray>();
    for (int i = 0; i < options->samplesBufferSize; i++)
    {
        if (samples[i].timestamp == 0)
        {
            break;
        }
        JsonObject jsonSample = jsonSamples.add<JsonObject>();
        jsonSample["timestamp"] = samples[i].timestamp;
        jsonSample["temperatureC"] = samples[i].temperatureC;
        jsonSample["pressureKpa"] = samples[i].pressureKpa;
        jsonSample["altitudeM"] = samples[i].altitudeMeters;
        jsonSample["movingStatus"] = samples[i].movingStatus;
        jsonSample["movingSpeed"] = samples[i].movingSpeed;

        JsonArray frequenciesX = jsonSample["frequenciesX"].to<JsonArray>();
        JsonArray frequenciesY = jsonSample["frequenciesY"].to<JsonArray>();
        JsonArray frequenciesZ = jsonSample["frequenciesZ"].to<JsonArray>();
        for (int j = 0; j < options->accNumSamples; j++)
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

    char filename[13];
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
    currentMillis = millis();
    isTimeForDataCollection = previousMillis == 0 || currentMillis - previousMillis >= options->intervalInMillis;
    if (!isTimeForDataCollection)
        return;

    previousMillis = currentMillis;

    accelerometer->sampleAcceleration();
    barometer->sampleBarometer();

    // while (1)
    //     ;

    // Buffer should always have space for a new sample because it's reset when it's full, right below
    for (int i = 0; i < options->samplesBufferSize; i++)
    {
        if (samples[i].timestamp == 0)
        {
            if (options->logLevel >= LogLevel::Info)
            {
                Serial.print("Adding sample at index: ");
                Serial.println(i);
            }

            SampleDataPoint *newSample(new SampleDataPoint(options->accNumSamples));
            copySample(newSample);
            samples[i] = *newSample;
            resetSample(sample);

            if (options->logLevel >= LogLevel::Info)
            {
                Serial.println("Sample added");
            }
            break;
        }
    }

    // Log data and reset the buffer when it's full
    if (samples[options->samplesBufferSize - 1].timestamp != 0)
    {
        if (options->logLevel >= LogLevel::Info)
            Serial.println("\nBuffer full. Saving to file and resetting buffer");

        if (options->saveToSdCard)
        {
            saveSamplesToFile();
            if (options->logLevel >= LogLevel::Info)
                Serial.println("Saved to SD card\n");
        }
        else
        {
            if (options->logLevel >= LogLevel::Info)
                Serial.println("Not saving to SD card\n");
        }

        if (options->logLevel >= LogLevel::Verbose)
            Serial.println("Resetting buffer\n");

        // Reset each sample in the buffer
        for (int i = 0; i < options->samplesBufferSize; i++)
        {
            resetSample(&samples[i]);
        }

        if (options->logLevel >= LogLevel::Verbose)
            Serial.println("Buffer reset\n");
    }
    else
    {
        if (options->logLevel >= LogLevel::Info)
            Serial.println("Buffer not full yet\n");
    }
}

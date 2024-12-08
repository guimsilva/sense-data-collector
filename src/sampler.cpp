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
    for (int j = 0; j < accSamples; j++)
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
    for (int j = 0; j < accSamples; j++)
    {
        sample->frequenciesX[j] = 0.0;
        sample->frequenciesY[j] = 0.0;
        sample->frequenciesZ[j] = 0.0;
    }
}

Sampler::Sampler(int16_t _accSamples, int16_t _samplingFrequency, int16_t _samplesBufferSize)
    : accSamples(_accSamples),
      samplesBufferSize(_samplesBufferSize),
      sample(new SampleDataPoint(_accSamples)),
      samples(new SampleDataPoint[_samplesBufferSize])
{
    accelerometer = new Accelerometer(sample, accSamples, _samplingFrequency);
    barometer = new Barometer(sample);

    for (int i = 0; i < samplesBufferSize; i++)
    {
        samples[i] = SampleDataPoint(_accSamples);
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
        jsonSample["temperature"] = samples[i].temperature;
        jsonSample["pressure"] = samples[i].pressure;
        jsonSample["altitude"] = samples[i].altitude;
        jsonSample["movingStatus"] = samples[i].movingStatus;
        jsonSample["movingSpeed"] = samples[i].movingSpeed;

        JsonArray frequenciesX = jsonSample["frequenciesX"].to<JsonArray>();
        JsonArray frequenciesY = jsonSample["frequenciesY"].to<JsonArray>();
        JsonArray frequenciesZ = jsonSample["frequenciesZ"].to<JsonArray>();
        for (int j = 0; j < accSamples; j++)
        {
            frequenciesX.add(samples[i].frequenciesX[j]);
            frequenciesY.add(samples[i].frequenciesY[j]);
            frequenciesZ.add(samples[i].frequenciesZ[j]);
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
    accelerometer->sampleAcceleration(printResults);
    barometer->sampleBarometer(printResults);

    // while (1)
    //     ;

    Serial.print("Adding sample to buffer. Buffer size: ");
    Serial.println(samplesBufferSize);

    // Buffer should always have space for a new sample because it's reset when it's full, right below
    for (int i = 0; i < samplesBufferSize; i++)
    {
        if (samples[i].timestamp == 0)
        {
            SampleDataPoint *newSample(new SampleDataPoint(accSamples));
            copySample(newSample);
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
        // saveSamplesToFile(printResults);
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

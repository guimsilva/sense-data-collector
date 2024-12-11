#include <Arduino.h>
#include <ArduinoJson.h>
#include <SPI.h>
#include <SD.h>

#include "Arduino_BMI270_BMM150.h"
#include "sampler.h"

Sampler::Sampler(SamplerOptions *_samplerOptions)
    : samplerOptions(_samplerOptions),
      sampleDataPoint(new SampleDataPoint(_samplerOptions->accNumSamples)),
      sampleDataPoints(new SampleDataPoint[_samplerOptions->sampleDataPointBufferSize])
{
    if (samplerOptions->logLevel >= LogLevel::Info)
        Serial.println("\nInitializing sampler\n");

    if (samplerOptions->saveToSdCard)
    {
        if (!SD.begin(A0))
        {
            Serial.println("Failed to initialize SD card!");
            while (1)
                ;
        }
        if (samplerOptions->logLevel >= LogLevel::Info)
            Serial.println("SD card initialized");
    }
    else
    {
        if (samplerOptions->logLevel >= LogLevel::Info)
            Serial.println("Skipping SD card");
    }

    accelerometer = new Accelerometer(samplerOptions);
    barometer = new Barometer(sampleDataPoint, samplerOptions);
    microphone = new Microphone(sampleDataPoint, samplerOptions);

    previousMillis = 0;
    currentMillis = 0;
    isTimeForDataCollection = false;

    if (samplerOptions->logLevel >= LogLevel::Info)
    {
        Serial.println("Sampler initialized with options (Interval, AccNumSamples, AccSamplingFrequency, AccDataPointBufferSize, SaveToSdCard):");
        Serial.println(samplerOptions->intervalInMillis);
        Serial.println(samplerOptions->accNumSamples);
        Serial.println(samplerOptions->accSamplingFrequency);
        Serial.println(samplerOptions->sampleDataPointBufferSize);
        Serial.println(samplerOptions->saveToSdCard);
        Serial.println();
    }
}

void Sampler::copySample(SampleDataPoint *newSample)
{
    newSample->timestamp = sampleDataPoint->timestamp;
    newSample->temperatureC = sampleDataPoint->temperatureC;
    newSample->pressureKpa = sampleDataPoint->pressureKpa;
    newSample->altitudeMeters = sampleDataPoint->altitudeMeters;
    newSample->movingStatus = sampleDataPoint->movingStatus;
    newSample->movingSpeed = sampleDataPoint->movingSpeed;

    for (int j = 0; j < samplerOptions->accNumSamples; j++)
    {
        newSample->accFrequenciesX[j] = sampleDataPoint->accFrequenciesX[j];
        newSample->accFequenciesY[j] = sampleDataPoint->accFequenciesY[j];
        newSample->accFrequenciesZ[j] = sampleDataPoint->accFrequenciesZ[j];
    }

    for (int j = 0; j < sizeof(sampleDataPoint->audioBuffer) / sizeof(sampleDataPoint->audioBuffer[0]); j++)
    {
        newSample->audioBuffer[j] = sampleDataPoint->audioBuffer[j];
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

    for (int j = 0; j < samplerOptions->accNumSamples; j++)
    {
        _sample->accFrequenciesX[j] = 0.0;
        _sample->accFequenciesY[j] = 0.0;
        _sample->accFrequenciesZ[j] = 0.0;
    }

    for (int j = 0; j < static_cast<int>(sizeof(_sample->audioBuffer) / sizeof(_sample->audioBuffer[0])); j++)
    {
        _sample->audioBuffer[j] = 0;
    }
}

void Sampler::saveSamplesToFile()
{
    if (samplerOptions->logLevel >= LogLevel::Info)
        Serial.println("Saving samples to file");

    jsonDoc.clear();
    JsonArray jsonSamples = jsonDoc["samples"].to<JsonArray>();
    for (int i = 0; i < samplerOptions->sampleDataPointBufferSize; i++)
    {
        if (sampleDataPoints[i].timestamp == 0)
        {
            break;
        }
        JsonObject jsonSample = jsonSamples.add<JsonObject>();
        jsonSample["timestamp"] = sampleDataPoints[i].timestamp;
        jsonSample["temperatureC"] = sampleDataPoints[i].temperatureC;
        jsonSample["pressureKpa"] = sampleDataPoints[i].pressureKpa;
        jsonSample["altitudeM"] = sampleDataPoints[i].altitudeMeters;
        jsonSample["movingStatus"] = sampleDataPoints[i].movingStatus;
        jsonSample["movingSpeed"] = sampleDataPoints[i].movingSpeed;

        JsonArray frequenciesX = jsonSample["frequenciesX"].to<JsonArray>();
        JsonArray frequenciesY = jsonSample["frequenciesY"].to<JsonArray>();
        JsonArray frequenciesZ = jsonSample["frequenciesZ"].to<JsonArray>();
        for (int j = 0; j < samplerOptions->accNumSamples; j++)
        {
            frequenciesX.add(sampleDataPoints[i].accFrequenciesX[j]);
            frequenciesY.add(sampleDataPoints[i].accFequenciesY[j]);
            frequenciesZ.add(sampleDataPoints[i].accFrequenciesZ[j]);
        }
    }

    if (samplerOptions->logLevel >= LogLevel::Verbose)
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

    if (samplerOptions->logLevel >= LogLevel::Info)
        Serial.println("Samples saved to file");
}

void Sampler::sampleFrequencies()
{
    if (samplerOptions->logLevel >= LogLevel::Info)
    {
        Serial.println("Sampling acc data...");
    }
    for (int i = 0; i < samplerOptions->accNumSamples; i++)
    {
        currentMicroseconds = micros();
        if (IMU.accelerationAvailable())
        {
            IMU.readAcceleration(accelerometer->accX, accelerometer->accY, accelerometer->accZ);
            if (samplerOptions->logLevel >= LogLevel::Verbose)
                Serial.println("Read acceleration data! >>> ");
        }
        else
        {
            if (samplerOptions->logLevel >= LogLevel::Verbose)
                Serial.println("Failed to read acceleration data! <<< ");

            accelerometer->accX = 0.0;
            accelerometer->accY = 0.0;
            accelerometer->accZ = 0.0;
        }

        if (samplerOptions->logLevel >= LogLevel::Verbose)
        {
            Serial.print("Acceleration read: ");
            Serial.print(accelerometer->accX);
            Serial.print(" ");
            Serial.print(accelerometer->accY);
            Serial.print(" ");
            Serial.println(accelerometer->accZ);
        }

        accelerometer->vRealX[i] = accelerometer->accX;
        accelerometer->vRealY[i] = accelerometer->accY;
        accelerometer->vRealZ[i] = accelerometer->accZ;

        // Call it once now then keep calling it until the next sample
        microphone->bufferCallback();

        while (micros() < (currentMicroseconds + accelerometer->samplingPeriodUs))
        {
            microphone->bufferCallback();
        }; // wait for next sample
    }

    // Add the frequencies to `sample.frequencies` variables
    sampleDataPoint->timestamp = millis();
    for (int i = 0; i < samplerOptions->accNumSamples; i++)
    {
        sampleDataPoint->accFrequenciesX[i] = accelerometer->vRealX[i];
        sampleDataPoint->accFequenciesY[i] = accelerometer->vRealY[i];
        sampleDataPoint->accFrequenciesZ[i] = accelerometer->vRealZ[i];

        // Reset the vReal arrays
        accelerometer->vRealX[i] = 0.0;
        accelerometer->vRealY[i] = 0.0;
        accelerometer->vRealZ[i] = 0.0;
    }

    if (samplerOptions->logLevel >= LogLevel::Info)
    {
        Serial.println("Acc data sampled\n");
    }
}

void Sampler::sampleData()
{
    currentMillis = millis();
    isTimeForDataCollection = previousMillis == 0 || currentMillis - previousMillis >= samplerOptions->intervalInMillis;
    if (!isTimeForDataCollection)
        return;

    previousMillis = currentMillis;

    // Sample Barometer first as the frequencies will block execution for some time
    barometer->sampleBarometer();
    // Audio sampling is asynchronous, so it won't block sampleFrequencies
    microphone->startAudioSampling();
    // Sample acc data at the same time as audio sampling
    sampleFrequencies();
    // Stop audio sampling
    microphone->stopAudioSampling();

    // while (1)
    //     ;

    // Buffer should always have space for a new acc data point because it's reset when it's full, right below
    for (int i = 0; i < samplerOptions->sampleDataPointBufferSize; i++)
    {
        if (sampleDataPoints[i].timestamp == 0)
        {
            if (samplerOptions->logLevel >= LogLevel::Info)
            {
                Serial.print("Adding sample at index: ");
                Serial.println(i);
            }

            copySample(&sampleDataPoints[i]);
            resetSample(sampleDataPoint);

            if (samplerOptions->logLevel >= LogLevel::Info)
            {
                Serial.println("Sample added");
            }
            break;
        }
    }

    // Log data and reset the buffer when it's full
    if (sampleDataPoints[samplerOptions->sampleDataPointBufferSize - 1].timestamp != 0)
    {
        if (samplerOptions->logLevel >= LogLevel::Info)
            Serial.println("\nBuffer full. Saving to file and resetting buffer");

        if (samplerOptions->saveToSdCard)
        {
            saveSamplesToFile();
            if (samplerOptions->logLevel >= LogLevel::Info)
                Serial.println("Saved to SD card\n");
        }
        else
        {
            if (samplerOptions->logLevel >= LogLevel::Info)
                Serial.println("Not saving to SD card\n");
        }

        if (samplerOptions->logLevel >= LogLevel::Verbose)
            Serial.println("Resetting buffer\n");

        // Reset each sample in the buffer data points
        for (int i = 0; i < samplerOptions->sampleDataPointBufferSize; i++)
        {
            resetSample(&sampleDataPoints[i]);
        }

        if (samplerOptions->logLevel >= LogLevel::Verbose)
            Serial.println("Buffer reset\n");
    }
    else
    {
        if (samplerOptions->logLevel >= LogLevel::Info)
            Serial.println("Buffer not full yet\n");
    }
}

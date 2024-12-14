#include <Arduino.h>
#include <ArduinoJson.h>
#include <SPI.h>
#include <SD.h>

#include "Arduino_BMI270_BMM150.h"
#include "sampler.h"

Sampler::Sampler(SamplerConfig *_samplerConfig)
    : samplerConfig(_samplerConfig),
      sampleDataPoint(new SampleDataPoint(_samplerConfig->accOptions->accNumSamples)),
      sampleDataPoints(new SampleDataPoint[_samplerConfig->samplerOptions->sampleDataPointBufferSize])
{
    if (samplerConfig->samplerOptions->logLevel >= LogLevel::Info)
        Serial.println("\nInitializing sampler\n");

    if (samplerConfig->samplerOptions->saveToSdCard)
    {
        if (!SD.begin(A0))
        {
            Serial.println("Failed to initialize SD card!");
            while (1)
                ;
        }
        if (samplerConfig->samplerOptions->logLevel >= LogLevel::Info)
            Serial.println("SD card initialized");
    }
    else
    {
        if (samplerConfig->samplerOptions->logLevel >= LogLevel::Info)
            Serial.println("Skipping SD card");
    }

    accelerometer = new Accelerometer(samplerConfig);
    barometer = new Barometer(sampleDataPoint, samplerConfig);
    microphone = new Microphone(sampleDataPoint, samplerConfig);

    for (int i = 0; i < sizeof(samplerConfig->samplerOptions->triggers) / sizeof(samplerConfig->samplerOptions->triggers[0]); i++)
    {
        if (samplerConfig->samplerOptions->triggers[i] == Triggers::Microphone)
        {
            microphone->startAudioSampling();
        }
    }

    previousMillis = 0;
    currentMillis = 0;
    startDataCollection = false;

    if (samplerConfig->samplerOptions->logLevel >= LogLevel::Info)
    {
        Serial.println("Sampler initialized with config..:");
        Serial.println("Triggers:");
        for (int i = 0; i < sizeof(samplerConfig->samplerOptions->triggers) / sizeof(samplerConfig->samplerOptions->triggers[0]); i++)
        {
            Serial.print((int)samplerConfig->samplerOptions->triggers[i]);
            if (i < sizeof(samplerConfig->samplerOptions->triggers) / sizeof(samplerConfig->samplerOptions->triggers[0]) - 1)
            {
                Serial.print(", ");
            }
        }
        Serial.println();
        Serial.println("Data sensors:");
        for (int i = 0; i < sizeof(samplerConfig->samplerOptions->dataSensors) / sizeof(samplerConfig->samplerOptions->dataSensors[0]); i++)
        {
            Serial.print((int)samplerConfig->samplerOptions->dataSensors[i]);
            if (i < sizeof(samplerConfig->samplerOptions->dataSensors) / sizeof(samplerConfig->samplerOptions->dataSensors[0]) - 1)
            {
                Serial.print(", ");
            }
        }
        Serial.println();
        Serial.println("Sampler Options (SampleDataPointBufferSize, SaveToSdCard):");
        Serial.println(samplerConfig->samplerOptions->sampleDataPointBufferSize);
        Serial.println(samplerConfig->samplerOptions->saveToSdCard);
        Serial.println("Acc Options (AccNumSamples, AccSamplingFrequency, AccSamplingLengthMs):");
        Serial.println(samplerConfig->accOptions->accNumSamples);
        Serial.println(samplerConfig->accOptions->accSamplingFrequency);
        Serial.println(samplerConfig->accOptions->accSamplingLengthMs);
        Serial.println("Mic Options (MicSamplingRate, MicNumSamples):");
        Serial.println(samplerConfig->micOptions->micSamplingRate);
        Serial.println(samplerConfig->micOptions->micNumSamples);
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
    newSample->movingDirection = sampleDataPoint->movingDirection;
    newSample->movingSpeed = sampleDataPoint->movingSpeed;

    for (int j = 0; j < samplerConfig->accOptions->accNumSamples; j++)
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
    _sample->movingStatus = MovingStatus::Stopped;
    _sample->movingSpeed = 0;

    for (int j = 0; j < samplerConfig->accOptions->accNumSamples; j++)
    {
        _sample->accFrequenciesX[j] = 0.0;
        _sample->accFequenciesY[j] = 0.0;
        _sample->accFrequenciesZ[j] = 0.0;
    }

    for (int j = 0; j < samplerConfig->micOptions->micNumSamples; j++)
    {
        _sample->audioBuffer[j] = 0;
    }
}

void Sampler::saveSamplesToFile()
{
    if (samplerConfig->samplerOptions->logLevel >= LogLevel::Info)
        Serial.println("Saving samples to file");

    jsonDoc.clear();
    JsonArray jsonSamples = jsonDoc["samples"].to<JsonArray>();
    for (int i = 0; i < samplerConfig->samplerOptions->sampleDataPointBufferSize; i++)
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
        jsonSample["movingStatus"] = (int)sampleDataPoints[i].movingStatus;
        jsonSample["movingDirection"] = (int)sampleDataPoints[i].movingDirection;
        jsonSample["movingSpeed"] = sampleDataPoints[i].movingSpeed;

        JsonArray frequenciesX = jsonSample["frequenciesX"].to<JsonArray>();
        JsonArray frequenciesY = jsonSample["frequenciesY"].to<JsonArray>();
        JsonArray frequenciesZ = jsonSample["frequenciesZ"].to<JsonArray>();
        for (int j = 0; j < samplerConfig->accOptions->accNumSamples; j++)
        {
            frequenciesX.add(sampleDataPoints[i].accFrequenciesX[j]);
            frequenciesY.add(sampleDataPoints[i].accFequenciesY[j]);
            frequenciesZ.add(sampleDataPoints[i].accFrequenciesZ[j]);
        }

        JsonArray audioBuffer = jsonSample["audioBuffer"].to<JsonArray>();
        for (int j = 0; j < samplerConfig->micOptions->micNumSamples; j++)
        {
            audioBuffer.add(sampleDataPoints[i].audioBuffer[j]);
        }
    }

    // if (samplerConfig->samplerOptions->logLevel >= LogLevel::Verbose)
    //     serializeJsonPretty(jsonDoc, Serial);

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

    if (samplerConfig->samplerOptions->logLevel >= LogLevel::Info)
        Serial.println("Samples saved to file");
}

void Sampler::sampleFrequencies()
{
    if (!samplerConfig->samplerOptions->hasAccSensor)
        return;

    if (samplerConfig->samplerOptions->logLevel >= LogLevel::Info)
    {
        Serial.println("Sampling acc data...");
    }
    for (int i = 0; i < samplerConfig->accOptions->accNumSamples; i++)
    {
        currentMicroseconds = micros();
        if (IMU.accelerationAvailable())
        {
            IMU.readAcceleration(accelerometer->accX, accelerometer->accY, accelerometer->accZ);
            if (samplerConfig->samplerOptions->logLevel >= LogLevel::Verbose)
                Serial.println("Read acceleration data! >>> ");
        }
        else
        {
            if (samplerConfig->samplerOptions->logLevel >= LogLevel::Verbose)
                Serial.println("Failed to read acceleration data! <<< ");

            accelerometer->accX = 0.0;
            accelerometer->accY = 0.0;
            accelerometer->accZ = 0.0;
        }

        if (samplerConfig->samplerOptions->logLevel >= LogLevel::Verbose)
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
    for (int i = 0; i < samplerConfig->accOptions->accNumSamples; i++)
    {
        sampleDataPoint->accFrequenciesX[i] = accelerometer->vRealX[i];
        sampleDataPoint->accFequenciesY[i] = accelerometer->vRealY[i];
        sampleDataPoint->accFrequenciesZ[i] = accelerometer->vRealZ[i];

        // Reset the vReal arrays
        accelerometer->vRealX[i] = 0.0;
        accelerometer->vRealY[i] = 0.0;
        accelerometer->vRealZ[i] = 0.0;
    }

    if (samplerConfig->samplerOptions->logLevel >= LogLevel::Info)
    {
        Serial.println("Acc data sampled\n");
    }
}

void Sampler::checkTriggers()
{
    if (samplerConfig->samplerOptions->logLevel >= LogLevel::Verbose)
    {
        Serial.println("Checking triggers");
    }

    currentMillis = millis();

    if (samplerConfig->samplerOptions->hasIntervalTrigger)
    {
        startDataCollection = previousMillis == 0 || currentMillis - previousMillis >= samplerConfig->samplerOptions->intervalMsTrigger;
    }
    else if (samplerConfig->samplerOptions->hasAccMovementTrigger)
    {
        // @todo: Implement acc movement trigger
    }
    else if (samplerConfig->samplerOptions->hasAccRawTrigger)
    {
        if (IMU.accelerationAvailable())
        {
            IMU.readAcceleration(accelerometer->accX, accelerometer->accY, accelerometer->accZ);
            if (samplerConfig->samplerOptions->logLevel >= LogLevel::Verbose)
                Serial.println("Read acceleration data! >>> ");
        }
        else
        {
            if (samplerConfig->samplerOptions->logLevel >= LogLevel::Verbose)
                Serial.println("Failed to read acceleration data! <<< ");

            accelerometer->accX = 0.0;
            accelerometer->accY = 0.0;
            accelerometer->accZ = 0.0;
        }

        if (abs(accelerometer->accX) > samplerConfig->samplerOptions->accThresholdTrigger[0] ||
            abs(accelerometer->accY) > samplerConfig->samplerOptions->accThresholdTrigger[1] ||
            abs(accelerometer->accZ) > samplerConfig->samplerOptions->accThresholdTrigger[2])
        {
            startDataCollection = true;
        }
    }
    else if (samplerConfig->samplerOptions->hasMicTrigger)
    {
        startDataCollection = microphone->isTriggered();
    }

    if (samplerConfig->samplerOptions->logLevel >= LogLevel::Verbose)
    {
        Serial.println("Triggers checked");
    }

    if (!startDataCollection)
        return;

    sampleData();
}

void Sampler::sampleData()
{
    if (samplerConfig->samplerOptions->logLevel >= LogLevel::Verbose)
    {
        Serial.println("Sampling data...");
    }

    previousMillis = currentMillis;

    // Sample Barometer first as the frequencies will block execution for some time
    barometer->sampleBarometer();

    // Audio sampling is asynchronous, so it won't block sampleFrequencies
    microphone->startAudioSampling();

    // If it has a mic trigger and senor data and not collecting acc data, then wait for the mic to sample the expected number of samples as it would be done in sampleFrequencies
    if (samplerConfig->samplerOptions->hasMicTrigger && samplerConfig->samplerOptions->hasMicSensor && !samplerConfig->samplerOptions->hasAccSensor)
    {
        while (samplerConfig->micOptions->micSamplingLengthMs < (currentMillis - previousMillis))
        {
            microphone->bufferCallback();
            currentMillis = millis();
        }
    }

    // Sample acc data at the same time as audio sampling
    sampleFrequencies();

    // Stop audio sampling
    microphone->stopAudioSampling();

    // while (1)
    //     ;

    if (samplerConfig->samplerOptions->logLevel >= LogLevel::Verbose)
    {
        Serial.println("Data sampled. Copying and reseting sample...\n");
    }

    // Buffer should always have space for a new acc data point because it's reset when it's full, right below
    for (int i = 0; i < samplerConfig->samplerOptions->sampleDataPointBufferSize; i++)
    {
        if (sampleDataPoints[i].timestamp == 0)
        {
            if (samplerConfig->samplerOptions->logLevel >= LogLevel::Info)
            {
                Serial.print("Adding sample at index: ");
                Serial.println(i);
            }

            copySample(&sampleDataPoints[i]);
            resetSample(sampleDataPoint);

            if (samplerConfig->samplerOptions->logLevel >= LogLevel::Info)
            {
                Serial.println("Sample added");
            }
            break;
        }
    }

    // Log data and reset the buffer when it's full
    if (sampleDataPoints[samplerConfig->samplerOptions->sampleDataPointBufferSize - 1].timestamp != 0)
    {
        if (samplerConfig->samplerOptions->logLevel >= LogLevel::Info)
            Serial.println("\nBuffer full. Saving to file and resetting buffer");

        if (samplerConfig->samplerOptions->saveToSdCard)
        {
            saveSamplesToFile();
            if (samplerConfig->samplerOptions->logLevel >= LogLevel::Info)
                Serial.println("Saved to SD card\n");
        }
        else
        {
            if (samplerConfig->samplerOptions->logLevel >= LogLevel::Info)
                Serial.println("Skipping and not saving to SD card\n");
        }

        if (samplerConfig->samplerOptions->logLevel >= LogLevel::Verbose)
            Serial.println("Resetting buffer\n");

        // Reset each sample in the buffer data points
        for (int i = 0; i < samplerConfig->samplerOptions->sampleDataPointBufferSize; i++)
        {
            resetSample(&sampleDataPoints[i]);
        }

        if (samplerConfig->samplerOptions->logLevel >= LogLevel::Verbose)
            Serial.println("Buffer reset\n");
    }
    else
    {
        if (samplerConfig->samplerOptions->logLevel >= LogLevel::Info)
            Serial.println("Buffer not full yet\n");
    }
}

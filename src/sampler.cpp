#include <Arduino.h>
#include <ArduinoJson.h>
#include <SPI.h>
#include <SD.h>

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

    if (samplerConfig->samplerOptions->hasAccSensor)
    {
        accelerometer = new Accelerometer(samplerConfig);
    }
    if (samplerConfig->samplerOptions->hasBarSensor)
    {
        barometer = new Barometer(sampleDataPoint, samplerConfig);
    }
    if (samplerConfig->samplerOptions->hasMicSensor)
    {
        microphone = new Microphone(sampleDataPoint, samplerConfig);
    }

    previousMillis = 0;
    currentMillis = 0;

    if (samplerConfig->samplerOptions->logLevel >= LogLevel::Info)
    {
        Serial.println("Sampler initialized with config..:");
        Serial.println("Triggers:");
        if (samplerConfig->samplerOptions->hasIntervalTrigger)
        {
            Serial.println("Interval");
        }
        if (samplerConfig->samplerOptions->hasMovementTrigger)
        {
            Serial.println("AccMovement");
        }
        if (samplerConfig->samplerOptions->hasAccRawTrigger)
        {
            Serial.println("AccRaw");
        }
        if (samplerConfig->samplerOptions->hasMicTrigger)
        {
            Serial.println("Mic");
        }
        Serial.println();
        Serial.println("Data sensors:");
        if (samplerConfig->samplerOptions->hasAccSensor)
        {
            Serial.println("Accelerometer");
        }
        if (samplerConfig->samplerOptions->hasMicSensor)
        {
            Serial.println("Microphone");
        }
        if (samplerConfig->samplerOptions->hasBarSensor)
        {
            Serial.println("Barometer");
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

void Sampler::copyFromSampleDataPoint(SampleDataPoint *destinationSampleDataPoint)
{
    destinationSampleDataPoint->timestamp = sampleDataPoint->timestamp;
    destinationSampleDataPoint->temperatureC = sampleDataPoint->temperatureC;
    destinationSampleDataPoint->pressureKpa = sampleDataPoint->pressureKpa;
    destinationSampleDataPoint->altitudeMeters = sampleDataPoint->altitudeMeters;
    destinationSampleDataPoint->movingStatus = sampleDataPoint->movingStatus;
    destinationSampleDataPoint->movingDirection = sampleDataPoint->movingDirection;
    destinationSampleDataPoint->movingSpeed = sampleDataPoint->movingSpeed;

    for (int j = 0; j < samplerConfig->accOptions->accNumSamples; j++)
    {
        destinationSampleDataPoint->accFrequenciesX[j] = sampleDataPoint->accFrequenciesX[j];
        destinationSampleDataPoint->accFequenciesY[j] = sampleDataPoint->accFequenciesY[j];
        destinationSampleDataPoint->accFrequenciesZ[j] = sampleDataPoint->accFrequenciesZ[j];
    }

    for (int j = 0; j < samplerConfig->micOptions->micNumSamples; j++)
    {
        destinationSampleDataPoint->audioBuffer[j] = sampleDataPoint->audioBuffer[j];
    }
}

void Sampler::resetSampleDataPoint(SampleDataPoint *targetSampleDataPoint)
{
    targetSampleDataPoint->timestamp = 0;
    targetSampleDataPoint->temperatureC = 0.0;
    targetSampleDataPoint->pressureKpa = 0.0;
    targetSampleDataPoint->altitudeMeters = 0.0;
    targetSampleDataPoint->movingStatus = MovingStatus::Stopped;
    targetSampleDataPoint->movingDirection = MovingDirection::None;
    targetSampleDataPoint->movingSpeed = 0;

    for (int j = 0; j < samplerConfig->accOptions->accNumSamples; j++)
    {
        targetSampleDataPoint->accFrequenciesX[j] = 0.0;
        targetSampleDataPoint->accFequenciesY[j] = 0.0;
        targetSampleDataPoint->accFrequenciesZ[j] = 0.0;
    }

    for (int j = 0; j < samplerConfig->micOptions->micNumSamples; j++)
    {
        targetSampleDataPoint->audioBuffer[j] = 0;
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
        Serial.println("Sampling frequency data...");
    }
    for (int i = 0; i < samplerConfig->accOptions->accNumSamples; i++)
    {
        currentMicroseconds = micros();

        accelerometer->sampleAccelerometer();

        accelerometer->vRealX[i] = accelerometer->accX;
        accelerometer->vRealY[i] = accelerometer->accY;
        accelerometer->vRealZ[i] = accelerometer->accZ;

        // Reset the last axies raw data
        accelerometer->accX = 0.0;
        accelerometer->accY = 0.0;
        accelerometer->accZ = 0.0;

        if (samplerConfig->samplerOptions->hasMicSensor)
            // Call it once now then keep calling it until the next sample
            microphone->bufferCallback();

        while (micros() < (currentMicroseconds + accelerometer->samplingPeriodUs))
        {
            if (samplerConfig->samplerOptions->hasMicSensor)
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

/**
 * The reason to have this here intead of in the accelerometer/barometer class is because
 * it's possible that acc data will also be included at some point, to improve the detection of movement.
 */
bool Sampler::hasNewMovement()
{
    if (samplerConfig->samplerOptions->logLevel >= LogLevel::Verbose)
    {
        Serial.println("Detecting vertical movement...");
    }

    static MovingTrigger lastTrigger = {sampleDataPoint->movingStatus, sampleDataPoint->movingDirection};
    bool hasNewTrigger = false;

    barometer->samplePressure(false);

    for (int i = 0; i < samplerConfig->samplerOptions->sizeofMovementTriggers; i++)
    {
        if (samplerConfig->samplerOptions->movementTriggers[i].movingStatus == sampleDataPoint->movingStatus &&
            samplerConfig->samplerOptions->movementTriggers[i].movingDirection == sampleDataPoint->movingDirection)
        {
            // If the last trigger is the same as the current one, then don't start data collection
            if (lastTrigger.movingStatus == sampleDataPoint->movingStatus && lastTrigger.movingDirection == sampleDataPoint->movingDirection)
            {
                break;
            }

            lastTrigger.movingStatus = sampleDataPoint->movingStatus;
            lastTrigger.movingDirection = sampleDataPoint->movingDirection;
            hasNewTrigger = true;
            break;
        }
    }

    return hasNewTrigger;
}

void Sampler::checkTriggers()
{
    if (samplerConfig->samplerOptions->logLevel >= LogLevel::Verbose)
    {
        Serial.println("Checking triggers");
    }

    currentMillis = millis();
    bool startDataCollection = false;

    if (samplerConfig->samplerOptions->hasIntervalTrigger)
    {
        startDataCollection = previousMillis == 0 || currentMillis - previousMillis >= samplerConfig->samplerOptions->intervalMsTrigger;
    }
    else if (samplerConfig->samplerOptions->hasMovementTrigger)
    {
        startDataCollection = hasNewMovement();
    }
    else if (samplerConfig->samplerOptions->hasAccRawTrigger)
    {
        accelerometer->sampleAccelerometer(false);

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
    {
        delay(100);
        return;
    }

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
    barometer->samplePressure();
    barometer->sampleTemperature();

    if (samplerConfig->samplerOptions->hasMicSensor)
    {
        // If it has mic trigger then it's already started
        if (!samplerConfig->samplerOptions->hasMicTrigger)
        {
            // Audio sampling is asynchronous, so it won't block sampleFrequencies
            microphone->startAudioSampling();
        }

        // If it has mic but no acc sensor, then wait for the mic to sample the expected number of samples as it would be done in sampleFrequencies() otherwise
        if (!samplerConfig->samplerOptions->hasAccSensor)
        {
            while (static_cast<unsigned long>(samplerConfig->micOptions->micSamplingLengthMs) < (currentMillis - previousMillis))
            {
                if (samplerConfig->samplerOptions->hasMicSensor)
                {
                    microphone->bufferCallback();
                }

                currentMillis = millis();
            }
        }
    }

    // Sample acc data at the same time as audio sampling
    sampleFrequencies();

    if (samplerConfig->samplerOptions->hasMicSensor)
    {
        // Stop audio sampling
        microphone->stopAudioSampling();
    }

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

            copyFromSampleDataPoint(&sampleDataPoints[i]);
            resetSampleDataPoint(sampleDataPoint);

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
            resetSampleDataPoint(&sampleDataPoints[i]);
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

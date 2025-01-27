#ifndef SAMPLER_H
#define SAMPLER_H

#include <Arduino.h>
#include <ArduinoJson.h>

#include "config.h"
#include "sample.h"
#include "accelerometer.h"
#include "barometer.h"
#include "microphone.h"

class Sampler
{
private:
    // The sampler configuration
    SamplerConfig *samplerConfig;
    // The sample data point
    SampleDataPoint *sampleDataPoint;
    // The sample data point buffer
    SampleDataPoint *sampleDataPoints;

    // Accelerometer instance
    Accelerometer *accelerometer;
    // Barometer instance
    Barometer *barometer;
    // Microphone instance
    Microphone *microphone;

    // Time interval for data collection
    // @deprecated once it's changed to be based on events
    unsigned long previousMillis; // Store the last time the data collection event occurred
    unsigned long currentMillis;

    // Used to measure the time it takes to sample the data for frequency analysis
    unsigned long currentMicroseconds;

    // Temporarily stores the sample json to be saved to file
    JsonDocument jsonDoc;

    /**
     * Sample the accelerometer data as well as the audio data
     */
    void sampleFrequencies();

    /**
     * Copy the sample data point to the buffer
     * @param destinationSampleDataPoint The "new" sample data point to copy from the current sample data point
     */
    void copyFromSampleDataPoint(SampleDataPoint *destinationSampleDataPoint);

    /**
     * Reset the sample data point
     * @param targetSampleDataPoint The sample data point to reset
     */
    void resetSampleDataPoint(SampleDataPoint *targetSampleDataPoint);

    /**
     * Save the samples to file when sd card is available
     */
    void saveSamplesToFile();

    /**
     * Sample the data when the triggers are met
     */
    void sampleData();

    /**
     * Detect vertical movement based on the barometer and acc data
     */
    bool hasNewMovement();

public:
    /**
     * @param _options The sampler options
     */
    Sampler(SamplerConfig *_samplerConfig);

    /**
     * Check the triggers to start data collection
     */
    void checkTriggers();
};

#endif // SAMPLER_H

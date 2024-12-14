#ifndef SAMPLER_H
#define SAMPLER_H

#include <Arduino.h>
#include <ArduinoJson.h>

#include "config.h"
#include "options.h"
#include "sample.h"
#include "accelerometer.h"
#include "barometer.h"
#include "microphone.h"

class Sampler
{
private:
    // The sampler configuration
    SamplerConfig *samplerConfig;

    // The sample data point reference
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
    bool isTimeForDataCollection;

    // Used to measure the time it takes to sample the data for frequency analysis
    unsigned long currentMicroseconds;

    // Temporarily stores the sample json to be saved to file
    JsonDocument jsonDoc;

    // Sample acc data
    void sampleFrequencies();

    // Copy the sample data point to the buffer
    void copySample(SampleDataPoint *newSample);

    // Reset the sample data point
    void resetSample(SampleDataPoint *_sample);

    // Save the samples to file
    void saveSamplesToFile();

public:
    /**
     * @param _options The sampler options
     */
    Sampler(SamplerConfig *_samplerConfig);

    void checkTriggers();
    void sampleData();
};

#endif // SAMPLER_H

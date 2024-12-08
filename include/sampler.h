#ifndef SAMPLER_H
#define SAMPLER_H

#include <Arduino.h>
#include <ArduinoJson.h>

#include "options.h"
#include "sample.h"
#include "accelerometer.h"
#include "barometer.h"

class Sampler
{
private:
    SamplerOptions *options;

    // The sample data point reference
    SampleDataPoint *sample;
    // The sample data point buffer
    SampleDataPoint *samples;

    // Accelerometer instance
    Accelerometer *accelerometer;
    // Barometer instance
    Barometer *barometer;

    // Time interval for data collection
    unsigned long previousMillis; // Store the last time the data collection event occurred
    unsigned long currentMillis;
    bool isTimeForDataCollection;

    // Temporarily stores the sample json to be saved to file
    JsonDocument jsonDoc;

    void saveSamplesToFile();

    void copySample(SampleDataPoint *newSample);

    void resetSample(SampleDataPoint *_sample);

public:
    /**
     * @param _options The sampler options
     */
    Sampler(SamplerOptions *_options);

    void sampleData();
};

#endif // SAMPLER_H

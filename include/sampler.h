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

    // Accelerometer # of samples
    int16_t accNumSamples;
    // The sample data point reference
    SampleDataPoint *sample;
    // The sample data point buffer
    SampleDataPoint *samples;

    const int16_t samplesBufferSize; // Number of complete samples to be saved before writing to file
    JsonDocument jsonDoc;

    // Accelerometer instance
    Accelerometer *accelerometer;

    // Barometer instance
    Barometer *barometer;

    void saveSamplesToFile();

    void copySample(SampleDataPoint *newSample);

    void resetSample();

public:
    /**
     * @param _accSamples Number of samples to be collected - must be a power of 2
     * @param _samplingFrequency Max sampling frequency in Hz
     * @param _samplesBufferSize Number of complete samples to be saved before writing to file
     */
    Sampler(SamplerOptions *_options);

    void sampleData();
};

#endif // SAMPLER_H

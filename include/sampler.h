#ifndef SAMPLER_H
#define SAMPLER_H

#include <Arduino.h>
#include <ArduinoJson.h>

#include "sample.h"
#include "accelerometer.h"
#include "barometer.h"

class Sampler
{
private:
    // Accelerometer samples
    int16_t accSamples;
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

    void saveSamplesToFile(bool printResults = true);

    void copySample(SampleDataPoint *newSample);

    void resetSample();

public:
    /**
     * @param _accSamples Number of samples to be collected - must be a power of 2
     * @param _samplingFrequency Max sampling frequency in Hz
     * @param _samplesBufferSize Number of complete samples to be saved before writing to file
     */
    Sampler(int16_t _accSamples, int16_t _samplingFrequency = 512, int16_t _samplesBufferSize = 10);

    void sampleData(bool printResults = true);
};

#endif // SAMPLER_H

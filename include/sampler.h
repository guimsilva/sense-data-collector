#ifndef SAMPLER_H
#define SAMPLER_H

#include <Arduino.h>
#include <ArduinoJson.h>
// #include <SPI.h>
// #include <SD.h>
#include "sample.h"
#include "vibration.h"
#include "barometer.h"

class Sampler
{
private:
    // Vibration samples
    int16_t vibrationSamples;
    // The sample data point reference
    SampleDataPoint *sample;
    // The sample data point buffer
    SampleDataPoint *samples;

    const int16_t samplesBufferSize; // Number of complete samples to be saved before writing to file
    JsonDocument jsonDoc;

    // Vibration instance
    Vibration *vibration;

    // Barometer instance
    Barometer *barometer;

    void saveSamplesToFile(bool printResults = true);

public:
    Sampler(int16_t _vibrationSamples = 512, int16_t _samplesBufferSize = 10);

    void sampleData(bool printResults = true);
};

#endif // SAMPLER_H

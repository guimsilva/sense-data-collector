#include <Arduino.h>
#include <ArduinoJson.h>
#include "sampler.h"

void Sampler::sampleData(bool printResults = true)
{
    Serial.println("Collecting vibration data...");

    vibration->sampleVibration(printResults);

    Serial.print("Adding sample to buffer. Buffer size: ");
    Serial.println(samplesBufferSize);

    // Buffer should always have space for a new sample because it's reset when it's full, right below
    for (int i = 0; i < samplesBufferSize; i++)
    {
        if (samples[i].timestamp == 0)
        {
            samples[i] = *sample;
            Serial.print("Sample added at index: ");
            Serial.println(i);
            break;
        }
    }

    // Log data and reset the buffer when it's full
    if (samples[samplesBufferSize - 1].timestamp != 0)
    {
        Serial.println("Buffer full. Saving to file and resetting buffer");
        saveSamplesToFile(printResults);
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

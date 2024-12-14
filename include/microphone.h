#ifndef MICROPHONE_H
#define MICROPHONE_H

#include "config.h"
#include "sample.h"

class Microphone
{
private:
    // The sample data point reference
    SampleDataPoint *sampleDataPoint;

    SamplerConfig *samplerConfig;

    static const int16_t tempBufferSize = 256; // Temporary buffer size
    int16_t *tempAudioBuffer;                  // Temporary buffer
    int sampleIndex = 0;                       // The current sample index

public:
    /**
     * @param _sampleDataPoint The sample data point reference
     * @param _samplerOptions The sampler options
     */
    Microphone(SampleDataPoint *_sampleDataPoint, SamplerConfig *_samplerConfig);

    void startAudioSampling();

    void bufferCallback();

    void stopAudioSampling();

    bool isTriggered();
};

#endif // MICROPHONE_H

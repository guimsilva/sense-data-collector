#ifndef MICROPHONE_H
#define MICROPHONE_H

#include "options.h"
#include "sample.h"

class Microphone
{
private:
    SamplerOptions *samplerOptions;
    // The sample data point reference
    SampleDataPoint *sampleDataPoint;

    static const int16_t tempBufferSize = 256; // Temporary buffer size
    int16_t tempAudioBuffer[tempBufferSize];   // Temporary buffer
    int16_t *audioBuffer;                      // The full audio buffer
    int sampleIndex = 0;                       // The current sample index
    int totalSamples;                          // The total number of samples to be collected

    void copyAudioBuffer();
    void resetAudioBuffer();

public:
    /**
     * @param _sampleDataPoint The sample data point reference
     * @param _samplerOptions The sampler options
     */
    Microphone(SampleDataPoint *_sampleDataPoint, SamplerOptions *_samplerOptions);

    void startAudioSampling();

    void bufferCallback();

    void stopAudioSampling();
};

#endif // MICROPHONE_H

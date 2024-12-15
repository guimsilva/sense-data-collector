#include <Arduino.h>

#include "microphone.h"
#include "PDM.h"

namespace microphone
{
    // Create static buffer for PDM samples
    int16_t *localTempAudioBuffer;
    // Whether new data is available
    volatile bool hasNewData = false;

    void onPDMdataCallback()
    {
        int bytesAvailable = PDM.available();
        PDM.read(localTempAudioBuffer, bytesAvailable);
        hasNewData = true;
    }
} // namespace

Microphone::Microphone(SampleDataPoint *_sampleDataPoint, SamplerConfig *_samplerConfig)
    : sampleDataPoint(_sampleDataPoint),
      samplerConfig(_samplerConfig),
      tempAudioBuffer(new int16_t[tempBufferSize])
{
    if (samplerConfig->samplerOptions->logLevel >= LogLevel::Info)
    {
        Serial.println("Initializing microphone");
    }

    // Calculate the total number of samples to be collected based on the accSamplingLengthMs so the mic will record for as long as the acc data is collected

    if (samplerConfig->accOptions->accSamplingLengthMs == 0)
    {
        Serial.println("accSamplingLengthMs is 0");
        while (1)
            ;
    }

    if (samplerConfig->samplerOptions->hasAccSensor)
    {
        samplerConfig->micOptions->micNumSamples = round(static_cast<double>(samplerConfig->micOptions->micSamplingRate * samplerConfig->accOptions->accSamplingLengthMs) / 1000);
    }
    else
    {
        samplerConfig->micOptions->micNumSamples = round(static_cast<double>(samplerConfig->micOptions->micSamplingRate * samplerConfig->micOptions->micSamplingLengthMs) / 1000);
    }
    // Re-initialize the sampleDataPoint audio buffer here because micNumSamples is now known
    sampleDataPoint->audioBuffer = new int16_t[samplerConfig->micOptions->micNumSamples];

    // Set the static buffer to the local buffer so the static callback can access it
    microphone::localTempAudioBuffer = tempAudioBuffer;

    PDM.onReceive(microphone::onPDMdataCallback);

    if (samplerConfig->samplerOptions->logLevel >= LogLevel::Info)
    {
        Serial.println("Microphone initialized");
        Serial.println();
    }
}

void Microphone::startAudioSampling()
{
    if (!samplerConfig->samplerOptions->hasMicTrigger)
        return;

    if (samplerConfig->samplerOptions->logLevel >= LogLevel::Info)
    {
        Serial.println("Sampling audio from microphone...");
    }

    if (samplerConfig->samplerOptions->hasIntervalTrigger)
    {
        if (!PDM.begin(1, samplerConfig->micOptions->micSamplingRate))
        {
            Serial.println("Failed to start PDM!");
            while (1)
                ;
        }
    }
    else
    {
        sampleIndex = 0;
        for (int i = 0; i < samplerConfig->micOptions->micNumSamples; i++)
        {
            sampleDataPoint->audioBuffer[i] = 0;
        }
        for (int i = 0; i < tempBufferSize; i++)
        {
            tempAudioBuffer[i] = 0;
        }
    }
}

void Microphone::stopAudioSampling()
{
    // Call bufferCallback one last time to get the remaining samples before stopping PDM
    bufferCallback();

    if (!samplerConfig->samplerOptions->hasMicTrigger)
    {
        PDM.end();
    }

    if (samplerConfig->samplerOptions->logLevel >= LogLevel::Info)
    {
        Serial.println("Audio from microphone sampled\n");
    }
}

/**
 * Used for both Interval and Microphone triggers.
 */
void Microphone::bufferCallback()
{
    if (!microphone::hasNewData)
        return;

    microphone::hasNewData = false;

    for (int i = 0; i < tempBufferSize && sampleIndex < samplerConfig->micOptions->micNumSamples; i++)
    {
        sampleDataPoint->audioBuffer[sampleIndex++] = tempAudioBuffer[i];
    }
}

/**
 * Used only with the Microphone trigger.
 * Microphone has a particular way to check for triggers.
 * Check if the microphone has new data and if the audioBufferSizeTrigger is equal or greater than tempAudioBuffer
 *
 * @todo Add more mic trigger conditions, such as min decibels
 */
bool Microphone::isTriggered()
{
    return microphone::hasNewData &&
           static_cast<unsigned int>(samplerConfig->samplerOptions->audioBufferSizeTrigger) >= (sizeof(tempAudioBuffer) / sizeof(tempAudioBuffer[0]));
}

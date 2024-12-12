#include <Arduino.h>

#include "options.h"
#include "config.h"
#include "microphone.h"
#include "PDM.h"

namespace microphone
{
    // Create static buffer for PDM samples
    int16_t *localTempAudioBuffer;
    // Whether new data is available
    volatile bool hasNewData = false;
    // Log level
    LogLevel logLevel = LogLevel::Info;

    void onPDMdataCallback()
    {
        if (logLevel >= LogLevel::Verbose)
        {
            Serial.print("PDM data callback. Bytes available: ");
        }

        int bytesAvailable = PDM.available();
        PDM.read(localTempAudioBuffer, bytesAvailable);

        if (logLevel >= LogLevel::Verbose)
        {
            Serial.println(bytesAvailable);
        }

        hasNewData = true;

        if (logLevel >= LogLevel::Verbose)
        {
            Serial.println("PDM data read");
        }
    }
} // namespace

Microphone::Microphone(SampleDataPoint *_sampleDataPoint, SamplerOptions *_samplerOptions)
    : sampleDataPoint(_sampleDataPoint),
      samplerOptions(_samplerOptions),
      tempAudioBuffer(new int16_t[tempBufferSize])
{
    if (samplerOptions->logLevel >= LogLevel::Info)
    {
        Serial.println("Initializing microphone");
    }

    // Calculate the total number of samples to be collected based on the accSamplingLengthMs so the mic will record for as long as the acc data is collected

    if (samplerOptions->accSamplingLengthMs == 0)
    {
        Serial.println("accSamplingLengthMs is 0");
        while (1)
            ;
    }

    samplerOptions->micNumSamples = round(static_cast<double>(samplerOptions->micSamplingRate * samplerOptions->accSamplingLengthMs) / 1000);

    if (samplerOptions->logLevel >= LogLevel::Info)
    {
        Serial.print("Total audio samples: ");
        Serial.println(samplerOptions->micNumSamples);
    }

    // Re-initialize the sampleDataPoint audio buffer here because micNumSamples is now known
    sampleDataPoint->audioBuffer = new int16_t[samplerOptions->micNumSamples];
    // Set the static buffer to the local buffer so the static callback can access it
    microphone::localTempAudioBuffer = tempAudioBuffer;
    microphone::logLevel = samplerOptions->logLevel;

    PDM.onReceive(microphone::onPDMdataCallback);

    if (samplerOptions->logLevel >= LogLevel::Info)
    {
        Serial.println("Microphone initialized");
        Serial.println();
    }
}

void Microphone::startAudioSampling()
{
    if (samplerOptions->logLevel >= LogLevel::Info)
    {
        Serial.println("Sampling audio from microphone...");
    }

    if (!PDM.begin(1, samplerOptions->micSamplingRate))
    {
        Serial.println("Failed to start PDM!");
        while (1)
            ;
    }
}

void Microphone::stopAudioSampling()
{
    // Call bufferCallback one last time to get the remaining samples before stopping PDM
    bufferCallback();
    PDM.end();

    if (samplerOptions->logLevel >= LogLevel::Info)
    {
        Serial.println("Audio from microphone sampled\n");
    }
}

void Microphone::bufferCallback()
{
    if (!microphone::hasNewData)
        return;

    microphone::hasNewData = false;
    for (int i = 0; i < tempBufferSize && sampleIndex < samplerOptions->micNumSamples; i++)
    {
        sampleDataPoint->audioBuffer[sampleIndex++] = tempAudioBuffer[i];
    }
}

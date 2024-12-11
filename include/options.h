#ifndef OPTIONS_H
#define OPTIONS_H

#include <Arduino.h>

enum class LogLevel
{
    None,
    Info,
    Verbose,
};

struct SamplerOptions
{
    /**
     * @param _intervalInMillis Interval at which allow data collection (milliseconds). Default is 2000
     * @param _accNumSamples Number of samples to be collected - must be a power of 2. Default is 256
     * @param _accSamplingFrequency Max acc sampling frequency in Hz. If left default 0, it will get the max sampling frequency from the IMU
     * @param _micSamplingRate Audio sampling frequency in Hz. Default is 16000
     * @param _samplesBufferSize Number of whole sample data points to be collected before saving to file. Default is 10
     * @param _logLevel The log level. Default is Info
     * @param _saveToSdCard Whether to save the data to the SD card. Default is true
     */
    SamplerOptions(
        unsigned long _intervalInMillis = 2000,
        int16_t _accNumSamples = 256,
        int16_t _accSamplingFrequency = 0,
        int16_t _micSamplingRate = 16000,
        int16_t _sampleDataPointBufferSize = 10,
        LogLevel _logLevel = LogLevel::Info,
        bool _saveToSdCard = true)
        : intervalInMillis(_intervalInMillis),
          accNumSamples(_accNumSamples),
          accSamplingFrequency(_accSamplingFrequency),
          micSamplingRate(_micSamplingRate),
          sampleDataPointBufferSize(_sampleDataPointBufferSize),
          logLevel(_logLevel),
          saveToSdCard(_saveToSdCard)
    {
        accSamplingLengthMs = 0; // Will be reset in the acc constructor
    }

    // Interval at which allow data collection (milliseconds)
    const unsigned long intervalInMillis;

    int accSamplingLengthMs;      // Calculated in acc constructor. e.g. x = 256 samples and sampling frequency y = 100 will result in ~2560 milliseconds of sampling (x / y * 1000 = millisecs)
    int16_t accNumSamples;        // Must be a power of 2
    int16_t accSamplingFrequency; // Hz. Determines maximum frequency

    int16_t micSamplingRate; // Hz. Determines audio sampling frequency

    int16_t sampleDataPointBufferSize; // Number of whole sample data points to be collected before saving to file
    bool saveToSdCard;

    LogLevel logLevel;
};

#endif // OPTIONS_H

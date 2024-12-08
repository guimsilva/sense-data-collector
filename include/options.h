#ifndef OPTIONS_H
#define OPTIONS_H
#include <Arduino.h>

#include "Arduino_BMI270_BMM150.h"

enum class LogLevel
{
    None,
    Info,
    Verbose,
};

struct SamplerOptions
{
    /**
     * @param _accSamples Number of samples to be collected - must be a power of 2
     * @param _samplingFrequency Max sampling frequency in Hz. If left default 0, it will get the max sampling frequency from the IMU
     */
    SamplerOptions(
        int16_t _accSamples = 512,
        int16_t _samplingFrequency = 0,
        int16_t _samplesBufferSize = 10,
        LogLevel _logLevel = LogLevel::Info,
        bool _saveToSdCard = true)
        : accNumSamples(_accSamples),
          samplingFrequency(_samplingFrequency),
          samplesBufferSize(_samplesBufferSize),
          logLevel(_logLevel),
          saveToSdCard(_saveToSdCard)
    {
        if (samplingFrequency == 0)
        {
            samplingFrequency = IMU.accelerationSampleRate();
        }
    }

    int16_t accNumSamples;
    int16_t samplingFrequency;
    int16_t samplesBufferSize;

    bool saveToSdCard = true;

    LogLevel logLevel = LogLevel::Info;
};

#endif // OPTIONS_H

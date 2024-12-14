#ifndef OPTIONS_H
#define OPTIONS_H

#include <Arduino.h>
#include <algorithm>

enum class Triggers
{
    Interval,
    /**
     * Uses accThreshold on acc X, Y and Z to trigger data collection
     */
    AccRaw,
    /**
     * Uses MovingStatus and MovingDirection to trigger data collection
     */
    AccMovement,
    Microphone,
};

enum class DataSensor
{
    Accelerometer,
    Microphone,
    Barometer,
};

enum class MovingStatus
{
    Stopped,
    Accelerating,
    Steady,
    Stopping,
};

enum class MovingDirection
{
    None,
    Up,
    Down
};

enum class LogLevel
{
    None,
    Info,
    Verbose,
};

struct AccOptions
{
    /**
     * Only attributes related to their respective sensors will be used.
     * @param _accNumSamples Number of samples to be collected - must be a power of 2. Default is 256
     * @param _accSamplingFrequency Max acc sampling frequency in Hz. If left default 0 then it will get the max sampling frequency from the IMU
     */
    AccOptions(
        int16_t _accNumSamples = 256,
        int16_t _accSamplingFrequency = 0)
        : accNumSamples(_accNumSamples),
          accSamplingFrequency(_accSamplingFrequency)
    {

        accSamplingLengthMs = 0; // Will be reset in the acc constructor
    }

    int16_t accNumSamples;        // Must be a power of 2
    int16_t accSamplingFrequency; // Hz. Determines maximum frequency

    // Internal i.e. not set by user
    int accSamplingLengthMs; // Calculated in acc constructor. e.g. x = 256 samples and sampling frequency y = 100 will result in ~2560 milliseconds of sampling (x / y * 1000 = millisecs)
};

struct MicOptions
{
    /**
     * @param _micSamplingRate Audio sampling frequency in Hz. Default is 16000
     */
    MicOptions(
        int16_t _micSamplingRate = 16000)
        : micSamplingRate(_micSamplingRate)
    {
    }

    int16_t micSamplingRate; // Hz. Determines audio sampling frequency

    // Internal i.e. not set by user
    int micNumSamples; // Calculated in the mic constructor e.g. micSamplingRate * accSamplingLengthMs / 1000
};

struct SamplerOptions
{
    /**
     * @param _triggers Triggers that can initiate data collection. Default is Interval
     * @param _dataSensors Supported sensors for data collection. Default is Accelerometer, Microphone, Barometer
     * @param _movementTriggers Movements that can trigger data collection. Default is Stopped, None
     * @param _accThresholdTrigger Raw acc threshold values to trigger data collection. Default is 100, 100, 100
     * @param _audioBufferSizeTrigger Min audio buffer size to trigger data collection. Default is 1000
     * @param _intervalInMillis Interval at which allow data collection (milliseconds). Default is 5000
     * @param _sampleDataPointBufferSize Number of whole sample data points to be collected before saving to file. Default is 10
     * @param _logLevel The log level. Default is Info
     * @param _saveToSdCard Whether to save the data to the SD card. Default is true
     */
    SamplerOptions(
        Triggers *_triggers = nullptr,
        DataSensor *_dataSensors = nullptr,
        std::tuple<MovingStatus, MovingDirection> *_movementTriggers = nullptr,
        int16_t *_accThresholdTrigger = nullptr,
        int16_t _audioBufferSizeTrigger = 0,
        unsigned long _intervalInMillis = 0,
        int16_t _sampleDataPointBufferSize = 10,
        LogLevel _logLevel = LogLevel::Info,
        bool _saveToSdCard = true)
        : sampleDataPointBufferSize(_sampleDataPointBufferSize),
          logLevel(_logLevel),
          saveToSdCard(_saveToSdCard)
    {
        if (_triggers == nullptr)
        {
            triggers = new Triggers[1];
            triggers[0] = Triggers::Interval;
        }
        else
        {
            triggers = _triggers;
        }

        if (_dataSensors == nullptr)
        {
            dataSensors = new DataSensor[3];
            dataSensors[0] = DataSensor::Accelerometer;
            dataSensors[1] = DataSensor::Microphone;
            dataSensors[2] = DataSensor::Barometer;
        }
        else
        {
            dataSensors = _dataSensors;
        }

        if (_intervalInMillis == 0 && std::find(triggers, triggers + 1, Triggers::Interval) != triggers + 1)
        {
            intervalMsTrigger = 5000;
        }
        else if (_intervalInMillis != 0)
        {
            intervalMsTrigger = _intervalInMillis;
        }

        if (_movementTriggers == nullptr && std::find(triggers, triggers + 1, Triggers::AccMovement) != triggers + 1)
        {
            movementTriggers = new std::tuple<MovingStatus, MovingDirection>[7];
            movementTriggers[0] = std::make_tuple(MovingStatus::Stopped, MovingDirection::None);
            movementTriggers[1] = std::make_tuple(MovingStatus::Accelerating, MovingDirection::Up);
            movementTriggers[2] = std::make_tuple(MovingStatus::Accelerating, MovingDirection::Down);
            movementTriggers[3] = std::make_tuple(MovingStatus::Steady, MovingDirection::Up);
            movementTriggers[4] = std::make_tuple(MovingStatus::Steady, MovingDirection::Down);
            movementTriggers[5] = std::make_tuple(MovingStatus::Stopping, MovingDirection::Up);
            movementTriggers[6] = std::make_tuple(MovingStatus::Stopping, MovingDirection::Down);
        }
        else
        {
            movementTriggers = _movementTriggers;
        }

        if (_accThresholdTrigger == nullptr && std::find(triggers, triggers + 1, Triggers::AccRaw) != triggers + 1)
        {
            accThresholdTrigger = new int16_t[3];
            accThresholdTrigger[0] = 100;
            accThresholdTrigger[1] = 100;
            accThresholdTrigger[2] = 100;
        }
        else
        {
            accThresholdTrigger = _accThresholdTrigger;
        }

        if (_audioBufferSizeTrigger == 0 && std::find(triggers, triggers + 1, Triggers::Microphone) != triggers + 1)
        {
            audioBufferSizeTrigger = 1000;
        }
        else
        {
            audioBufferSizeTrigger = _audioBufferSizeTrigger;
        }
    }

    /**
     * Triggers that can initiate data collection
     */
    Triggers *triggers;

    /**
     * Supported sensors for data collection
     */
    DataSensor *dataSensors;

    /**
     * Interval at which allow data collection (milliseconds)
     */
    unsigned long intervalMsTrigger;

    /**
     * Raw acc threshold values to trigger data collection. One for each axis (X, Y, Z)
     */
    int16_t *accThresholdTrigger;

    /**
     * Movements that can trigger data collection
     */
    std::tuple<MovingStatus, MovingDirection> *movementTriggers;

    /**
     * Min audio buffer size to trigger data collection.
     * The first flag that will be checked is the hasAudio flag, then it will check the audioBufferSizeTrigger.
     */
    int16_t audioBufferSizeTrigger;

    int16_t sampleDataPointBufferSize; // Number of whole sample data points to be collected before saving to file
    bool saveToSdCard;
    LogLevel logLevel;
};

#endif // OPTIONS_H

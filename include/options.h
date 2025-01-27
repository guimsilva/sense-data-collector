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
    Movement,
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

struct MovingTrigger
{
    MovingTrigger(MovingStatus _movingStatus = MovingStatus::Stopped, MovingDirection _movingDirection = MovingDirection::None)
        : movingStatus(_movingStatus),
          movingDirection(_movingDirection)
    {
    }
    MovingStatus movingStatus;
    MovingDirection movingDirection;
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
        int16_t _micSamplingRate = 16000,
        int16_t _micSamplingLengthMs = 2000)
        : micSamplingRate(_micSamplingRate),
          micSamplingLengthMs(_micSamplingLengthMs)
    {
    }

    int16_t micSamplingRate;     // Hz. Determines audio sampling frequency
    int16_t micSamplingLengthMs; // Used when Acc sampling is not set

    // Internal i.e. not set by user
    int micNumSamples; // Calculated in the mic constructor e.g. micSamplingRate * accSamplingLengthMs / 1000
};

struct SamplerOptions
{
    /**
     * @param _triggers Triggers that can initiate data collection. Default is Interval
     * @param _sizeofTriggers Size of the triggers array. Default is 1
     * @param _dataSensors Supported sensors for data collection. Default is Accelerometer, Microphone, Barometer
     * @param _sizeofDataSensors Size of the dataSensors array. Default is 3
     * @param _movementTriggers Movements that can trigger data collection. Default is Stopped, None
     * @param _sizeofMovementTriggers Size of the movementTriggers array. Default is 0 for no movement trigger or 7 (all possible movements currently) when there's one
     * @param _accThresholdTrigger Raw acc threshold values to trigger data collection. Default is 100, 100, 100 when there's an accRaw trigger or 0, 0, 0 when there's none
     * @param _audioBufferSizeTrigger Min audio buffer size to trigger data collection. Default is 1000
     * @param _intervalInMillis Interval at which allow data collection (milliseconds). Default is 5000
     * @param _sampleDataPointBufferSize Number of whole sample data points to be collected before saving to file. Default is 10
     * @param _logLevel The log level. Default is Info
     * @param _saveToSdCard Whether to save the data to the SD card. Default is true
     */
    SamplerOptions(
        bool _saveToSdCard = true,
        LogLevel _logLevel = LogLevel::Info,
        int16_t _sampleDataPointBufferSize = 10,
        unsigned long _intervalInMillis = 0,
        Triggers *_triggers = nullptr,
        unsigned short _sizeofTriggers = 1,
        DataSensor *_dataSensors = nullptr,
        unsigned short _sizeofDataSensors = 3,
        MovingTrigger *_movementTriggers = nullptr,
        unsigned short _sizeofMovementTriggers = 0,
        int16_t _accThresholdTrigger[3] = nullptr,
        int16_t _audioBufferSizeTrigger = 0)
        : saveToSdCard(_saveToSdCard),
          logLevel(_logLevel),
          sampleDataPointBufferSize(_sampleDataPointBufferSize)

    {
        if (_triggers == nullptr)
        {
            if (logLevel >= LogLevel::Info)
                Serial.println("Setting triggers to Interval only");

            triggers = new Triggers[1];
            triggers[0] = Triggers::Interval;
            sizeofTriggers = 1;
        }
        else
        {
            if (logLevel >= LogLevel::Info)
                Serial.println("Setting triggers to user defined");

            triggers = _triggers;
            sizeofTriggers = _sizeofTriggers;
        }

        if (_dataSensors == nullptr)
        {
            if (logLevel >= LogLevel::Info)
                Serial.println("Setting dataSensors to \"all sensors\"");

            dataSensors = new DataSensor[3];
            dataSensors[0] = DataSensor::Accelerometer;
            dataSensors[1] = DataSensor::Microphone;
            dataSensors[2] = DataSensor::Barometer;
            sizeofDataSensors = 3;
        }
        else
        {
            if (logLevel >= LogLevel::Info)
                Serial.println("Setting dataSensors to user defined");

            dataSensors = _dataSensors;
            sizeofDataSensors = _sizeofDataSensors;
        }

        if (_intervalInMillis == 0 && std::find(triggers, triggers + 1, Triggers::Interval) != triggers + 1)
        {
            if (logLevel >= LogLevel::Info)
                Serial.println("Setting intervalMsTrigger to 5000");

            intervalMsTrigger = 5000;
        }
        else if (_intervalInMillis != 0)
        {
            if (logLevel >= LogLevel::Info)
            {
                Serial.print("Setting intervalMsTrigger to ");
                Serial.println(_intervalInMillis);
            }
            intervalMsTrigger = _intervalInMillis;
        }

        if (_movementTriggers == nullptr)
        {
            if (logLevel >= LogLevel::Info)
                Serial.println("Setting movementTriggers to default");

            if (std::find(triggers, triggers + 1, Triggers::Movement) != triggers + 1)
            {
                movementTriggers = new MovingTrigger[7];
                movementTriggers[0] = MovingTrigger(MovingStatus::Stopped, MovingDirection::None);
                movementTriggers[1] = MovingTrigger(MovingStatus::Accelerating, MovingDirection::Up);
                movementTriggers[2] = MovingTrigger(MovingStatus::Accelerating, MovingDirection::Down);
                movementTriggers[3] = MovingTrigger(MovingStatus::Steady, MovingDirection::Up);
                movementTriggers[4] = MovingTrigger(MovingStatus::Steady, MovingDirection::Down);
                movementTriggers[5] = MovingTrigger(MovingStatus::Stopping, MovingDirection::Up);
                movementTriggers[6] = MovingTrigger(MovingStatus::Stopping, MovingDirection::Down);
                sizeofMovementTriggers = 7;
            }
            else
            {
                movementTriggers = _movementTriggers;
                sizeofMovementTriggers = _sizeofMovementTriggers;
            }
        }
        else
        {
            if (logLevel >= LogLevel::Info)
                Serial.println("Setting movementTriggers to user defined");

            movementTriggers = _movementTriggers;
            sizeofMovementTriggers = _sizeofMovementTriggers;
        }

        if (_accThresholdTrigger == nullptr)
        {
            if (logLevel >= LogLevel::Info)
                Serial.println("Setting accThresholdTrigger to default");

            if (std::find(triggers, triggers + 1, Triggers::AccRaw) != triggers + 1)
            {
                accThresholdTrigger[0] = 100;
                accThresholdTrigger[1] = 100;
                accThresholdTrigger[2] = 100;
            }
        }
        else
        {
            if (logLevel >= LogLevel::Info)
                Serial.println("Setting accThresholdTrigger to user defined");

            accThresholdTrigger[0] = _accThresholdTrigger[0];
            accThresholdTrigger[1] = _accThresholdTrigger[1];
            accThresholdTrigger[2] = _accThresholdTrigger[2];
        }

        if (_audioBufferSizeTrigger == 0 && std::find(triggers, triggers + 1, Triggers::Microphone) != triggers + 1)
        {
            if (logLevel >= LogLevel::Info)
                Serial.println("Setting audioBufferSizeTrigger to 1000");

            audioBufferSizeTrigger = 1000;
        }
        else
        {
            if (logLevel >= LogLevel::Info)
            {
                Serial.println("Setting audioBufferSizeTrigger to default");
            }
            audioBufferSizeTrigger = _audioBufferSizeTrigger;
        }
    }

    /**
     * Triggers that can initiate data collection
     */
    Triggers *triggers;
    unsigned short sizeofTriggers;

    /**
     * Supported sensors for data collection
     */
    DataSensor *dataSensors;
    unsigned short sizeofDataSensors;

    /**
     * Interval at which allow data collection (milliseconds)
     */
    unsigned long intervalMsTrigger;

    /**
     * Raw acc threshold values to trigger data collection. One for each axis (X, Y, Z)
     */
    int16_t accThresholdTrigger[3];

    /**
     * Movements that can trigger data collection
     */
    MovingTrigger *movementTriggers;
    unsigned short sizeofMovementTriggers;

    /**
     * Min audio buffer size to trigger data collection.
     * The first flag that will be checked is the mic hasNewData flag, then it will check this trigger with the tempAudioBuffer size.
     *
     * @todo Add more mic trigger conditions, such as min decibels
     */
    int16_t audioBufferSizeTrigger;

    int16_t sampleDataPointBufferSize; // Number of whole sample data points to be collected before saving to file
    bool saveToSdCard;
    LogLevel logLevel;

    // Internal - convert triggers into boolean values for faster checking
    bool hasIntervalTrigger = false;
    bool hasAccRawTrigger = false;
    bool hasMovementTrigger = false;
    bool hasMicTrigger = false;

    // Convert sensor data into boolean values for faster checking
    bool hasAccSensor = false;
    bool hasMicSensor = false;
    bool hasBarSensor = false;
};

#endif // OPTIONS_H

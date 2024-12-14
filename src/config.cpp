#include <Arduino.h>
#include <tuple>
#include <algorithm>

#include "config.h"
#include "options.h"

SamplerConfig::SamplerConfig(
    SamplerOptions *_samplerOptions,
    Triggers *_triggers,
    DataSensor *_dataSensors,
    unsigned long _intervalInMillis,
    std::tuple<MovingStatus, MovingDirection> *_movementTriggers,
    int16_t *_accThreshold,
    int16_t _audioBufferSizeTrigger)
    : samplerOptions(_samplerOptions)
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

    if (_accThreshold == nullptr && std::find(triggers, triggers + 1, Triggers::AccRaw) != triggers + 1)
    {
        accThresholdTrigger = new int16_t[3];
        accThresholdTrigger[0] = 100;
        accThresholdTrigger[1] = 100;
        accThresholdTrigger[2] = 100;
    }
    else
    {
        accThresholdTrigger = _accThreshold;
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

/**
 * Customize MovingStatus and MovingDirection as per your requirements.
 * Create additional enums and triggers for other sensors data if needed,
 * e.g. TiltingStatus, TiltingAngle, gyroscopeTrigger, temperatureTrigger etc.,
 * or even a combination of multiple sensors for a single trigger.
 * The Sample::checkTriggers() will also need to be updated to check for these new custom triggers.
 */

#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <tuple>
#include <algorithm>

#include "options.h"

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

class SamplerConfig
{
public:
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
    unsigned long intervalMsTrigger; // @TODO rename to intervalMsTrigger <<<<<

    /**
     * Movements that can trigger data collection
     */
    std::tuple<MovingStatus, MovingDirection> *movementTriggers;

    /**
     * Raw acc threshold values to trigger data collection. One for each axis (X, Y, Z)
     */
    int16_t *accThresholdTrigger;

    /**
     * Min audio buffer size to trigger data collection.
     * The first flag that will be checked is the hasAudio flag, then it will check the audioBufferSizeTrigger.
     */
    int16_t audioBufferSizeTrigger;

    /**
     * Additional parameters, complimenting the SamplerConfig.
     * More granular control over the sampler.
     */
    SamplerOptions *samplerOptions;

    /**
     * Basic configuration for the sampler triggers and sensors to be used.
     * Defaults are:
     * - Triggers: Interval
     * - Data sensors: Accelerometer, Microphone, Barometer
     * - Interval: 5000 milliseconds
     * The rest will be ignored if their respective triggers are not set.
     */
    SamplerConfig(
        SamplerOptions *_samplerOptions,
        Triggers *_triggers = nullptr,
        DataSensor *_dataSensors = nullptr,
        unsigned long _intervalInMillis = 0,
        std::tuple<MovingStatus, MovingDirection> *_movementTriggers = nullptr,
        int16_t *_accThreshold = nullptr,
        int16_t _audioBufferSizeTrigger = 0);
};

#endif // CONFIG_H

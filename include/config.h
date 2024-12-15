/**
 * Customize MovingStatus and MovingDirection as per your requirements.
 * Create additional enums and triggers for other sensors data if needed,
 * e.g. TiltingStatus, TiltingAngle, gyroscopeTrigger, temperatureTrigger etc.,
 * or even a combination of multiple sensors for a single trigger.
 * The Sample::checkTriggers() will also need to be updated to check for these new custom triggers.
 */

#ifndef CONFIG_H
#define CONFIG_H

#include "options.h"

class SamplerConfig
{
public:
    SamplerOptions *samplerOptions;
    AccOptions *accOptions;
    MicOptions *micOptions;

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
        AccOptions *_accOptions,
        MicOptions *_micOptions)
        : samplerOptions(_samplerOptions),
          accOptions(_accOptions),
          micOptions(_micOptions)
    {
        for (unsigned int i = 0; i < sizeof(samplerOptions->triggers) / sizeof(samplerOptions->triggers[0]); i++)
        {
            if (samplerOptions->triggers[i] == Triggers::Interval)
            {
                samplerOptions->hasIntervalTrigger = true;
            }
            else if (samplerOptions->triggers[i] == Triggers::AccMovement)
            {
                samplerOptions->hasAccMovementTrigger = true;
            }
            else if (samplerOptions->triggers[i] == Triggers::AccRaw)
            {
                samplerOptions->hasAccRawTrigger = true;
            }
            else if (samplerOptions->triggers[i] == Triggers::Microphone)
            {
                samplerOptions->hasMicTrigger = true;
            }
        }

        // Check if triggers has Interval and other triggers at the same time, and if so, show a warning and stop
        if (samplerOptions->hasIntervalTrigger &&
            (samplerOptions->hasAccMovementTrigger || samplerOptions->hasAccRawTrigger || samplerOptions->hasMicTrigger))
        {
            Serial.println("Cannot have Interval and other triggers at the same time");
            while (1)
                ;
        }

        for (int i = 0; i < static_cast<int>(sizeof(samplerOptions->dataSensors) / sizeof(samplerOptions->dataSensors[0])); i++)
        {
            if (samplerOptions->dataSensors[i] == DataSensor::Accelerometer)
            {
                samplerOptions->hasAccSensor = true;
            }
            else if (samplerOptions->dataSensors[i] == DataSensor::Microphone)
            {
                samplerOptions->hasMicSensor = true;
            }
            else if (samplerOptions->dataSensors[i] == DataSensor::Barometer)
            {
                samplerOptions->hasBarSensor = true;
            }
        }
    }
};

#endif // CONFIG_H

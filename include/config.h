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
    }
};

#endif // CONFIG_H

#ifndef ACCELEROMETER_H
#define ACCELEROMETER_H

#include "config.h"

class Accelerometer
{
private:
    SamplerConfig *samplerConfig;

public:
    // IMU sensor
    float accX, accY, accZ;

    /**
     * Frequencies with the raw acceleration data
     **/
    double *vRealX, *vRealY, *vRealZ;

    // Sampling period in microseconds
    unsigned int samplingPeriodUs;

    /**
     * @param _samplerOptions The sampler options
     */
    Accelerometer(SamplerConfig *samplerConfig);
};

#endif // ACCELEROMETER_H

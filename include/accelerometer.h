#ifndef ACCELEROMETER_H
#define ACCELEROMETER_H

#include "options.h"

class Accelerometer
{
private:
    SamplerOptions *samplerOptions;

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
    Accelerometer(SamplerOptions *_samplerOptions);
};

#endif // ACCELEROMETER_H

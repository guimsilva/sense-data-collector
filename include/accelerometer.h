#ifndef ACCELEROMETER_H
#define ACCELEROMETER_H

#include <ArduinoJson.h>

#include "Arduino_BMI270_BMM150.h"
#include "options.h"
#include "sample.h"

class Accelerometer
{
private:
    SamplerOptions *options;
    // The sample data point reference
    SampleDataPoint *sample;

    // IMU sensor
    float accX, accY, accZ;

    // Sampling period and time
    unsigned int samplingPeriodUs;
    unsigned long microseconds;

    /**
     * Frequencies with the raw acceleration data
     **/
    double *vRealX, *vRealY, *vRealZ;

public:
    /**
     * @param _sample The sample data point reference
     * @param _options The sampler options
     */
    Accelerometer(SampleDataPoint *_sample, SamplerOptions *_options);

    /**
     * Sampling data from the accelerometer
     **/
    void sampleAcceleration();
};

#endif // ACCELEROMETER_H

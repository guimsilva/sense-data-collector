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

    // x = 512 samples and sampling frequency y = 512 will result in 1 second of sampling (x / y = sec)
    const int16_t nummSamples;       // Must be a power of 2
    const int16_t samplingFrequency; // Hz. Determines maximum frequency

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
     * @param _numSamples Number of samples to be collected - must be a power of 2
     * @param _samplingFrequency Max sampling frequency in Hz
     */
    Accelerometer(SampleDataPoint *_sample, SamplerOptions *_options);

    /**
     * Sampling data from the accelerometer
     **/
    void sampleAcceleration();
};

#endif // ACCELEROMETER_H

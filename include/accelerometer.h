#ifndef ACCELEROMETER_H
#define ACCELEROMETER_H

#include <ArduinoJson.h>

#include "Arduino_BMI270_BMM150.h"
#include "sample.h"

class Accelerometer
{
private:
    // The sample data point reference
    SampleDataPoint *sample;

    // x = 512 samples and sampling frequency y = 512 will result in 1 second of sampling (x / y = sec)
    const int16_t samples;           // Must be a power of 2
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
     * @param _samples Number of samples to be collected - must be a power of 2
     * @param _samplingFrequency Max sampling frequency in Hz
     */
    Accelerometer(SampleDataPoint *_sample, int16_t _samples = 512, int16_t _samplingFrequency = 512);

    /**
     * Sampling data from the accelerometer
     **/
    void sampleAcceleration(bool printResults = true);
};

#endif // ACCELEROMETER_H

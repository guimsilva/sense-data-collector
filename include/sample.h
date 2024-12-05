#ifndef SAMPLE_H
#define SAMPLE_H

#include <Arduino.h>

struct SampleDataPoint
{
    SampleDataPoint(int16_t vibrationSamples = 512)
        : dominantFrequency(0.0),
          frequencies(new double[vibrationSamples]),
          timestamp(0)
    {
        for (int i = 0; i < vibrationSamples; ++i)
        {
            frequencies[i] = 0.0;
        }
    }

    // IMU acceleration sensor data
    double dominantFrequency;
    double *frequencies;

    // Pressure sensor data
    double temperature;
    double pressure;
    double altitude;

    // Combined
    /**
     * variable for moving status:
     * 0 - stopped
     * 1 - steady moving up
     * 2 - steady moving down
     * 3 - breaking moving up
     * 4 - breaking moving down
     * 5 - accelerating moving up
     * 6 - accelerating moving down
     **/
    int8_t movingStatus;
    int8_t movingSpeed;

    unsigned long timestamp;
};

#endif // SAMPLE_H

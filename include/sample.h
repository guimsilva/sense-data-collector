#ifndef SAMPLE_H
#define SAMPLE_H

#include <Arduino.h>

struct SampleDataPoint
{
    SampleDataPoint(int16_t accSamples = 512)
        : accFrequenciesX(new double[accSamples]),
          accFequenciesY(new double[accSamples]),
          accFrequenciesZ(new double[accSamples]),
          audioBuffer(nullptr)
    {
        temperatureC = 0.0;
        pressureKpa = 0.0;
        altitudeMeters = 0.0;
        movingStatus = 0;
        movingSpeed = 0;
        timestamp = 0;

        for (int i = 0; i < accSamples; ++i)
        {
            accFrequenciesX[i] = 0.0;
            accFequenciesY[i] = 0.0;
            accFrequenciesZ[i] = 0.0;
        }
    }

    // Pressure sensor data
    double temperatureC;
    double pressureKpa;
    double altitudeMeters;

    // IMU acceleration sensor data
    double *accFrequenciesX;
    double *accFequenciesY;
    double *accFrequenciesZ;

    // Audio sensor data
    int16_t *audioBuffer;

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

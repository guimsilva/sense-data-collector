#ifndef SAMPLE_H
#define SAMPLE_H

#include <Arduino.h>

struct SampleDataPoint
{
    SampleDataPoint(int16_t accSamples = 512)
        : frequenciesX(new double[accSamples]),
          frequenciesY(new double[accSamples]),
          frequenciesZ(new double[accSamples]),
          timestamp(0)
    {
        for (int i = 0; i < accSamples; ++i)
        {
            frequenciesX[i] = 0.0;
            frequenciesY[i] = 0.0;
            frequenciesZ[i] = 0.0;
        }
    }

    // IMU acceleration sensor data
    double *frequenciesX;
    double *frequenciesY;
    double *frequenciesZ;

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

#ifndef SAMPLE_H
#define SAMPLE_H

#include <Arduino.h>

#include "config.h"

struct SampleDataPoint
{
    SampleDataPoint(int16_t accNumSamples = 256)
        : accFrequenciesX(new double[accNumSamples]),
          accFequenciesY(new double[accNumSamples]),
          accFrequenciesZ(new double[accNumSamples]),
          audioBuffer(nullptr)
    {
        temperatureC = 0.0;
        pressureKpa = 0.0;
        altitudeMeters = 0.0;
        movingStatus = MovingStatus::Stopped;
        movingDirection = MovingDirection::None;
        movingSpeed = 0;
        timestamp = 0;

        for (int i = 0; i < accNumSamples; ++i)
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

    MovingStatus movingStatus;
    MovingDirection movingDirection;
    int8_t movingSpeed;

    unsigned long timestamp;
};

#endif // SAMPLE_H

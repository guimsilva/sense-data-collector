#ifndef BAROMETER_H
#define BAROMETER_H

#include <Arduino.h>

#include "config.h"
#include "sample.h"

class Barometer
{
private:
    SamplerConfig *samplerConfig;
    SampleDataPoint *sampleDataPoint;

    float currentPressureKpa = 0.0f;
    float newPressure = 0.0f;
    float altitudeMeters = 0.0f;
    float temperatureC = 0.0f;

    bool isMoving = false;
    float movingSpeed = 0.0f;
    MovingStatus movingStatus = MovingStatus::Stopped;

    void getPressure();

    void getTemperature();

public:
    Barometer(SampleDataPoint *_sampleDataPoint, SamplerConfig *_samplerConfig);

    void samplePressure(bool logData = true);

    void sampleTemperature();

    float getAltitudeMeters() { return altitudeMeters; }
};

#endif // BAROMETER_H

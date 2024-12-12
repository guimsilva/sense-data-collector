#ifndef BAROMETER_H
#define BAROMETER_H

#include <Arduino.h>

#include "options.h"
#include "config.h"
#include "sample.h"

class Barometer
{
private:
    SamplerOptions *sampleOptions;
    // The sample data point reference
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

    /** @todo fix/improve this */
    void getMovingStatus();

public:
    Barometer(SampleDataPoint *_sampleDataPoint, SamplerOptions *_samplerOptions);

    void sampleBarometer();
};

#endif // BAROMETER_H

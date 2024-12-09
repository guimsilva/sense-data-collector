#ifndef BAROMETER_H
#define BAROMETER_H

#include <Arduino.h>
#include <Arduino_LPS22HB.h>

#include "options.h"
#include "sample.h"

class Barometer
{
private:
    SamplerOptions *options;
    // The sample data point reference
    SampleDataPoint *sample;

    float currentPressureKpa = 0.0f;
    float newPressure = 0.0f;
    float altitudeMeters = 0.0f;
    float temperatureC = 0.0f;

    bool isMoving = false;
    float movingSpeed = 0.0f;
    int8_t movingStatus = 0;

    void getPressure();

    void getTemperature();

    /** @todo fix/improve this */
    void getMovingStatus();

public:
    Barometer(SampleDataPoint *_sample, SamplerOptions *_options);

    void sampleBarometer();
};

#endif // BAROMETER_H

#ifndef BAROMETER_H
#define BAROMETER_H

#include <Arduino.h>
#include <Arduino_LPS22HB.h>

#include "sample.h"

class Barometer
{
private:
    // The sample data point reference
    SampleDataPoint *sample;

    float currentPressure = 0.0f;
    float newPressure = 0.0f;
    float altitude = 0.0f;
    float temperature = 0.0f;

    bool isMoving = false;
    float movingSpeed = 0.0f;
    int8_t movingStatus = 0;

    void getPressure();

    void getTemperature();

    /** @todo fix/improve this */
    void getMovingStatus();

public:
    Barometer(SampleDataPoint *_sample);

    void sampleBarometer(bool printResults = true);
};

#endif // BAROMETER_H

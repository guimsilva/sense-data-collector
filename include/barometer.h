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

    void getPressure()
    {
        newPressure = BARO.readPressure();
        if (newPressure != currentPressure)
        {
            currentPressure = newPressure;
            altitude = 44330 * (1 - pow(currentPressure / 101.325, 1 / 5.255));
        }

        Serial.print("Altitude according to kPa is = ");
        Serial.print(altitude);
        Serial.println(" m");
        Serial.println();
    }

    void getTemperature()
    {
        temperature = BARO.readTemperature();
        Serial.print("Temperature is = ");
        Serial.print(temperature);
        Serial.println(" C");
        Serial.println();
    }

    /** @todo fix/improve this */
    void getMovingStatus()
    {
        if (currentPressure != newPressure)
        {
            isMoving = true;
            movingSpeed = (currentPressure - newPressure) / 2;
        }
        else
        {
            isMoving = false;
            movingSpeed = 0.0f;
        }

        // This is probably not right
        movingStatus = isMoving ? (movingSpeed > 0 ? 1 : 2) : 0;

        Serial.print("Moving status: ");
        Serial.println(isMoving ? "Moving" : "Stopped");
        Serial.print("Moving speed: ");
        Serial.print(movingSpeed);
        Serial.println(" kPa/s");
        Serial.println();
    }

public:
    Barometer(SampleDataPoint *_sample)
    {
        sample = _sample;
    }

    void sampleBarometer(bool printResults = true);
};

#endif // BAROMETER_H

#include <Arduino.h>
#include <Arduino_LPS22HB.h>

#include "barometer.h"

void Barometer::sampleBarometer(bool printResults)
{
    if (printResults)
    {
        Serial.println("Sampling pressure data...");
    }

    getPressure();
    sample->pressure = currentPressure;
    sample->altitude = altitude;
    sample->movingStatus = movingStatus;
    sample->movingSpeed = movingSpeed;

    if (printResults)
    {
        Serial.println("Pressure data sampled");
    }

    if (printResults)
    {
        Serial.println("Sampling temperature data...");
    }

    getTemperature();
    sample->temperature = temperature;

    if (printResults)
    {
        Serial.println("Temperature data sampled");
    }
}

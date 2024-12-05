#include <Arduino.h>
#include <Arduino_LPS22HB.h>

#include "barometer.h"

Barometer::Barometer(SampleDataPoint *_sample)
{
    sample = _sample;
}

void Barometer::getPressure()
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

void Barometer::getTemperature()
{
    temperature = BARO.readTemperature();
    Serial.print("Temperature is = ");
    Serial.print(temperature);
    Serial.println(" C");
    Serial.println();
}

/** @todo fix/improve this */
void Barometer::getMovingStatus()
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

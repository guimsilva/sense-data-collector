#include <Arduino.h>
#include <Arduino_LPS22HB.h>

#include "options.h"
#include "barometer.h"

Barometer::Barometer(SampleDataPoint *_sampleDataPoint, SamplerOptions *_samplerOptions)
    : sampleOptions(_samplerOptions),
      sampleDataPoint(_sampleDataPoint)
{
    if (sampleOptions->logLevel >= LogLevel::Info)
    {
        Serial.println("Initializing barometer");
    }

    // Start pressure sensor
    if (!BARO.begin())
    {
        Serial.println("Failed to initialize pressure sensor!");
        while (1)
            ;
    }

    if (sampleOptions->logLevel >= LogLevel::Info)
    {
        Serial.println("Barometer initialized\n");
    }
}

void Barometer::getPressure()
{
    newPressure = BARO.readPressure();
    if (newPressure != currentPressureKpa)
    {
        currentPressureKpa = newPressure;
        altitudeMeters = 44330 * (1 - pow(currentPressureKpa / 101.325, 1 / 5.255));
    }

    Serial.print("Altitude according to kPa is = ");
    Serial.print(altitudeMeters);
    Serial.println(" m");
}

void Barometer::getTemperature()
{
    temperatureC = BARO.readTemperature();
    Serial.print("Temperature is = ");
    Serial.print(temperatureC);
    Serial.println(" C");
}

/** @todo fix/improve this */
void Barometer::getMovingStatus()
{
    if (currentPressureKpa != newPressure)
    {
        isMoving = true;
        movingSpeed = (currentPressureKpa - newPressure) / 2;
    }
    else
    {
        isMoving = false;
        movingSpeed = 0.0f;
    }

    // This is probably not right
    movingStatus = isMoving ? (movingSpeed > 0 ? 1 : 2) : 0;

    if (sampleOptions->logLevel >= LogLevel::Verbose)
    {
        Serial.print("Moving status: ");
        Serial.println(isMoving ? "Moving" : "Stopped");
        Serial.print("Moving speed: ");
        Serial.print(movingSpeed);
        Serial.println(" kPa/s");
    }
}

void Barometer::sampleBarometer()
{
    if (sampleOptions->logLevel >= LogLevel::Info)
    {
        Serial.println("Sampling pressure data...");
    }

    getPressure();
    sampleDataPoint->pressureKpa = currentPressureKpa;
    sampleDataPoint->altitudeMeters = altitudeMeters;
    sampleDataPoint->movingStatus = movingStatus;
    sampleDataPoint->movingSpeed = movingSpeed;

    if (sampleOptions->logLevel >= LogLevel::Info)
    {
        Serial.println("Pressure data sampled\n");
    }

    if (sampleOptions->logLevel >= LogLevel::Info)
    {
        Serial.println("Sampling temperature data...");
    }

    getTemperature();
    sampleDataPoint->temperatureC = temperatureC;

    if (sampleOptions->logLevel >= LogLevel::Info)
    {
        Serial.println("Temperature data sampled\n");
    }
}

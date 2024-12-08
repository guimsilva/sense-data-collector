#include <Arduino.h>
#include <Arduino_LPS22HB.h>

#include "options.h"
#include "barometer.h"

Barometer::Barometer(SampleDataPoint *_sample, SamplerOptions *_options)
    : options(_options),
      sample(_sample)
{
    // Start pressure sensor
    if (!BARO.begin())
    {
        Serial.println("Failed to initialize pressure sensor!");
        while (1)
            ;
    }

    if (options->logLevel >= LogLevel::Info)
    {
        Serial.println("Barometer initialized");
    }
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

    if (options->logLevel >= LogLevel::Verbose)
    {
        Serial.print("Moving status: ");
        Serial.println(isMoving ? "Moving" : "Stopped");
        Serial.print("Moving speed: ");
        Serial.print(movingSpeed);
        Serial.println(" kPa/s");
        Serial.println();
    }
}

void Barometer::sampleBarometer()
{
    if (options->logLevel >= LogLevel::Info)
    {
        Serial.println("Sampling pressure data...");
    }

    getPressure();
    sample->pressure = currentPressure;
    sample->altitude = altitude;
    sample->movingStatus = movingStatus;
    sample->movingSpeed = movingSpeed;

    if (options->logLevel >= LogLevel::Info)
    {
        Serial.println("Pressure data sampled");
    }

    if (options->logLevel >= LogLevel::Info)
    {
        Serial.println("Sampling temperature data...");
    }

    getTemperature();
    sample->temperature = temperature;

    if (options->logLevel >= LogLevel::Info)
    {
        Serial.println("Temperature data sampled");
    }
}

#include <Arduino.h>
#include <Arduino_LPS22HB.h>

#include "barometer.h"
#include "accelerometer.h"

Barometer::Barometer(SampleDataPoint *_sampleDataPoint, SamplerConfig *_samplerConfig)
    : sampleDataPoint(_sampleDataPoint),
      samplerConfig(_samplerConfig)
{
    if (samplerConfig->samplerOptions->logLevel >= LogLevel::Info)
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

    if (samplerConfig->samplerOptions->logLevel >= LogLevel::Info)
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

    if (samplerConfig->samplerOptions->logLevel >= LogLevel::Verbose)
    {
        Serial.print("Altitude according to kPa is = ");
        Serial.print(altitudeMeters);
        Serial.println(" m");
    }
}

void Barometer::getTemperature()
{
    temperatureC = BARO.readTemperature();

    if (samplerConfig->samplerOptions->logLevel >= LogLevel::Verbose)
    {
        Serial.print("Temperature is = ");
        Serial.print(temperatureC);
        Serial.println(" C");
    }
}

void Barometer::sampleTemperature()
{
    if (!samplerConfig->samplerOptions->hasBarSensor)
        return;

    if (samplerConfig->samplerOptions->logLevel >= LogLevel::Info)
    {
        Serial.println("Sampling temperature data...");
    }

    getTemperature();
    sampleDataPoint->temperatureC = temperatureC;

    if (samplerConfig->samplerOptions->logLevel >= LogLevel::Info)
    {
        Serial.println("Temperature data sampled\n");
    }
}

void Barometer::samplePressure(bool logData)
{
    if (!samplerConfig->samplerOptions->hasBarSensor)
        return;

    if (samplerConfig->samplerOptions->logLevel >= LogLevel::Info && logData)
    {
        Serial.println("Sampling pressure data...");
    }

    getPressure();
    sampleDataPoint->pressureKpa = currentPressureKpa;
    sampleDataPoint->altitudeMeters = altitudeMeters;

    if (samplerConfig->samplerOptions->logLevel >= LogLevel::Info && logData)
    {
        Serial.println("Pressure data sampled\n");
    }

    unsigned long currentTimestamp = millis();
    MovingStatus movingStatus = MovingStatus::Stopped;
    MovingDirection movingDirection = MovingDirection::None;
    float movingSpeed = 0.0;

    static unsigned long lastTimestamp = 0;
    static float lastAltitudeMeters = 0.0;
    static float lastSpeed = 0.0;
    if (lastTimestamp == 0)
    {
        lastTimestamp = currentTimestamp;
        lastAltitudeMeters = altitudeMeters;
    }

    unsigned long timeDifference = currentTimestamp - lastTimestamp;
    float altitudeDifference = altitudeMeters - lastAltitudeMeters;
    movingSpeed = abs(altitudeDifference / timeDifference);

    if (movingSpeed < 0.2)
    {
        movingStatus = MovingStatus::Stopped;
        movingDirection = MovingDirection::None;
        movingSpeed = 0.0;
    }
    else
    {
        if (movingSpeed > (lastSpeed + 0.1))
        {
            movingStatus = MovingStatus::Accelerating;
        }
        else if (movingSpeed < (lastSpeed - 0.1))
        {
            movingStatus = MovingStatus::Stopping;
        }
        else
        {
            movingStatus = MovingStatus::Steady;
        }

        if (altitudeDifference > 0.5)
        {
            movingDirection = MovingDirection::Up;
        }
        else if (altitudeDifference < -0.5)
        {
            movingDirection = MovingDirection::Down;
        }
    }

    sampleDataPoint->movingStatus = movingStatus;
    sampleDataPoint->movingDirection = movingDirection;
    sampleDataPoint->movingSpeed = movingSpeed;

    lastAltitudeMeters = altitudeMeters;
    lastTimestamp = currentTimestamp;
    lastSpeed = movingSpeed;
}

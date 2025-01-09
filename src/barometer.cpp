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

    static unsigned long lastTimestamp = currentTimestamp;
    static float lastAltitudeMeters = altitudeMeters;
    static float lastSpeed = 0.0;

    unsigned long timeDifference = currentTimestamp - lastTimestamp;
    double altitudeDifference = altitudeMeters - lastAltitudeMeters;
    double distance = abs(altitudeDifference);
    movingSpeed = timeDifference == 0 ? 0 : distance / timeDifference;

    if (altitudeDifference > 1)
    {
        movingDirection = MovingDirection::Up;
    }
    else if (altitudeDifference < -1)
    {
        movingDirection = MovingDirection::Down;
    }

    if (movingSpeed > (lastSpeed + 0.01))
    {
        movingStatus = MovingStatus::Accelerating;
    }
    else if (movingSpeed < (lastSpeed - 0.01))
    {
        movingStatus = MovingStatus::Stopping;
    }
    else if (movingSpeed < 0.01)
    {
        movingStatus = MovingStatus::Stopped;
    }
    else
    {
        movingStatus = MovingStatus::Steady;
    }

    // If nothing important has changed, return
    if (movingStatus == sampleDataPoint->movingStatus &&
        movingDirection == sampleDataPoint->movingDirection)
    {
        return;
    }

    sampleDataPoint->movingStatus = movingStatus;
    sampleDataPoint->movingDirection = movingDirection;
    sampleDataPoint->movingSpeed = movingSpeed;

    lastAltitudeMeters = altitudeMeters;
    lastTimestamp = currentTimestamp;
    lastSpeed = movingSpeed;

    if (samplerConfig->samplerOptions->logLevel >= LogLevel::Info)
    {
        Serial.print("Moving status: ");
        char movingStatusStr[13];
        switch (sampleDataPoint->movingStatus)
        {
        case MovingStatus::Stopped:
            strcpy(movingStatusStr, "Stopped");
            break;
        case MovingStatus::Accelerating:
            strcpy(movingStatusStr, "Accelerating");
            break;
        case MovingStatus::Steady:
            strcpy(movingStatusStr, "Steady");
            break;
        case MovingStatus::Stopping:
            strcpy(movingStatusStr, "Stopping");
            break;
        default:
            strcpy(movingStatusStr, "Unknown");
            break;
        }
        Serial.println(movingStatusStr);
        Serial.print("Moving direction: ");
        Serial.println(sampleDataPoint->movingDirection == MovingDirection::Up ? "Up" : "Down");
        Serial.print("Moving speed: ");
        Serial.print(sampleDataPoint->movingSpeed);
        Serial.println(" m/s\n");
    }
}

/**
 * Customize MovingStatus and MovingDirection as per your requirements.
 * Create additional enums and triggers for other sensors data if needed,
 * e.g. TiltingStatus, TiltingAngle, gyroscopeTrigger, temperatureTrigger etc.,
 * or even a combination of multiple sensors for a single trigger.
 * The Sample::checkTriggers() will also need to be updated to check for these new custom triggers.
 */

#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <tuple>

enum class LogLevel
{
    None,
    Info,
    Verbose,
};

enum class MovingStatus
{
    Stopped,
    Accelerating,
    Steady,
    Stopping,
};

enum class MovingDirection
{
    None,
    Up,
    Down
};

enum class SupportedSensor
{
    Accelerometer,
    Microphone,
    Barometer,
};

class Config
{
public:
    SupportedSensor *supportedSensors;
    /**
     * If array is empty, it means all sensors are supported.
     * The sensors here mean that will be triggered when the movement status changes.
     */
    std::tuple<MovingStatus, MovingDirection, std::array<SupportedSensor, 3>> *movementTriggers;

    Config(SupportedSensor *supportedSensors, std::tuple<MovingStatus, MovingDirection, std::array<SupportedSensor, 3>> *_movementTriggers)
        : supportedSensors(supportedSensors)
    {
        if (_movementTriggers == nullptr)
        {
            movementTriggers = new std::tuple<MovingStatus, MovingDirection, std::array<SupportedSensor, 3>>[7];
            movementTriggers[0] = std::make_tuple(MovingStatus::Stopped, MovingDirection::None, std::array<SupportedSensor, 3>{});
            movementTriggers[1] = std::make_tuple(MovingStatus::Accelerating, MovingDirection::Up, std::array<SupportedSensor, 3>{});
            movementTriggers[2] = std::make_tuple(MovingStatus::Accelerating, MovingDirection::Down, std::array<SupportedSensor, 3>{});
            movementTriggers[3] = std::make_tuple(MovingStatus::Steady, MovingDirection::Up, std::array<SupportedSensor, 3>{});
            movementTriggers[4] = std::make_tuple(MovingStatus::Steady, MovingDirection::Down, std::array<SupportedSensor, 3>{});
            movementTriggers[5] = std::make_tuple(MovingStatus::Stopping, MovingDirection::Up, std::array<SupportedSensor, 3>{});
            movementTriggers[6] = std::make_tuple(MovingStatus::Stopping, MovingDirection::Down, std::array<SupportedSensor, 3>{});
        }
        else
        {
            movementTriggers = _movementTriggers;
        }
    }
};

#endif // CONFIG_H

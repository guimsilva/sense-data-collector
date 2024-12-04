#ifndef BLE_UTILS_H
#define BLE_UTILS_H

#include <Arduino.h>
#include <ArduinoBLE.h>
#include <ArduinoJson.h>

#include "vibration_utils.h"

#define BLE_SENSE_UUID(val) ("4798e0f2-" val "-4d68-af64-8a8f5258404e")

constexpr uint16_t vibrationSampleByteCount = 5120;
uint8_t vibrationSampleBuffer[vibrationSampleByteCount];

// BLE settings
bool LOG_VIA_BLUETOOTH = true;
BLEService bleService(BLE_SENSE_UUID("0000"));
BLECharacteristic bleVibSampleCharacteristic(BLE_SENSE_UUID("300a"), BLERead | BLENotify, vibrationSampleByteCount);

// String to calculate the local and device name
String bleName;

void bleSetup()
{
    // Configure BLE
    String bleAddress = BLE.address();

    // Output BLE settings over Serial
    Serial.print("address = ");
    Serial.println(bleAddress);

    bleAddress.toUpperCase();

    bleName = "BLESense-";
    bleName += bleAddress[bleAddress.length() - 5];
    bleName += bleAddress[bleAddress.length() - 4];
    bleName += bleAddress[bleAddress.length() - 2];
    bleName += bleAddress[bleAddress.length() - 1];

    Serial.print("BLE name = ");
    Serial.println(bleName);

    BLE.setLocalName(bleName.c_str());
    BLE.setDeviceName(bleName.c_str());
    BLE.setAdvertisedService(bleService);
    bleService.addCharacteristic(bleVibSampleCharacteristic);
    BLE.addService(bleService);

    memset(vibrationSampleBuffer, 0, vibrationSampleByteCount);
    bleVibSampleCharacteristic.writeValue(vibrationSampleBuffer, vibrationSampleByteCount);
    BLE.advertise();

    Serial.println("BLE setup done");
}

void bleComms()
{
    if (!LOG_VIA_BLUETOOTH)
    {
        return;
    }

    BLEDevice bleCentral = BLE.central();

    // if a central is connected to the peripheral:
    static bool bleWasConnectedLast = false;
    if (!bleCentral && !bleWasConnectedLast)
    {
        Serial.println("No central connected");
        return;
    }
    else if (bleCentral && !bleWasConnectedLast)
    {
        bleWasConnectedLast = true;
        Serial.print("Connected to central: ");
        Serial.println(bleCentral.address());
    }
    else if (!bleCentral && bleWasConnectedLast)
    {
        bleWasConnectedLast = false;
        Serial.println("Disconnected from central");
        return;
    }

    if (JSON_DOC.isNull())
    {
        Serial.println("JSON_DOC is null/empty");
        return;
    }

    // Convert the JSON document to the vibrationSampleBuffer array of bytes
    size_t n = serializeJson(JSON_DOC, vibrationSampleBuffer);
    serializeJson(JSON_DOC, vibrationSampleBuffer);
    bleVibSampleCharacteristic.writeValue(vibrationSampleBuffer, vibrationSampleByteCount);
    // bleVibSampleCharacteristic.broadcast();
    // BLE.advertise();
    // BLE.poll();

    JSON_DOC.clear();

    // // Clear the buffer
    // memset(vibrationSampleBuffer, 0, vibrationSampleByteCount);

    Serial.print("Sent BLE data");
}

#endif // BLE_UTILS_H

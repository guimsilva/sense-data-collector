#ifndef BLE_UTILS_H
#define BLE_UTILS_H

#include <Arduino.h>
#include <ArduinoBLE.h>
#include <ArduinoJson.h>

/**
 * This doesn't work for the purpose of collecting vibration samples because of the 512 bytes limit,
 * so it's kept here for reference only and future BLE communication.
 */

#define BLE_SENSE_UUID(val) ("4798e0f2-" val "-4d68-af64-8a8f5258404e")

// BLE settings
bool LOG_VIA_BLUETOOTH = true;
BLEService bleService(BLE_SENSE_UUID("0000"));
BLEStringCharacteristic bleVibSampleCharacteristic(BLE_SENSE_UUID("300a"), BLERead | BLENotify, 512); // Max size is 512 bytes

// String to calculate the local and device name
String bleName;

void bleSetup()
{
    if (!BLE.begin())
    {
        Serial.println("Failed to initialized BLE!");
        while (1)
            ;
    }

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

    String initialVibSample = "{}";
    bleVibSampleCharacteristic.writeValue(initialVibSample);
    BLE.advertise();

    Serial.println("BLE setup done");
}

void bleComms(JsonDocument &jsonDoc)
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

    if (jsonDoc.isNull())
    {
        Serial.println("JSON_DOC is null/empty");
        return;
    }

    // Convert the JSON document to the vibrationSampleBuffer array of bytes
    size_t jsonSize = measureJson(jsonDoc);

    Serial.print("JSON size: ");
    Serial.println(jsonSize);

    String vibrationSampleString;
    serializeJson(jsonDoc, vibrationSampleString);

    // Serial.print("Sending BLE data: ");
    // Serial.println(vibrationSampleString);

    bleVibSampleCharacteristic.writeValue(vibrationSampleString);

    jsonDoc.clear();

    Serial.print("Sent BLE data");
}

#endif // BLE_UTILS_H

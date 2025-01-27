# sense-data-collector

## Introduction

Welcome to my first C++ IoT project! This project aims to collect sensor data using the Arduino Nano 33 BLE Sense Rev2. It is the result of my learning journey with TinyML, so please bear with any beginner issues and feel free to suggest improvements. It's definitely a work in progress!

## Features

- Collects data from various sensors on the Arduino Nano 33 BLE Sense
- Stores the data for further analysis
- Has options for each sensor and different triggers, buffer sizes and intervals
- It was implemented with memory constraints in mind due to the limited resources on the board, which is why it avoids using an OS and other heavy libraries
- Open to suggestions and improvements from the community

## Hardware Required

- Arduino Nano 33 BLE Sense Rev2
- To store the collected data, it will require an SD card module attached

## Libs Required

- Arduino_BMI270_BMM150
- Arduino_LPS22HB
- ArduinoJson
- SD

## Observations

- This repo is a working in progress. Using specific triggers or trying to combine them may not work as expected yet.
- It may work with other variations of Arduino Nano 33 BLE Sense, but it hasn't been tested yet. It may require installation of different libs such as `Arduino_LSM9DS1` and including them on the respective files
- This project has been implemented and tested using PlatformIO (PIO) only

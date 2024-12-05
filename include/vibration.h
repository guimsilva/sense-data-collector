#ifndef VIBRATION_H
#define VIBRATION_H

#include <ArduinoJson.h>
// #include <SPI.h>
// #include <SD.h>

#include "Arduino_BMI270_BMM150.h"
#include "arduinoFFT.h"

struct VibrationSample
{
    VibrationSample(int16_t samples = 512)
        : dominantFrequency(0.0),
          frequencies(new double[samples]),
          timestamp(0)
    {
        for (int i = 0; i < samples; ++i)
        {
            frequencies[i] = 0.0;
        }
    }

    double dominantFrequency;
    double *frequencies;
    unsigned long timestamp;
};

class Vibration
{
private:
    // Used for printing FFT results
    static const uint8_t sclIndex = 0x00;
    static const uint8_t sclTime = 0x01;
    static const uint8_t sclFrequency = 0x02;
    static const uint8_t sclPlot = 0x03;

    // IMU sensor
    float accX, accY, accZ;

    // FFT sampling period and time
    unsigned int sampling_period_us;
    unsigned long microseconds;

    /**
     * These are the input and output vectors
     * Input vectors receive computed results from FFT
     **/
    double *vReal;
    double *vImag;
    double dominantFrequency;

    ArduinoFFT<double> FFT;
    VibrationSample *vibrationSamples;

    void printFFTVector(double *vData, uint16_t bufferSize, uint8_t scaleType)
    {
        for (uint16_t i = 0; i < bufferSize; i++)
        {
            double abscissa;
            /* Print abscissa value */
            switch (scaleType)
            {
            case sclIndex:
                abscissa = (i * 1.0);
                break;
            case sclTime:
                abscissa = ((i * 1.0) / samplingFrequency);
                break;
            case sclFrequency:
                abscissa = ((i * 1.0 * samplingFrequency) / samples);
                break;
            }
            Serial.print(abscissa, 6);
            if (scaleType == sclFrequency)
                Serial.print("Hz");
            Serial.print(" ");
            Serial.println(vData[i], 4);
        }
        Serial.println();
    }

    void saveVibrationSamplesToFile(bool printResults = true)
    {
        if (printResults)
        {
            Serial.println("Saving vibration samples to file");
        }

        jsonDoc.clear();
        JsonArray samples = jsonDoc["samples"].to<JsonArray>();
        for (int i = 0; i < vibrationSamplesBufferSize; i++)
        {
            if (vibrationSamples[i].timestamp == 0)
            {
                break;
            }
            JsonObject sample = samples.add<JsonObject>();
            sample["timestamp"] = vibrationSamples[i].timestamp;
            sample["dominantFrequency"] = vibrationSamples[i].dominantFrequency;
            JsonArray frequencies = sample["frequencies"].to<JsonArray>();
            for (int j = 0; j < samples; j++)
            {
                frequencies.add(vibrationSamples[i].frequencies[j]);
            }
        }

        if (printResults)
        {
            serializeJsonPretty(jsonDoc, Serial);
        }

        // File file = SD.open("/vibration_samples_" + String(millis()) + ".json", FILE_WRITE);
        // if (!file)
        // {
        //     Serial.println("Failed to open file for writing");
        //     return;
        // }
        // serializeJson(jsonDoc, file);
        // file.close();
        // jsonDoc.clear();

        if (printResults)
        {
            Serial.println("Vibration samples saved to file");
        }
    }

public:
    Vibration(int16_t _samples = 512, int16_t _samplingFrequency = 512, int16_t _vibrationSamplesBufferSize = 10)
        : samples(_samples),
          samplingFrequency(_samplingFrequency),
          vibrationSamplesBufferSize(_vibrationSamplesBufferSize),
          vReal(new double[_samples]), vImag(new double[_samples]), dominantFrequency(0.0)
    {
        /* Create FFT object */
        FFT = ArduinoFFT<double>(vReal, vImag, _samples, _samplingFrequency);
        vibrationSamples = new VibrationSample(_vibrationSamplesBufferSize);
    }

    // x = 512 samples and sampling frequency y = 512 will result in 1 second of sampling (x / y = sec)
    const int16_t samples;                    // Must be a power of 2
    const int16_t samplingFrequency;          // Hz. Determines maximum frequency that can be analysed by the FFT.
    const int16_t vibrationSamplesBufferSize; // Number of complete samples to be saved before writing to file

    JsonDocument jsonDoc;

    /**
     * Sampling data and preparation for FFT conversion
     **/
    void sampleVibration(bool printResults = true);

    /**
     * Compute FFT and print results
     **/
    void computeVibrationFFT(bool printResults = true);
};

#endif // VIBRATION_H

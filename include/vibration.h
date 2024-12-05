#ifndef VIBRATION_H
#define VIBRATION_H

#include <ArduinoJson.h>

#include "Arduino_BMI270_BMM150.h"
#include "arduinoFFT.h"
#include "sample.h"

class Vibration
{
private:
    // The sample data point reference
    SampleDataPoint *sample;

    // Used for printing FFT results
    static const uint8_t sclIndex = 0x00;
    static const uint8_t sclTime = 0x01;
    static const uint8_t sclFrequency = 0x02;
    static const uint8_t sclPlot = 0x03;

    // IMU sensor
    float accX, accY, accZ;

    // FFT sampling period and time
    unsigned int samplingPeriodUs;
    unsigned long microseconds;

    /**
     * These are the input and output vectors
     * Input vectors receive computed results from FFT
     **/
    double *vReal;
    double *vImag;
    double dominantFrequency;

    ArduinoFFT<double> FFT;

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

public:
    Vibration(SampleDataPoint *_sample, int16_t _samples = 512, int16_t _samplingFrequency = 512)
        : samples(_samples),
          samplingFrequency(_samplingFrequency),
          vReal(new double[_samples]), vImag(new double[_samples]), dominantFrequency(0.0)
    {
        sample = _sample;
        samplingPeriodUs = round(1000000 * (1.0 / samplingFrequency));

        /* Create FFT object */
        FFT = ArduinoFFT<double>(vReal, vImag, _samples, _samplingFrequency);
    }

    // x = 512 samples and sampling frequency y = 512 will result in 1 second of sampling (x / y = sec)
    const int16_t samples;           // Must be a power of 2
    const int16_t samplingFrequency; // Hz. Determines maximum frequency that can be analysed by the FFT.

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

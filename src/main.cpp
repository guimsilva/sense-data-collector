#include <Arduino.h>
#include <Arduino_LPS22HB.h>

#include "sampler.h"

namespace
{
  // Time interval for data collection
  unsigned long previousMillis = 0;            // Store the last time the data collection event occurred
  const unsigned long intervalInMillis = 1000; // Interval at which to trigger the data collection event (milliseconds)
  unsigned long currentMillis = 0;
  bool isTimeForDataCollection = false;

  SamplerOptions *options;
  Sampler *sampler;
} // namespace

void setup()
{
  Serial.begin(9600);
  while (!Serial)
    ;
  Serial.println("Serial started");

  options = new SamplerOptions(512, 0, 10, LogLevel::Info, true);
  sampler = new Sampler(options);
}

void loop()
{
  currentMillis = millis();
  isTimeForDataCollection = previousMillis == 0 || currentMillis - previousMillis >= intervalInMillis;
  if (!isTimeForDataCollection)
    return;

  if (options->logLevel >= LogLevel::Info)
  {
    Serial.println("Collecting acc data...");
  }

  previousMillis = currentMillis;
  sampler->sampleData();

  if (options->logLevel >= LogLevel::Info)
  {
    Serial.println("Acc data collected");
  }
}

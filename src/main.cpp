#include <Arduino.h>
#include <Arduino_LPS22HB.h>

#include "sampler.h"

namespace
{
  SamplerOptions *samplerOptions;
  Sampler *sampler;
} // namespace

void setup()
{
  Serial.begin(9600);
  while (!Serial)
    ;
  Serial.println("\nSerial started\n");

  samplerOptions = new SamplerOptions(3000, 256, 0, 5, LogLevel::Info, true);
  sampler = new Sampler(samplerOptions);
}

void loop()
{
  sampler->sampleData();
}

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

  samplerOptions = new SamplerOptions(5000, 256, 0, 16000, 5, LogLevel::Verbose, false);
  sampler = new Sampler(samplerOptions);

  Serial.println("Completed setup()\n");
}

void loop()
{
  sampler->sampleData();
}

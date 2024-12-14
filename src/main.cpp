#include <Arduino.h>
#include <Arduino_LPS22HB.h>

#include "sampler.h"
#include "config.h"

namespace
{
  SamplerOptions *samplerOptions;
  SamplerConfig *samplerConfig;
  Sampler *sampler;
} // namespace

void setup()
{
  Serial.begin(9600);
  while (!Serial)
    ;
  Serial.println("\nSerial started\n");

  samplerOptions = new SamplerOptions(256, 0, 16000, 5, LogLevel::Info, false);
  samplerConfig = new SamplerConfig(samplerOptions);
  sampler = new Sampler(samplerConfig);

  Serial.println("Completed setup()\n");
}

void loop()
{
  sampler->sampleData();
}

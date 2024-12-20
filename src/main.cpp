#include <Arduino.h>
#include <Arduino_LPS22HB.h>

#include "sampler.h"

namespace
{
  SamplerOptions *samplerOptions;
  AccOptions *accOptions;
  MicOptions *micOptions;
  SamplerConfig *samplerConfig;
  Sampler *sampler;
} // namespace

void setup()
{
  Serial.begin(9600);
  while (!Serial)
    ;
  Serial.println("\nSerial started\n");

  samplerOptions = new SamplerOptions(false, LogLevel::Info, 3, 0, new Triggers[1]{Triggers::Movement}, 1);
  accOptions = new AccOptions();
  micOptions = new MicOptions();
  samplerConfig = new SamplerConfig(samplerOptions, accOptions, micOptions);
  sampler = new Sampler(samplerConfig);

  Serial.println("Completed setup()\n");
}

void loop()
{
  sampler->checkTriggers();
}

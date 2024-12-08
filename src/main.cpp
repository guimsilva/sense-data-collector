#include <Arduino.h>
#include <Arduino_LPS22HB.h>

#include "sampler.h"

namespace
{
  SamplerOptions *options;
  Sampler *sampler;
} // namespace

void setup()
{
  Serial.begin(9600);
  while (!Serial)
    ;
  Serial.println("Serial started");

  options = new SamplerOptions(2000, 256, 0, 5, LogLevel::Info, true);
  sampler = new Sampler(options);
}

void loop()
{
  sampler->sampleData();
}

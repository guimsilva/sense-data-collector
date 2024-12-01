/* Copyright 2020 The TensorFlow Authors. All Rights Reserved.
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at
    http://www.apache.org/licenses/LICENSE-2.0
Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/
#include <Arduino.h>
#include <TensorFlowLite.h>
#include <Arduino_LPS22HB.h>

#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "tensorflow/lite/version.h"

#include "vibration_model_data.h"
#include "rasterize_stroke.h"
#include "imu_provider.h"

#define BLE_SENSE_UUID(val) ("4798e0f2-" val "-4d68-af64-8a8f5258404e")

namespace
{
  const int VERSION = 0x00000000;

  // Constants for image rasterization
  constexpr int raster_width = 32;
  constexpr int raster_height = 32;
  constexpr int raster_channels = 3;
  constexpr int raster_byte_count = raster_height * raster_width * raster_channels;
  int8_t raster_buffer[raster_byte_count];

  // BLE settings
  BLEService ble_service(BLE_SENSE_UUID("0000"));
  BLECharacteristic ble_strokeCharacteristic(BLE_SENSE_UUID("300a"), BLERead, stroke_struct_byte_count);

  // String to calculate the local and device name
  String ble_name;

  // Create an area of memory to use for input, output, and intermediate arrays.
  // The size of this will depend on the model you're using, and may need to be
  // determined by experimentation.
  constexpr int kTensorArenaSize = 30 * 1024;
  uint8_t tensor_arena[kTensorArenaSize];

  tflite::ErrorReporter *error_reporter = nullptr;
  const tflite::Model *model = nullptr;
  tflite::MicroInterpreter *interpreter = nullptr;

  // -------------------------------------------------------------------------------- //
  // UPDATE THESE VARIABLES TO MATCH THE NUMBER AND LIST OF GESTURES IN YOUR DATASET  //
  // -------------------------------------------------------------------------------- //
  constexpr int label_count = 10;
  const char *labels[label_count] = {"0", "1", "2", "3", "4", "5", "6", "7", "8", "9"};
} // namespace

void setup()
{
  // Start serial
  Serial.begin(9600);
  while (!Serial)
    ;
  Serial.println("Serial started");

  // Start IMU
  if (!IMU.begin())
  {
    Serial.println("Failed to initialized IMU!");
    while (1)
      ;
  }
  SetupIMU();

  // Start BLE
  if (!BLE.begin())
  {
    Serial.println("Failed to initialized BLE!");
    while (1)
      ;
  }
  String ble_address = BLE.address();

  // Output BLE settings over Serial
  Serial.print("address = ");
  Serial.println(ble_address);

  ble_address.toUpperCase();

  ble_name = "BLESense-";
  ble_name += ble_address[ble_address.length() - 5];
  ble_name += ble_address[ble_address.length() - 4];
  ble_name += ble_address[ble_address.length() - 2];
  ble_name += ble_address[ble_address.length() - 1];

  Serial.print("BLE name = ");
  Serial.println(ble_name);

  BLE.setLocalName(ble_name.c_str());
  BLE.setDeviceName(ble_name.c_str());
  BLE.setAdvertisedService(ble_service);

  ble_service.addCharacteristic(ble_strokeCharacteristic);

  BLE.addService(ble_service);
  BLE.advertise();

  // Set up logging. Google style is to avoid globals or statics because of
  // lifetime uncertainty, but since this has a trivial destructor it's okay.
  static tflite::MicroErrorReporter micro_error_reporter; // NOLINT
  error_reporter = &micro_error_reporter;

  // Map the model into a usable data structure. This doesn't involve any
  // copying or parsing, it's a very lightweight operation.
  model = tflite::GetModel(g_vibration_model_data);
  if (model->version() != TFLITE_SCHEMA_VERSION)
  {
    TF_LITE_REPORT_ERROR(error_reporter,
                         "Model provided is schema version %d not equal "
                         "to supported version %d.",
                         model->version(), TFLITE_SCHEMA_VERSION);
    return;
  }

  // Pull in only the operation implementations we need.
  // This relies on a complete list of all the ops needed by this graph.
  // An easier approach is to just use the AllOpsResolver, but this will
  // incur some penalty in code space for op implementations that are not
  // needed by this graph.
  static tflite::MicroMutableOpResolver<4> micro_op_resolver; // NOLINT
  micro_op_resolver.AddConv2D();
  micro_op_resolver.AddMean();
  micro_op_resolver.AddFullyConnected();
  micro_op_resolver.AddSoftmax();

  // Build an interpreter to run the model with.
  static tflite::MicroInterpreter static_interpreter(
      model, micro_op_resolver, tensor_arena, kTensorArenaSize, error_reporter);
  interpreter = &static_interpreter;

  // Allocate memory from the tensor_arena for the model's tensors.
  interpreter->AllocateTensors();

  // Set model input settings
  TfLiteTensor *model_input = interpreter->input(0);
  if ((model_input->dims->size != 4) || (model_input->dims->data[0] != 1) || (model_input->dims->data[1] != raster_height) || (model_input->dims->data[2] != raster_width) || (model_input->dims->data[3] != raster_channels) || (model_input->type != kTfLiteInt8))
  {
    TF_LITE_REPORT_ERROR(error_reporter, "Bad input tensor parameters in model");
    return;
  }

  // Set model output settings
  TfLiteTensor *model_output = interpreter->output(0);
  if ((model_output->dims->size != 2) || (model_output->dims->data[0] != 1) || (model_output->dims->data[1] != label_count) || (model_output->type != kTfLiteInt8))
  {
    TF_LITE_REPORT_ERROR(error_reporter, "Bad output tensor parameters in model");
    return;
  }
}

void loop()
{
  BLEDevice ble_central = BLE.central();

  // if a central is connected to the peripheral:
  static bool ble_was_connected_last = false;
  if (ble_central && !ble_was_connected_last)
  {
    Serial.print("Connected to central: ");
    // print the central's BT address:
    Serial.println(ble_central.address());
  }
  ble_was_connected_last = ble_central;

  Serial.println("ALL GOOD");
  delay(1000);
}

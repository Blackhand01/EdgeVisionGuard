// Nicla Vision — FocoosAI (fai-cls-n-coco) + Z-Ant smoke test
// Goal: verify toolchain & library linking by running inference on a dummy tensor.
//
// 🔁 Steps before building this sketch:
// 1) Export ONNX with Focoos CLI:
//      focoos export --model fai-cls-n-coco --format onnx --im-size 96 --output-dir ../models
// 2) Prepare & build Z-Ant (clone Z-Ant beside this folder or set paths accordingly):
//      # put ONNX at Z-Ant/datasets/models/fai-cls-n-coco/fai-cls-n-coco.onnx
//      ./zant input_setter --path datasets/models/fai-cls-n-coco/fai-cls-n-coco.onnx --shape 1,3,96,96
//      zig build lib-gen -Dmodel="fai-cls-n-coco"
//      zig build lib -Dmodel="fai-cls-n-coco"
//    Copy generated headers + static lib into: arduino/lib/ZAnt/
//
// 3) Update the include + function names below to match Z-Ant's generated API.
//
// Note: This test does NOT use the camera. It only checks that the generated
//       model library links and runs on Nicla by feeding a deterministic buffer.

#include <Arduino.h>

// ---- Z-Ant generated header ----
// Replace "zant_model.h" with the actual header name produced by Z-Ant (check generated/include/).
// Open that header and confirm the function signatures, then update the prototypes below.
// Common pattern (example): void zant_init(); void zant_infer(const float* in, float* out);
#include "ZAnt/zant_model.h"   // <-- put the real header here

extern "C" {
  // ⚠️ EDIT THESE PROTOTYPES to match your generated header exactly.
  // Example signatures (placeholders):
  void zant_infer(const float* input, float* output);
}

// ---- Model I/O sizes (must match the model you exported) ----
// For fai-cls-n-coco with --im-size 96 and 3 channels:
static const int IN_N = 1;
static const int IN_C = 3;
static const int IN_H = 96;
static const int IN_W = 96;
static const int OUT_CLASSES = 80;  // COCO classes (example)

// Buffers
static float input_tensor[IN_N * IN_C * IN_H * IN_W];
static float output_tensor[OUT_CLASSES];

// Simple argmax
int argmax(const float* arr, int len) {
  int best_i = 0;
  float best_v = arr[0];
  for (int i = 1; i < len; ++i) {
    if (arr[i] > best_v) { best_v = arr[i]; best_i = i; }
  }
  return best_i;
}

void fill_dummy_input() {
  // Deterministic pattern (0..1 ramp) so runs are reproducible
  const int size = IN_N * IN_C * IN_H * IN_W;
  for (int i = 0; i < size; ++i) {
    input_tensor[i] = (float)i / (float)size;
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n[Z-Ant smoke] Booting");

  // If available on your core, you can use the Nicla LED API:
  // nicla::begin(); nicla::leds.begin(); nicla::leds.setColor(green);

  fill_dummy_input();

  Serial.println("[Z-Ant smoke] ready");
}

void loop() {
  Serial.println("[Z-Ant smoke] infer()");
  unsigned long t0 = millis();
  zant_infer(input_tensor, output_tensor);
  unsigned long t1 = millis();
  int top = argmax(output_tensor, OUT_CLASSES);

  Serial.print("Top-1 class: "); Serial.print(top);
  Serial.print(" | t="); Serial.print(t1 - t0); Serial.println(" ms");

  // Optional LED pulse between inferences:
  // nicla::leds.setColor(blue); delay(100);
  // nicla::leds.setColor(off);

  delay(1000);
}
# Nicla Vision + FocoosAI + Z-Ant — Starter Template

This template helps you validate the full pipeline:
FocoosAI (export ONNX) → Z-Ant (generate & build library with Zig) → Arduino CLI (flash to Nicla Vision).

## Prereqs
- Python 3.10–3.12, Focoos SDK installed
- Zig 0.14.x
- arduino-cli, dfu-util
- Z-Ant repo cloned (sibling folder recommended): ../Z-Ant

## 1) Export a tiny classification model (fai-cls-n-coco) as ONNX
```bash
focoos export --model fai-cls-n-coco --format onnx --im-size 96 --output-dir ./models
```

## 2) Prepare the model for Z-Ant and build the static library
```bash
# Copy ONNX into Z-Ant expected location
mkdir -p ../Z-Ant/datasets/models/fai-cls-n-coco
cp ./models/fai-cls-n-coco.onnx ../Z-Ant/datasets/models/fai-cls-n-coco/fai-cls-n-coco.onnx

cd ../Z-Ant
./zant input_setter --path datasets/models/fai-cls-n-coco/fai-cls-n-coco.onnx --shape 1,3,96,96
zig build lib-gen -Dmodel="fai-cls-n-coco"
zig build lib -Dmodel="fai-cls-n-coco"

# Inspect Z-Ant build output; copy generated headers & static lib back into this project:
# (Adjust paths/names according to the build output)
cp -r generated/include/* ../nicla_focoos_zant_template/arduino/lib/ZAnt/ 2>/dev/null || true
cp generated/lib/*.a ../nicla_focoos_zant_template/arduino/lib/ZAnt/ 2>/dev/null || true
```

## 3) Wire the header in the sketch
- Edit `arduino/src/main.ino`:
  - Replace `#include "ZAnt/zant_model.h"` with the **actual** header name from Z-Ant.
  - Update the two function prototypes (`zant_init`, `zant_infer`) to match your generated API.

## 4) Build and upload to Nicla Vision
```bash
arduino-cli compile --fqbn arduino:mbed_nicla:niclavision ./arduino
arduino-cli upload -p /dev/ttyACM0 --fqbn arduino:mbed_nicla:niclavision ./arduino
# If needed: double-tap reset to enter DFU bootloader, then use dfu-util
```

## Notes
- The sketch feeds a deterministic dummy tensor; it's only a **smoke test** to validate the pipeline.
- Once this runs, swap the dummy tensor with real camera frames and the proper pre-processing.
# Z-Ant outputs here

Place the **generated headers** and **static library** from Z-Ant here, for example:

  arduino/lib/ZAnt/
    ├── zant_model.h           # header produced by Z-Ant (name may differ)
    ├── zant_runtime.h         # any extra headers (if present)
    └── libzant_model.a        # static library to link (name may differ)

Then update `arduino/src/main.ino` include and function prototypes to match your generated API.

Build:
  arduino-cli compile --fqbn arduino:mbed_nicla:niclavision ../arduino
Upload:
  arduino-cli upload -p <PORT> --fqbn arduino:mbed_nicla:niclavision ../arduino
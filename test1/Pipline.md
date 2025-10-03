Perfetto—ecco il “copiaincolla” completo per il tuo compagno, partendo **solo** dallo ZIP `nicla_focoos_zant_template.zip` su macOS (Apple Silicon). È tutto in sequenza.

> Sostituisci i percorsi con i tuoi se diverso da `~/Downloads`.

---

# 0) Scompatta e posizionati nel progetto

```bash
cd ~/Downloads
unzip nicla_focoos_zant_template.zip
cd nicla_focoos_zant_template
PROJ="$PWD"
```

# 1) Strumenti di sistema (una volta sola)

```bash
# Homebrew e tool necessari
brew update
brew install bash arduino-cli dfu-util git llvm

# Bash moderna (serve per lo script Z-Ant)
 /opt/homebrew/bin/bash --version

# Arduino CLI: core per Nicla Vision
arduino-cli config init
arduino-cli core update-index
arduino-cli core install arduino:mbed_nicla
```

# 2) Zig 0.14 (richiesto da Z-Ant)

```bash
mkdir -p ~/zig && cd ~/zig
curl -L https://ziglang.org/download/0.14.0/zig-macos-aarch64-0.14.0.tar.xz -o zig-0.14.0.tar.xz
tar -xf zig-0.14.0.tar.xz
export ZIG=~/zig/zig-macos-aarch64-0.14.0/zig
$ZIG version       # deve stampare 0.14.0
cd "$PROJ"
```

> (Su Mac Intel usa il pacchetto `zig-macos-x86_64-0.14.0.tar.xz` e cambia il path in `$ZIG`.)

# 3) Python 3.11 + ambiente (evita rogne con onnx/onnxsim)

```bash
brew install python@3.11
/opt/homebrew/bin/python3.11 -m venv .venv311
source .venv311/bin/activate
python -V

pip install -U pip uv
uv pip install "focoos[onnx-cpu] @ git+https://github.com/FocoosAI/focoos.git"
pip install onnx onnxruntime
```

# 4) Esporta un modello Focoos (piccolo) in ONNX

```bash
# Modello “nano” per smoke test
focoos export --model fai-cls-n-coco --format onnx --im-size 96 --device cpu --output-dir ./models
ls -lh models    # deve esserci model.onnx (o simile)
```

# 5) Clona Z-Ant nel progetto

```bash
cd "$PROJ"
git clone https://github.com/ZantFoundation/Z-Ant.git
```

# 6) Copia l’ONNX dove lo aspetta Z-Ant

```bash
mkdir -p Z-Ant/datasets/models/fai-cls-n-coco
cp models/*.onnx Z-Ant/datasets/models/fai-cls-n-coco/fai-cls-n-coco.onnx
```

# 7) **Fissa la shape** nel modello (salta onnxsim)

> Dal log standard l’input si chiama `images`. Se differisce, lo script usa comunque **il primo input**.

```bash
cd Z-Ant
python - <<'PY'
import onnx
from onnx import shape_inference
PATH="datasets/models/fai-cls-n-coco/fai-cls-n-coco.onnx"
m=onnx.load(PATH)
inp=m.graph.input[0]                 # usa il primo input (tipicamente 'images')
dims=[1,3,96,96]                     # NCHW
tt=inp.type.tensor_type
tt.shape.dim.clear()
for d in dims:
    dd=onnx.TensorShapeProto.Dimension(); dd.dim_value=d; tt.shape.dim.append(dd)
m=shape_inference.infer_shapes(m)
onnx.checker.check_model(m)
onnx.save(m, PATH)
print("OK: fixed shape for", inp.name, "->", dims)
PY
```

# 8) Genera **codice** e **libreria** con Zig (per Cortex-M7)

```bash
# Generazione codice dal modello
$ZIG build lib-gen -Dmodel="fai-cls-n-coco"

# Build libreria statica per MCU (Nicla Vision ha Cortex-M7)
$ZIG build lib -Dmodel="fai-cls-n-coco" \
  -Dtarget=thumb-freestanding -Dcpu=cortex_m7 -Doptimize=ReleaseSmall

# Verifica dove sta la .a
find zig-out -type f -name "*.a" -print
```

# 9) Copia la libreria nel progetto Arduino

```bash
cd "$PROJ"
mkdir -p arduino/lib/ZAnt
cp Z-Ant/zig-out/**/lib*.a arduino/lib/ZAnt/
```

# 10) Trova i **nomi delle funzioni** esportate dalla lib

```bash
# Aggiungi strumenti LLVM al PATH (se non già)
export PATH="/opt/homebrew/opt/llvm/bin:$PATH"

# (A) veloce: leggi dal codice Zig generato le funzioni C esportate
grep -n "pub export fn" Z-Ant/generated/fai-cls-n-coco/lib_fai-cls-n-coco.zig

# (B) alternativa: elenca i simboli nella .a
llvm-nm -g --defined-only arduino/lib/ZAnt/lib*.a
```

> Prendi i **nomi esatti** (es. `predict`, `reset`, …). Servono per i prototipi C nello sketch.

# 11) **(1 volta)** Apri lo sketch e aggiorna i prototipi

```bash
# Apri con VS Code (o usa nano/vi)
code -n arduino/src/main.ino
```

* Sostituisci l’`#include "ZAnt/zant_model.h"` se non esiste un header generato (puoi anche rimuoverlo).
* Metti i prototipi in `extern "C"` con i **nomi reali** e tipi `const float*` / `float*` se richiesti, ad es.:

  ```cpp
  extern "C" {
    void predict(const float* input, float* output);
    // void reset(void);  // se presente
  }
  ```
* **Non cambiare** il resto: il test crea un tensore dummy e chiama `predict(...)`.

# 12) Compila, carica, monitora

```bash
# Compila per Nicla Vision
arduino-cli compile --fqbn arduino:mbed_nicla:niclavision ./arduino

# Trova la porta (collega la board via USB)
arduino-cli board list
PORT=/dev/tty.usbmodemXXXX   # <-- sostituisci

# Upload
arduino-cli upload -p "$PORT" --fqbn arduino:mbed_nicla:niclavision ./arduino

# Monitor seriale (115200)
arduino-cli monitor -p "$PORT" -c baudrate=115200
```

Output atteso ripetuto ogni ~1s:

```
Top-1 class: <indice> | t=<ms> ms
```

# 13) (Opzionale) Se non carica via seriale: DFU

```bash
# doppio tap sul reset → DFU mode
dfu-util -l
# (facoltativo) flash bin:
# dfu-util -a 0 -D ./arduino/build/arduino.mbed_nicla.niclavision/arduino.ino.bin
```

---

## Done ✅

A questo punto il tuo compagno ha validato l’intero giro **Focoos → Z-Ant/Zig → Arduino**.
Se volete, il passo successivo è sostituire il tensore finto con frame da camera e mappare il Top-1 a un LED/azione.

## Descrizione Dettagliata del Repository Focoos

Il repository **Focoos** è una libreria Python open-source sviluppata da Focoos AI, progettata per facilitare lo sviluppo e l'implementazione di soluzioni di computer vision (visione artificiale) efficienti e personalizzabili. Si tratta di una piattaforma che permette agli sviluppatori e alle aziende di selezionare, fine-tunare e distribuire modelli di intelligenza artificiale pre-addestrati per compiti come il rilevamento di oggetti, la segmentazione semantica, la segmentazione di istanze e il rilevamento di keypoints (punti chiave). La libreria è ottimizzata per essere utilizzata sia in ambienti cloud che su dispositivi edge, con un focus su prestazioni elevate, basso consumo di risorse e facilità d'uso.

### Struttura del Repository
Il repository è organizzato in moduli Python ben strutturati, con documentazione completa e esempi pratici. Ecco una panoramica dei componenti principali:

- **focoos/**: Il pacchetto principale contenente il codice sorgente.
  - **cli/**: Interfaccia a riga di comando per operazioni come benchmark, esportazione e gestione del hub.
  - **data/**: Utilità per la gestione dei dataset, inclusi caricatori automatici (AutoDataset), trasformazioni e augmentations.
  - **hub/**: Integrazione con Focoos Hub per sincronizzazione di modelli e dati.
  - **infer/**: Moduli per l'inferenza ottimizzata, con supporto per runtime come TorchScript, ONNX e TensorRT.
  - **model_registry/**: Registro dei modelli pre-addestrati disponibili.
  - **models/**: Implementazioni dei modelli (es. BiSeNetFormer, FAI-DETR, FAI-MF, RTMO).
  - **trainer/**: Framework di training con supporto per checkpointing, logging e valutazione.
  - **utils/**: Utilità varie come visualizzatori, metriche e gestione del sistema.

- **docs/**: Documentazione completa in formato MkDocs, con guide per setup, training, inference e API reference.
- **tests/**: Suite di test per garantire la qualità del codice.
- **tutorials/**: Notebook Jupyter per esempi pratici (es. training, inference, quantizzazione).
- **scripts/**: Script di esempio per training e validazione.

Il repository utilizza Python 3.10+, con dipendenze chiave come PyTorch, TorchVision, OpenCV, Pillow e altre librerie per computer vision. La versione attuale è 0.21.0.

### Modelli Disponibili
Focoos offre una collezione di modelli pre-addestrati ottimizzati per efficienza (velocità e consumo energetico). I modelli sono basati su architetture avanzate come RT-DETR, Mask2Former e BiSeNetFormer, addestrati su dataset standard come COCO, ADE20K e Objects365. Ecco una sintesi:

- **Segmentazione Semantica**: Modelli come fai-mf-l-ade (mIoU 48.27), bisenetformer-l-ade (mIoU 45.07) per scene comuni (150 classi).
- **Rilevamento Oggetti**: Modelli come fai-detr-l-coco (AP 53.06) per oggetti comuni (80 classi).
- **Segmentazione di Istanze**: Modelli come fai-mf-l-coco-ins (AP 44.23) per oggetti con maschere.
- **Rilevamento Keypoints**: Modelli come rtmo-l-coco (AP 72.14) per pose umane.

Questi modelli raggiungono fino a 10x velocità superiori e 4x costi inferiori rispetto a modelli tradizionali, con supporto per inferenza su GPU NVIDIA (fino a 181 FPS su T4).

### Caratteristiche Principali
- **Modelli Pre-addestrati Frugali**: Accesso rapido a modelli efficienti tramite ModelRegistry.
- **Fine-Tuning Facile**: Personalizzazione su dataset propri con poche righe di codice.
- **Inferenza Ottimizzata**: Esportazione in runtime come TorchScript, ONNX (CPU/GPU), TensorRT per prestazioni massime.
- **Integrazione Focoos Hub**: Sincronizzazione di esperimenti, modelli e metriche su una piattaforma cloud (opzionale, richiede API key).
- **Supporto Multi-Runtime**: Training con PyTorch, inferenza con accelerazione hardware.
- **Dataset Automatici**: Supporto per layout come COCO, Roboflow, con augmentations integrate.
- **CLI e API**: Interfaccia flessibile per sviluppatori.

## Come Usarlo per il Training di Modelli di Computer Vision

Focoos semplifica il training di modelli CV attraverso un pipeline intuitiva. Di seguito, una guida passo-passo per fine-tunare un modello su un dataset personalizzato. Il processo richiede tipicamente pochi minuti per setup e ore/giorni per training, a seconda della complessità.

### 1. Installazione
Installa la libreria in un ambiente virtuale (raccomandato Python 3.10+):

```bash
# Con UV (raccomandato)
uv venv --python 3.12
source .venv/bin/activate
uv pip install 'focoos @ git+https://github.com/FocoosAI/focoos.git'

# Per inferenza ottimizzata (opzionale)
uv pip install 'focoos[onnx] @ git+https://github.com/FocoosAI/focoos.git'  # GPU
# o 'focoos[tensorrt]' per TensorRT
```

Per Docker: `docker build -t focoos-gpu . --target=focoos-gpu` e `docker run -it focoos-gpu`.

### 2. Preparazione del Dataset
- **Formato Supportato**: Dataset in formato ZIP con layout come COCO JSON, Roboflow, o altri (vedi `DatasetLayout`).
- **Caricamento**: Usa `AutoDataset` per importare automaticamente.

Esempio:
```python
from focoos.data import AutoDataset, DatasetSplitType, get_default_by_task
from focoos import Task, DatasetLayout

# Definisci task (es. rilevamento oggetti)
task = Task.DETECTION
layout = DatasetLayout.ROBOFLOW_COCO

# Carica dataset (locale o da hub)
auto_dataset = AutoDataset(dataset_name="path/to/dataset.zip", task=task, layout=layout)

# Applica augmentations (es. risoluzione 640)
train_augs, val_augs = get_default_by_task(task, 640, advanced=False)
train_dataset = auto_dataset.get_split(augs=train_augs.get_augmentations(), split=DatasetSplitType.TRAIN)
valid_dataset = auto_dataset.get_split(augs=val_augs.get_augmentations(), split=DatasetSplitType.VAL)
```

Se usi Focoos Hub (opzionale):
```python
from focoos import FocoosHUB
hub = FocoosHUB(api_key="your_api_key")
dataset = hub.get_remote_dataset("dataset_ref")
dataset_path = dataset.download_data()
```

### 3. Selezione e Configurazione del Modello
- Scegli un modello pre-addestrato dal registro (es. "fai-detr-m-coco" per rilevamento).
- Configura iperparametri con `TrainerArgs`.

Esempio:
```python
from focoos import ModelManager, TrainerArgs

# Carica modello
model = ModelManager.get("fai-detr-m-coco")  # O "hub://model_ref" se da hub

# Configura training
args = TrainerArgs(
    run_name=f"{model.name}_{train_dataset.name}",
    batch_size=16,  # Immagini per iterazione
    max_iters=500,  # Numero massimo iterazioni
    eval_period=100,  # Valutazione ogni 100 iterazioni
    learning_rate=0.0001,  # Tasso di apprendimento
    weight_decay=0.0001,  # Regolarizzazione
    sync_to_hub=False,  # True per sincronizzare su hub
    output_dir="./experiments"  # Directory di output
)
```

### 4. Training del Modello
Avvia il training con il metodo `train()`:
```python
model.train(args, train_dataset, valid_dataset, hub=hub)  # hub opzionale
```

Durante il training:
- Supporto per mixed precision, scheduling LR automatico, checkpointing.
- Monitoraggio con TensorBoard (integrato).
- Se abilitato, sincronizzazione su Focoos Hub per tracking esperimenti.

### 5. Valutazione e Test
Dopo il training, valuta il modello:
```python
model.eval(args, valid_dataset)
```

Test su immagini:
```python
import random
from PIL import Image

index = random.randint(0, len(valid_dataset))
image = Image.open(valid_dataset[index]["file_name"])
outputs = model.infer(image, annotate=True)
# Visualizza outputs (es. bounding boxes o segmentazioni)
```

### 6. Esportazione e Inferenza Ottimizzata
Esporta per inferenza efficiente:
```python
from focoos import RuntimeType

infer_model = model.export(runtime_type=RuntimeType.ONNX_CUDA32)  # O TORCHSCRIPT_32, ONNX_TRT32, etc.
infer_model.benchmark()  # Benchmark prestazioni
```

Inferenza:
```python
detections = infer_model.infer("path/to/image.jpg", annotate=True)
```

### Note Aggiuntive
- **Requisiti Hardware**: CPU per test, GPU NVIDIA per training/inferenza veloce (CUDA 12+).
- **Ottimizzazioni**: Modelli fino a 4x più economici in termini di energia.
- **Esempi Completi**: Vedi `tutorials/training.ipynb` per un notebook interattivo.
- **Supporto**: Documentazione su https://focoosai.github.io/focoos/, community su GitHub.

Questa libreria è ideale per prototipazione rapida e deployment su edge, riducendo costi e tempo rispetto a framework tradizionali.

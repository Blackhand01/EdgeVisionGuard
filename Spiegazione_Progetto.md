# Spiegazione del Progetto: Test di Arduino Nicla Vision con Librerie Zig e XIP

## Cosa Deve Essere Fatto
Il compito è utilizzare i file dai repository `focoos` e `Z-Ant` per testare la scheda Arduino Nicla Vision. Seguiremo la guida fornita per costruire una libreria statica da un modello Zig (ad esempio mobilenet_v2), impacchettarla come libreria Arduino, abilitare QSPI XIP (Execute In Place) su Arduino Nicla Vision (STM32H747), posizionare i pesi del modello in una sezione dedicata `.flash_weights` mappata a 0x9000_0000, e dividere/flashare il firmware interno e i pesi esterni tramite objcopy e dfu-util.

## Perché
Lo scopo è testare il deployment di un modello di machine learning su una scheda embedded come Arduino Nicla Vision, utilizzando tecniche avanzate come XIP per eseguire i pesi direttamente dalla memoria flash esterna senza copiarli in RAM, ottimizzando l'uso delle risorse limitate della scheda.

## Scopo Finale
Dimostrare che è possibile eseguire inferenze di modelli ML su dispositivi embedded con prestazioni ottimali, utilizzando toolchain come Zig per la generazione di codice efficiente e Arduino per l'integrazione hardware.

## Cosa Verrà Installato
- **Arduino CLI**: Per compilare e gestire progetti Arduino.
- **Core Arduino Nicla Vision (arduino:mbed_nicla)**: Versione 4.4.1 o compatibile.
- **Toolchain arm-none-eabi**: Per compilazione cross-platform.
- **dfu-util**: Per upload via DFU (Device Firmware Update).
- **Zig toolchain**: Per costruire la libreria statica .a dal progetto Zig.
- **Libreria Arduino ZantLib**: Creata manualmente con il .a generato, header e properties.

## Come Testarlo
1. **Preparazione Ambiente**: Installare Arduino CLI, Zig, dfu-util e configurare il core Nicla Vision.
2. **Costruzione Libreria**: Usare Zig per generare libzant.a con flag XIP.
3. **Packaging Libreria Arduino**: Creare la struttura ~/Arduino/libraries/ZantLib con .a, header e properties.
4. **Sketch Arduino**: Usare tiny_hack.ino per inizializzare QSPI, abilitare XIP e chiamare predict().
5. **Linker Custom**: Usare custom.ld per mappare .flash_weights a QSPI.
6. **Compilazione**: Usare arduino-cli con linker custom.
7. **Split e Flash**: Usare script per separare firmware e pesi, flashare via DFU.
8. **Verifica**: Aprire Serial a 115200 baud, controllare output per QSPI init, memory-mapped e predict().

## TODO
- [ ] Verificare installazione prerequisiti (Arduino CLI, Zig, dfu-util).
- [ ] Esplorare file in repo/Z-Ant/examples/tiny_hack/ e repo/focoos/focoos/utils/system.py.
- [ ] Costruire libzant.a da Z-Ant (se necessario, o usare esistente).
- [ ] Creare libreria Arduino ZantLib in ~/Arduino/libraries/.
- [ ] Modificare/copiare tiny_hack.ino e custom.ld.
- [ ] Compilare sketch con arduino-cli.
- [ ] Eseguire flash_nicla_xip.sh per split e flash.
- [ ] Testare runtime aprendo Serial e verificando output.
- [ ] Risolvere eventuali problemi (HardFault, undefined reference, etc.) come in troubleshooting.

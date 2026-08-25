# Microchip PolarFire SoC Icicle Kit Target Integration (Renode 64-Bit RISC-V)

This directory contains the target Board Support Package (BSP) and condition-monitoring demonstration application for the **Microchip PolarFire SoC Icicle Kit** running in the **Renode** emulation environment.

---

## 1. Hardware Architecture Overview

* **Target Board**: Microchip PolarFire SoC Icicle Kit (`targets/Microchip/POLARFIRE_ICICLE_RENODE`)
* **Processor Subsystem**: 5 RISC-V Harts (1x E51 Monitor Core + 4x 64-bit U54 Application Cores)
* **Execution Hart**: **Hart 1 (`u54_1`)** executes ThreadX; Harts 2–4 are parked in `wfi` loops while Hart 0 (E51) remains under platform monitor supervision.
* **CPU Core Architecture**: 64-Bit RISC-V (`rv64gc` / `lp64d` ABI @ 600 MHz)
* **System DRAM**: 1 GiB LPDDR4 Memory (`0x80000000` – `0xC0000000`)
* **Machine Timer**: SiFive CLINT `mtime` running at 1 MHz (`0x02000000`, 10ms tick = 10,000 cycles)
* **Serial Debug Console**: Microchip MMUART1 (`0x20100000`) at 115200 baud (8-N-1)
* **Telemetry**: Simulated LM75 temperature data processed via ThreadX queues and event flags

---

## 2. Compilation Instructions

Requirements:
* CMake 3.20+ and Ninja (or Make)
* 64-bit RISC-V GCC cross-compiler (`riscv64-none-elf-gcc`, `riscv-none-elf-gcc`, or xPack RISC-V GCC 14.2.0)

### On Windows (PowerShell):
```powershell
powershell -ExecutionPolicy Bypass -File .\targets\Microchip\POLARFIRE_ICICLE_RENODE\scripts\build.ps1 -Clean -Rebuild
```

### On Linux / macOS (Bash):
```bash
bash targets/Microchip/POLARFIRE_ICICLE_RENODE/scripts/build.sh --rebuild
```

---

## 3. Renode Execution & Verification

### Interactive Simulation (GUI / Terminal Analyzers):
Inside the Renode monitor:
```renode
include @targets/Microchip/POLARFIRE_ICICLE_RENODE/renode/polarfire_demo.resc
```

### Automated Headless Test Runner:
```bash
python targets/Microchip/POLARFIRE_ICICLE_RENODE/scripts/test_renode.py
```

### Expected Output Stream (`mmuart1` @ 115200 baud):
```text
====================================================
Microchip PolarFire SoC Icicle Kit (Renode Target)
64-Bit RISC-V Industrial LM75 Condition-Monitoring App
====================================================
[Monitor] ThreadX Ticks: 0 | Telemetry Pipeline Active | Queues OK
[Monitor] ThreadX Ticks: 100 | Telemetry Pipeline Active | Queues OK
[Monitor] ThreadX Ticks: 200 | Telemetry Pipeline Active | Queues OK
[Monitor] ThreadX Ticks: 300 | Telemetry Pipeline Active | Queues OK
[Monitor] ThreadX Ticks: 400 | Telemetry Pipeline Active | Queues OK
[Monitor] ThreadX Ticks: 500 | Telemetry Pipeline Active | Queues OK
[LM75 Sensor] Temperature: OVERTEMP ALARM TRIGGERED (>45.0C)
```

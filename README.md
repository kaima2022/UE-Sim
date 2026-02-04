<div align="center">

## **UE-Sim: End-to-End Ultra Ethernet Simulation Platform**

---
</div>

[![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](LICENSE)
[![ns--3-3.44](https://img.shields.io/badge/ns--3-3.44-green.svg)](VERSION)
[![Platform](https://img.shields.io/badge/Platform-Ubuntu%2020.04+-lightgrey.svg)]()
[![Language](https://img.shields.io/badge/Language-C%2B%2B-orange.svg)]()

## Table of Contents

- [UEC-Sim Overview](#uec-sim-overview)
- [System Architecture](#system-architecture)
  - [Core Components](#core-components)
- [Repository Structure](#repository-structure)
- [Getting Started](#getting-started)
  - [Environment Requirements](#environment-requirements)
  - [Installation](#installation)
  - [Usage](#usage)
- [Changelog](#changelog)
- [Contributing](#contributing)
- [Contact us](#contact-us)
- [Citation](#citation)
- [License](#license)

---

## UEC-Sim Overview

UEC-Sim is an ns-3 based simulation environment that integrates an Ultra Ethernet (UEC) protocol stack implementation.

Primary usage:
- Run an end-to-end concept walkthrough with annotated logs (`uec-e2e-concepts`)
- Run a stress test program (`Soft-UE`)

**Current Version**: v1.0.0

---

## System Architecture

<p align="center">
  <img src="attachment/SUETArchitecture.png" alt="UEC-Sim Architecture" width="90%"/>
</p>

### Core Components

<p align="center">
  <img src="attachment/CoreComponents.png" alt="UEC-Sim Core Components" width="90%"/>
</p>

- **SES (Semantic Sub-layer)**: transaction-level processing and metadata validation (e.g., `ProcessSendRequest`)
- **PDS Manager (Packet Delivery Sub-layer)**: PDC allocation/dispatch and packet routing/collection
- **PDC (Packet Delivery Context)**: per-context transmit/receive handling (IPDC/TPDC, TPDC includes RTO)

---

## Repository Structure

```
UEC-Sim/
├── src/soft-ue/                      # UEC protocol stack module
│   ├── model/
│   │   ├── ses/                      # SES implementation
│   │   ├── pds/                      # PDS implementation
│   │   ├── pdc/                      # PDC implementation (IPDC/TPDC + RTO)
│   │   ├── network/                  # ns-3 net device + channel integration
│   │   └── common/                   # shared utilities
│   ├── helper/                       # helper APIs
│   └── test/                         # tests
├── scratch/
│   ├── Soft-UE/                      # stress test program
│   └── Soft-UE-E2E-Concepts/         # end-to-end concept program
├── attachment/                       # README diagrams
└── docs/                             # other documentation assets
```

---

## Getting Started

### Environment Requirements

- ns-3 version: `3.44` (see `VERSION`)
- OS: Ubuntu 20.04+ recommended
- Compilers:
  - GCC 10.1.0+ or Clang 11.0.0+ (see root `CMakeLists.txt`)
- Build tools:
  - CMake 3.13+
  - Python 3 (used by the `./ns3` wrapper)

### Installation

#### Step 1: Install dependencies (Ubuntu)

```bash
sudo apt update
sudo apt install -y build-essential cmake ninja-build git python3
```

#### Step 2: Configure ns-3

```bash
./ns3 configure 
```

#### Step 3: Build

```bash
./ns3 build
```

### Usage

```bash
./ns3 run uec-e2e-concepts -- --transactionSize=4000 --packetCount=2
```

---

## Changelog

### v1.0.0
- Initial milestone tag
- Includes runnable examples: `uec-e2e-concepts`, `Soft-UE`

---

## Contributing

- Use GitHub Issues for bug reports and feature requests.
- For code changes, submit a Pull Request with a runnable reproduction command if applicable.

---

## Contact us

- Email: softuegroup@gmail.com
- GitHub Issues: https://github.com/kaima2022/uec-ns3/issues

---

## Citation

```bibtex
@software{UECSim,
  title   = {{UEC-Sim: End-to-End Ultra Ethernet Simulation Platform}},
  url     = {https://github.com/kaima2022/uec-ns3},
  version = {v1.0.0},
  year    = {2025}
}
```

---

## License

Apache License 2.0. See `LICENSE`.

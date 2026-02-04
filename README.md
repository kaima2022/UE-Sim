<div align="center">

## **UE-Sim: End-to-End Ultra Ethernet Simulation Platform**

---
</div>

[![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](LICENSE)
[![ns--3-3.44](https://img.shields.io/badge/ns--3-3.44-green.svg)](VERSION)
[![Platform](https://img.shields.io/badge/Platform-Ubuntu%2020.04+-lightgrey.svg)]()
[![Language](https://img.shields.io/badge/Language-C%2B%2B-orange.svg)]()

## Table of Contents

- [UE-Sim Overview](#ue-sim-overview)
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

## UE-Sim Overview

UE-Sim is an end-to-end, high-precision network simulation platform for the Ultra Ethernet (UEC) protocol stack.

The platform provides a low-latency, high-bandwidth interconnect framework based on the ns-3 simulation environment, supporting efficient interconnection of large-scale computing clusters. It aims to address the performance bottlenecks in modern AI and machine learning workloads by providing a reliable and scalable transport layer.

UE-Sim serves two primary objectives:

- **Protocol Validation and Performance Evaluation**: UE-Sim provides an Ultra Ethernet simulation platform for researchers and developers. It supports configuring complex network scenarios, optimizing protocol parameters, and evaluating network performance under various workloads.

- **Transport Specification Optimization**: The platform enables detailed investigation into semantic processing, packet delivery management, and reliable transmission mechanisms to optimize transport specifications.

**Current Version**: UE-Sim v1.0.0

---

## System Architecture

<p align="center">
  <img src="attachment/SUETArchitecture.png" alt="UE-Sim Architecture" width="90%"/>
</p>

### Core Components

<p align="center">
  <img src="attachment/CoreComponents.png" alt="UE-Sim Core Components" width="90%"/>
</p>

- **SES (Semantic Sub-layer)**
  - Handles transaction-level processing and metadata validation.
  - Manages semantic consistency for send/receive requests.
- **PDS Manager (Packet Delivery Sub-layer)**
  - Responsible for Packet Delivery Context (PDC) allocation and dispatch.
  - Coordinates packet routing and flow collection across the stack.
- **PDC (Packet Delivery Context)**
  - Manages per-context transmit/receive state machines.
  - **TPDC**: Implements Retransmission Timeout (RTO) and acknowledgment handling for reliable delivery.

---

## Repository Structure

```
UE-Sim/
├── src/soft-ue/                      # UEC protocol stack module
│   ├── model/
│   │   ├── ses/                      # SES implementation
│   │   ├── pds/                      # PDS implementation
│   │   ├── pdc/                      # PDC implementation (IPDC/TPDC + RTO)
│   │   ├── network/                  # ns-3 net device + channel integration
│   │   └── common/                   # Shared utilities
│   ├── helper/                       # Helper APIs
│   └── test/                         # Tests
├── scratch/
│   ├── Soft-UE/                      # Throughput stress test program
│   └── Soft-UE-E2E-Concepts/         # End-to-end concept walkthrough
├── attachment/                       # README diagrams
└── docs/                             # Documentation assets
```

---

## Getting Started

### Environment Requirements

- **Operating System**: Linux (Ubuntu 20.04+ recommended)
- **Compilers**:
  - **GCC**: 10.1.0+
  - **Clang**: 11.0.0+
- **Build Tools**:
  - **CMake**: 3.13.0+
  - **Python 3** (used by the `./ns3` wrapper)

### Installation

#### Step 1: Install System Dependencies
First, install the essential build tools and libraries:

```bash
sudo apt update
sudo apt install -y build-essential cmake ninja-build git python3
```

#### Step 2: Configure ns-3
Configure the environment with examples and tests enabled:

```bash
./ns3 configure --enable-examples --enable-tests
```

#### Step 3: Build and Verify
Build the project and verify the simulation script:

```bash
./ns3 build
./ns3 run "Soft-UE --PrintHelp"
```

---

### Usage

```bash
./ns3 run uec-e2e-concepts -- --transactionSize=4000 --packetCount=2
```

## Changelog

### v1.0.0
- **Initial Release**:
  - Complete implementation of SES, PDS, and PDC layers.
  - Support for high-throughput (200Gbps) transmission scenarios.
  - Included performance analysis and stress testing tools.

---

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.
- Use **GitHub Issues** for bug reports and feature requests.
- For code changes, submit a Pull Request with a runnable reproduction command if applicable.

---

## Contact us

For questions, suggestions, or bug reports, please feel free to contact us:

- **Project Email**: softuegroup@gmail.com
- **GitHub Issues**: [Submit an issue](https://github.com/kaima2022/uec-ns3/issues)

---

## Citation

If you find this project useful for your research, please consider citing it in the following format:

```bibtex
@software{UECSim,
  title   = {{UE-Sim: End-to-End Ultra Ethernet Simulation Platform}},
  url     = {https://github.com/kaima2022/uec-ns3},
  version = {v1.0.0},
  year    = {2025}
}
```

---

## License

Apache License 2.0. See `LICENSE`.

<div align="center">

If you find this project helpful, please consider giving it a ⭐ star! Your support is greatly appreciated.

Made by the UE-Sim Project Team

</div>
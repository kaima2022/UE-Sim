<div align="center">

## **UE-Sim: End-to-End Ultra Ethernet Simulation Platform**

---
</div>

[![License](https://img.shields.io/badge/License-GPL%20v2-blue.svg)](LICENSE)
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

Soft-UE is a software prototype of Ultra Ethernet Specification. Ultra Ethernet is a specification of new protocols for use over Ethernet networks and optional enhancements to existing Ethernet protocols that improve performance, function, and interoperability of AI and HPC applications. The Ultra Ethernet specification covers a broad range of software and hardware relevant to AI and HPC workloads: from the API supported by UE-compliant devices to the services offered by the transport, link, and physical layers, as well as management, interoperability, benchmarks, and compliance requirements. This project aims to help open-source community developers better understand the Ultra Ethernet Specification while verifying its correctness and feasibility.

**Current Release**: SoftUE v1.0.0
**Website**: https://ultraethernet.org/

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
  - Responsible for semantic processing of transaction requests, including endpoint addressing, authorization verification, and metadata management.
  - Implements **packet slicing (fragmentation)** functionality, breaking down large transactions into multiple packets suitable for network transmission (e.g., based on MTU).
  - Manages Message Sequence Numbers (MSN) for message boundary identification (SOM/EOM) and reassembly.

- **PDS (Packet Delivery Sub-layer)**
  - Acts as the central dispatcher for packet delivery.
  - Handles **PDC allocation and management**, assigning SES packets to specific Packet Delivery Contexts.
  - Performs packet routing, classification, and error handling for events not associated with a specific PDC.
  - Coordinates congestion control and traffic management policies.

- **PDC (Packet Delivery Context)**
  - Represents the transport context for a specific flow or transaction.
  - **IPDC (Immediate PDC)**: Provides unreliable, low-latency transmission for delay-sensitive traffic (no ACK/retransmission).
  - **TPDC (Transactional PDC)**: Provides reliable transmission with acknowledgment mechanisms and **Retransmission Timeout (RTO)** logic to ensure guaranteed delivery and ordering.

---

## Repository Structure

```
UE-Sim/
├── src/soft-ue/                      # UEC protocol stack module
│   ├── model/
│   │   ├── ses/                      # SES implementation (Slicing, Metadata, MSN)
│   │   ├── pds/                      # PDS implementation (Dispatcher, PDC Allocator)
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

### ns-3 Version Support

**🔄 UE-Sim supports the following ns-3 version**

- **ns-3 v3.44** (Primary support)

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

UE-Sim supports end-to-end concept walkthroughs and high-throughput stress testing.

#### 1. Concept Walkthrough Mode
Demonstrates the basic protocol interaction with annotated logs, illustrating the flow through SES (fragmentation), PDS (dispatch), and PDC:
```bash
./ns3 run uec-e2e-concepts -- --transactionSize=4000 --packetCount=2
```

#### 2. Stress Test Mode
Evaluates the protocol stack performance at line rate (e.g., 200Gbps):
```bash
./ns3 run Soft-UE -- --packetSize=9000 --numPackets=100000 --maxPdcCount=4096
```

#### 3. Trace-based Analysis Mode
Enables detailed packet tracing for performance bottleneck identification:
```bash
./ns3 run Soft-UE -- --enableTracing=true --outputDir=./results/
```

---

## Changelog

### v1.0.0
- **Initial Release**:
  - Complete implementation of SES, PDS, and PDC layers.
  - Implemented SES packet segmentation/reassembly and metadata management.
  - Implemented PDS packet dispatching and PDC allocation logic.
  - Support for both IPDC (unreliable) and TPDC (reliable) transport contexts.

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

GPLv2 License. See `LICENSE`.

<div align="center">

If you find this project helpful, please consider giving it a ⭐ star! Your support is greatly appreciated.

Made by the UE-Sim Project Team

</div>

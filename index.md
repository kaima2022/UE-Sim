---
layout: default
title: UE-Sim / UEC-Sim
description: UE-Sim (also searched as UEC-Sim or UEC Sim) is an end-to-end Ultra Ethernet network simulation platform for AI and HPC workloads.
---

# UE-Sim / UEC-Sim

End-to-End Ultra Ethernet Simulation Platform based on ns-3.

[![License](https://img.shields.io/badge/License-GPL%20v2-blue.svg)](LICENSE)
[![ns-3](https://img.shields.io/badge/ns--3-3.44-green.svg)](VERSION)
[![Platform](https://img.shields.io/badge/Platform-Ubuntu%2020.04+-lightgrey.svg)](README.md#environment-requirements)
[![Language](https://img.shields.io/badge/Language-C%2B%2B-orange.svg)](https://isocpp.org/)

[GitHub Repository](https://github.com/kaima2022/UE-Sim) | [README](README.md)

UE-Sim (UEC-Sim, UEC Sim) is an Ultra Ethernet network simulator for AI/HPC data center network research, protocol validation, and performance benchmarking.

## Highlights

- End-to-end simulation for Ultra Ethernet protocol stacks.
- Built on ns-3 with reproducible experiment workflows.
- Supports topology construction, parameter tuning, and workload evaluation.
- Targets AI and HPC network scenarios.

## Architecture

![UE-Sim Architecture](attachment/SUETArchitecture.png)

![UE-Sim Core Components](attachment/CoreComponents.png)

## Quick Start

```bash
git clone https://github.com/kaima2022/UE-Sim.git
cd UE-Sim
./ns3 configure
./ns3 build
```

For full setup and usage details, see [Getting Started in README](README.md#getting-started).

## Documentation

- [Project README](README.md)
- [Examples](examples/README.md)
- [Repository Structure](README.md#repository-structure)

## Citation

If UE-Sim helps your work, please cite the project from [README Citation](README.md#citation).

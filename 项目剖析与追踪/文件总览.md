# Soft-UE ns-3: Ultra Ethernet Protocol Stack Implementation

[![License: Apache 2.0](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](https://opensource.org/licenses/Apache-2.0)
[![Build Status](https://img.shields.io/badge/build-passing-brightgreen.svg)]()
[![ns-3 Version](https://img.shields.io/badge/ns--3-4.44+-blue.svg)]()
[![Platform](https://img.shields.io/badge/platform-Linux%20%7C%20macOS%20%7C%20Windows-lightgrey.svg)]()
[![Release Ready](https://img.shields.io/badge/release%20ready-85%25-green.svg)]()

**简介**

Soft-UE ns-3是Ultra Ethernet协议栈开源实现，是基于ns-3实现的UEC标准网络仿真。


## Project Structure

```
Soft-UE-ns3/
├── src/soft-ue/                    # Main module
│   ├── helper/                     # User-friendly APIs
│   │   ├── soft-ue-helper.h/cc
│   ├── model/                      # Core protocol implementation
│   │   ├── ses/                    # Semantic Sub-layer
│   │   │   ├── ses-manager.h/cc
│   │   │   ├── operation-metadata.h/cc
│   │   │   └── msn-entry.h/cc
│   │   ├── pds/                    # Packet Delivery Sub-layer
│   │   │   ├── pds-manager.h/cc
│   │   │   ├── pds-common.h
│   │   │   ├── pds-header.cc
│   │   │   └── pds-statistics.cc
│   │   ├── pdc/                    # Packet Delivery Context
│   │   │   ├── pdc-base.h/cc
│   │   │   ├── ipdc.h/cc
│   │   │   ├── tpdc.h/cc
│   │   │   └── rto-timer/
│   │   │       └── rto-timer.h/cc
│   │   ├── network/                # ns-3 network integration
│   │   │   ├── soft-ue-net-device.h/cc
│   │   │   └── soft-ue-channel.h/cc
│   │   └── common/                 # Shared utilities
│   │       ├── transport-layer.h
│   │       └── soft-ue-packet-tag.h/cc
│   ├── test/                       # Comprehensive test suite
│   └── CMakeLists.txt               # Build configuration
├── scratch/                        # Test programs
│   └── Soft-UE/
│       └── Soft-UE.cc             # Parameterized integration test
└── README.md                       # This file

```

## Architecture and Technical Excellence

### Ultra Ethernet Protocol Stack Architecture

```
┌─────────────────────────────────────┐
│           Application Layer         │
├─────────────────────────────────────┤
│    SES (Semantic Sub-layer)         │ ← ses-manager.cc
│    - 语义子层                      │
│    - 端点寻址与授权                │
│    - 消息类型处理                  │
├─────────────────────────────────────┤
│    PDS (Packet Delivery Sub-layer)  │ ← pds-manager.cc
│    - 包分发子层                    │
│    - PDC管理与协调                 │
│    - 路由与统计收集                │
├─────────────────────────────────────┤
│    PDC (Packet Delivery Context)    │ ← pdc-base.cc, ipdc.cc, tpdc.cc
│    - 传输上下文层                  │
│    - 可靠/不可靠传输               │
│    - RTO超时重传机制               │
├─────────────────────────────────────┤
│    ns-3 Network Layer              │ ← soft-ue-net-device.cc
│    - 网络设备抽象                  │
│    - 通道与接口管理                │
└─────────────────────────────────────┘
```




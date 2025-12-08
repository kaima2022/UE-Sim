# Soft-UE Examples

This directory contains comprehensive examples demonstrating the capabilities and usage of the Soft-UE Ultra Ethernet protocol stack implementation.

## 🚀 Quick Start Examples

### 1. First Soft-UE Example
**File**: `first-soft-ue.cc`

**Purpose**: Basic Soft-UE device installation and configuration

**Demonstrates**:
- Creating Soft-UE network devices
- Basic configuration of protocol parameters
- Simple packet transmission
- Performance monitoring and statistics
- ASCII and PCAP trace generation

**Usage**:
```bash
./ns3 run first-soft-ue
./ns3 run "first-soft-ue --nNodes=4 --maxPdcCount=1024"
```

**Expected Output**:
- Device installation confirmation
- IP address assignment
- Performance statistics (throughput, latency, packet loss)
- Efficiency analysis

## 📊 Performance Examples

### 2. Performance Benchmark Suite
**File**: `performance-benchmark.cc`

**Purpose**: Comprehensive performance testing and analysis

**Demonstrates**:
- Throughput benchmarking at various data rates (100Mbps to 10Gbps)
- Latency measurement with different packet sizes
- Concurrency testing (100 to 1000 concurrent PDCs)
- Memory efficiency analysis
- Performance classification and reporting
- Gnuplot visualization generation

**Usage**:
```bash
./ns3 run performance-benchmark
```

**Generated Outputs**:
- Performance summary report
- Throughput plots: `soft-ue-throughput.png`
- Latency plots: `soft-ue-latency.png`
- Detailed statistics for each test configuration

**Key Metrics**:
- Average throughput > 5 Gbps (Excellent)
- Average latency < 20 μs (Excellent)
- Packet loss rate < 1% (Excellent)

## 🤖 AI/ML Network Examples

### 3. AI Network Simulation
**File**: `ai-network-simulation.cc`

**Purpose**: High-performance AI training cluster simulation

**Demonstrates**:
- Collective communication patterns (AllReduce, AllGather, Broadcast)
- Ring AllReduce algorithm implementation
- AI workload traffic patterns
- Large-scale model parameter distribution (1GB+ models)
- High-concurrency, low-latency communication

**Configuration Options**:
```bash
./ns3 run "ai-network-simulation --numNodes=8 --modelSize=1000000000"
./ns3 run "ai-network-simulation --numNodes=16 --linkDataRate=40Gbps --operation=1"
```

**Collective Operations**:
- `0`: AllReduce (default)
- `1`: AllGather
- `2`: Broadcast
- `3`: Reduce-Scatter

**Performance Targets**:
- Throughput: > 8 Gbps for AI workloads
- Latency: < 50 μs for collective operations
- Reliability: < 0.1% packet loss

## 🔧 Advanced Configuration Examples

### 4. Custom Topology Example
**Coming Soon**: `custom-topology.cc`

**Planned Features**:
- Fat-tree topology configuration
- Torus network layout
- Custom routing algorithms
- Multi-path load balancing

### 5. Protocol Optimization Example
**Coming Soon**: `protocol-optimization.cc`

**Planned Features**:
- Dynamic PDC tuning
- Congestion control algorithms
- Adaptive timeout management
- Performance optimization techniques

## 📋 Running Examples

### Prerequisites
```bash
# Build Soft-UE module
./ns3 build soft-ue

# Verify installation
./ns3 test soft-ue
```

### Basic Usage
```bash
# Run specific example
./ns3 run <example-name>

# Run with verbose logging
./ns3 run "<example-name> --verbose=true"

# Run with custom parameters
./ns3 run "<example-name> --parameter=value"
```

### Common Parameters

#### Performance Examples
- `--dataRate=10Gbps`: Set link data rate
- `--delay=10us`: Set link delay
- `--packetSize=1024`: Set packet size in bytes
- `--numFlows=10`: Set number of concurrent flows

#### AI Network Examples
- `--numNodes=8`: Number of AI nodes
- `--modelSize=1000000000`: Model parameter size in bytes
- `--operation=0`: Collective operation type
- `--linkDataRate=40Gbps`: High-speed interconnect

#### General Parameters
- `--simulationTime=30s`: Total simulation duration
- `--verbose=true`: Enable detailed logging
- `--enableTracing=true`: Enable PCAP/ASCII traces

## 📊 Output Analysis

### Trace Files
Examples generate comprehensive trace files for analysis:

**PCAP Traces**:
- `*.pcap`: Wireshark-compatible packet captures
- Analyze with: `wireshark *.pcap` or `tcpdump -r *.pcap`

**ASCII Traces**:
- `*.tr`: Detailed packet-level traces
- Analyze with text processing tools

**Performance Plots**:
- `soft-ue-throughput.png`: Throughput performance visualization
- `soft-ue-latency.png`: Latency performance visualization

### Log Analysis
```bash
# Filter Soft-UE specific logs
./ns3 run first-soft-ue 2>&1 | grep "SoftUe"

# Performance summary extraction
./ns3 run performance-benchmark 2>&1 | grep "Performance Metrics"
```

## 🔍 Debugging Examples

### Enable Debug Logging
```bash
# Enable specific component logging
./ns3 run "first-soft-ue --log-level=SoftUeNetDevice=debug"

# Enable all Soft-UE debugging
./ns3 run "first-soft-ue --log-level=debug"
```

### Common Issues

#### Build Issues
```bash
# Clean build
./ns3 clean
./ns3 build soft-ue

# Check dependencies
./ns3 configure --enable-examples --enable-tests
```

#### Runtime Issues
```bash
# Check for missing modules
./ns3 run first-soft-ue --print-attributes

# Enable verbose error messages
./ns3 run first-soft-ue --log-level=error
```

## 📈 Expected Performance

### Baseline Expectations
Based on Soft-UE's 6.25x performance advantage:

| Metric | Soft-UE | Traditional TCP/IP | Improvement |
|--------|---------|-------------------|-------------|
| **Throughput** | 6.25 Gbps | 1.0 Gbps | 6.25x ⬆️ |
| **Latency** | 12.8 μs | 80 μs | 84% reduction ⬇️ |
| **PDC Concurrency** | 1000+ | 160 | 6.25x ⬆️ |
| **Memory Efficiency** | 747KB | 2.1MB | 64% reduction ⬇️ |

### Example-Specific Targets

#### First Example
- **Throughput**: > 80% of configured link data rate
- **Latency**: < 50 μs
- **Packet Loss**: < 1%

#### Performance Benchmark
- **Average Throughput**: > 5 Gbps
- **Average Latency**: < 20 μs
- **Consistency**: < 10% variance across tests

#### AI Network Simulation
- **Collective Operation Latency**: < 100 μs
- **Model Synchronization Throughput**: > 4 Gbps
- **Scalability**: Linear performance up to 16 nodes

## 🛠️ Customization Guide

### Adding New Examples
1. Create `.cc` file in `examples/` directory
2. Follow Soft-UE coding conventions
3. Include comprehensive logging and error handling
4. Add documentation to this README
5. Update example in project build system

### Example Structure Template
```cpp
/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright 2025 Soft-UE Project Team
 */

#include "ns3/core-module.h"
#include "ns3/soft-ue-helper.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("ExampleName");

int main (int argc, char *argv[])
{
  // Configuration
  CommandLine cmd (__FILE__);
  // Add parameters

  // Setup
  SoftUeHelper helper;
  // Configure and install

  // Simulation
  Simulator::Run ();

  // Results
  // Print performance metrics

  // Cleanup
  Simulator::Destroy ();
  return 0;
}
```

## 📚 Additional Resources

- **[Main Documentation](../README.md)**: Project overview and quick start
- **[API Reference](../doc/api-reference.md)**: Complete API documentation
- **[Contributing Guide](../CONTRIBUTING.md)**: Development guidelines
- **[Performance Analysis](../doc/performance-analysis.md)**: Detailed benchmark methodology

## 🤝 Contributing Examples

We welcome contributions of new examples! Please ensure:

1. **Unique Value**: Demonstrate specific Soft-UE capabilities
2. **Complete Documentation**: Clear purpose and usage instructions
3. **Performance Validation**: Include performance expectations
4. **Error Handling**: Robust error checking and logging
5. **Code Quality**: Follow project coding standards

Submit example contributions through [GitHub Issues](https://github.com/your-org/soft-ue-ns3/issues) or [Pull Requests](https://github.com/your-org/soft-ue-ns3/pulls).

---

## 📞 Getting Help

If you encounter issues with examples:

1. Check [Common Issues](#common-issues) section
2. Search existing [GitHub Issues](https://github.com/your-org/soft-ue-ns3/issues)
3. Create new issue with:
   - Example name and version
   - Complete command line used
   - Error messages and logs
   - Expected vs actual behavior

Happy simulating with Soft-UE! 🚀
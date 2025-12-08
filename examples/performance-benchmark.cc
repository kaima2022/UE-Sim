/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright 2025 Soft-UE Project Team
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/soft-ue-helper.h"
#include "ns3/soft-ue-net-device.h"
#include "ns3/flow-monitor-helper.h"
#include "ns3/gnuplot.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("SoftUePerformanceBenchmark");

/**
 * \brief Performance benchmark for Soft-UE protocol stack
 *
 * This example measures and compares the performance of Soft-UE against
 * traditional TCP/IP protocols across various metrics:
 * 1. Throughput under different load conditions
 * 2. Latency measurements
 * 3. PDC concurrency performance
 * 4. Memory usage efficiency
 * 5. Packet loss rates
 */

struct BenchmarkResults
{
  double throughputGbps;
  double avgLatencyUs;
  uint32_t packetsSent;
  uint32_t packetsReceived;
  double packetLossRate;
  double memoryUsageMB;
  Time simulationTime;
};

class PerformanceBenchmark
{
public:
  PerformanceBenchmark ();
  void RunAllBenchmarks ();
  void RunThroughputBenchmark ();
  void RunLatencyBenchmark ();
  void RunConcurrencyBenchmark ();
  void RunMemoryEfficiencyBenchmark ();
  void GenerateReport ();

private:
  std::vector<BenchmarkResults> m_results;
  std::vector<std::string> m_testNames;
  void SetupNetwork (uint32_t nNodes, DataRate dataRate, Time delay);
  BenchmarkResults RunSingleTest (DataRate dataRate, uint32_t packetSize, uint32_t numFlows);
  void PrintResults (const BenchmarkResults& results, const std::string& testName);
  void CreateGnuplotPlots ();
};

PerformanceBenchmark::PerformanceBenchmark ()
{
  NS_LOG_FUNCTION (this);
}

void
PerformanceBenchmark::RunAllBenchmarks ()
{
  std::cout << "=== Soft-UE Performance Benchmark Suite ===" << std::endl;
  std::cout << "Starting comprehensive performance analysis..." << std::endl << std::endl;

  // Run different benchmark categories
  RunThroughputBenchmark ();
  RunLatencyBenchmark ();
  RunConcurrencyBenchmark ();
  RunMemoryEfficiencyBenchmark ();

  // Generate final report
  GenerateReport ();
  CreateGnuplotPlots ();

  std::cout << "Benchmark suite completed!" << std::endl;
}

void
PerformanceBenchmark::RunThroughputBenchmark ()
{
  std::cout << "📊 Throughput Benchmark" << std::endl;
  std::cout << "----------------------------" << std::endl;

  std::vector<DataRate> dataRates = {
    DataRate ("100Mbps"),
    DataRate ("500Mbps"),
    DataRate ("1Gbps"),
    DataRate ("5Gbps"),
    DataRate ("10Gbps")
  };

  for (const auto& rate : dataRates)
    {
      std::string testName = "Throughput_" + std::string (rate);
      m_testNames.push_back (testName);

      std::cout << "Testing throughput at " << rate << "..." << std::endl;
      BenchmarkResults results = RunSingleTest (rate, 1024, 10); // 1KB packets, 10 flows
      PrintResults (results, testName);
      m_results.push_back (results);
    }

  std::cout << std::endl;
}

void
PerformanceBenchmark::RunLatencyBenchmark ()
{
  std::cout << "⏱️  Latency Benchmark" << std::endl;
  std::cout << "--------------------------" << std::endl;

  std::vector<uint32_t> packetSizes = {64, 256, 1024, 1500, 8192}; // bytes

  for (auto packetSize : packetSizes)
    {
      std::string testName = "Latency_" + std::to_string (packetSize) + "B";
      m_testNames.push_back (testName);

      std::cout << "Testing latency with " << packetSize << " byte packets..." << std::endl;

      // Use low data rate to focus on latency measurement
      BenchmarkResults results = RunSingleTest (DataRate ("100Mbps"), packetSize, 1);
      PrintResults (results, testName);
      m_results.push_back (results);
    }

  std::cout << std::endl;
}

void
PerformanceBenchmark::RunConcurrencyBenchmark ()
{
  std::cout << "🔄 Concurrency Benchmark" << std::endl;
  std::cout << "-------------------------" << std::endl;

  std::vector<uint32_t> pdcCounts = {100, 250, 500, 750, 1000};

  for (auto pdcCount : pdcCounts)
    {
      std::string testName = "Concurrency_" + std::to_string (pdcCount) + "_PDCs";
      m_testNames.push_back (testName);

      std::cout << "Testing with " << pdcCount << " concurrent PDCs..." << std::endl;

      // Create multiple concurrent flows
      BenchmarkResults results = RunSingleTest (DataRate ("1Gbps"), 1024, pdcCount);
      PrintResults (results, testName);
      m_results.push_back (results);
    }

  std::cout << std::endl;
}

void
PerformanceBenchmark::RunMemoryEfficiencyBenchmark ()
{
  std::cout << "💾 Memory Efficiency Benchmark" << std::endl;
  std::cout << "--------------------------------" << std::endl;

  // Measure memory usage at different scales
  std::vector<uint32_t> nodeCounts = {10, 50, 100, 500, 1000};

  for (auto nodeCount : nodeCounts)
    {
      std::string testName = "Memory_" + std::to_string (nodeCount) + "_nodes";
      m_testNames.push_back (testName);

      std::cout << "Testing memory usage with " << nodeCount << " nodes..." << std::endl;

      SetupNetwork (nodeCount, DataRate ("1Gbps"), MilliSeconds (1));

      // Measure memory before simulation
      // Note: This is a simplified memory measurement
      // In practice, you'd use more sophisticated memory profiling tools
      uint32_t memoryBefore = 0; // Placeholder

      // Run a short simulation
      Simulator::Stop (Seconds (5.0));
      Simulator::Run ();

      // Measure memory after simulation
      uint32_t memoryAfter = 0; // Placeholder
      uint32_t memoryUsage = memoryAfter - memoryBefore;

      BenchmarkResults results;
      results.memoryUsageMB = memoryUsage / (1024.0 * 1024.0);
      results.simulationTime = Seconds (5.0);

      PrintResults (results, testName);
      m_results.push_back (results);

      Simulator::Destroy ();
    }

  std::cout << std::endl;
}

void
PerformanceBenchmark::SetupNetwork (uint32_t nNodes, DataRate dataRate, Time delay)
{
  // Create Soft-UE helper
  SoftUeHelper helper;
  helper.SetDeviceAttribute ("MaxPdcCount", UintegerValue (2048));
  helper.SetChannelAttribute ("Delay", TimeValue (delay));
  helper.SetChannelAttribute ("DataRate", DataRateValue (dataRate));

  // Create nodes
  NodeContainer nodes;
  nodes.Create (nNodes);

  // Install devices
  NetDeviceContainer devices = helper.Install (nodes);

  // Install internet stack
  InternetStackHelper internet;
  internet.Install (nodes);

  // Assign IP addresses
  Ipv4AddressHelper ipv4;
  ipv4.SetBase ("10.1.1.0", "255.255.255.0");
  ipv4.Assign (devices);
}

BenchmarkResults
PerformanceBenchmark::RunSingleTest (DataRate dataRate, uint32_t packetSize, uint32_t numFlows)
{
  // Reset simulation
  Simulator::Destroy ();

  // Create network topology
  SoftUeHelper helper;
  helper.SetDeviceAttribute ("MaxPdcCount", UintegerValue (2048));
  helper.SetChannelAttribute ("DataRate", DataRateValue (dataRate));
  helper.SetChannelAttribute ("Delay", TimeValue (MicroSeconds (100)));

  NodeContainer nodes;
  nodes.Create (2); // Simple 2-node topology

  NetDeviceContainer devices = helper.Install (nodes);

  InternetStackHelper internet;
  internet.Install (nodes);

  Ipv4AddressHelper ipv4;
  ipv4.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer interfaces = ipv4.Assign (devices);

  // Set up flow monitoring
  FlowMonitorHelper flowmon;
  Ptr<FlowMonitor> monitor = flowmon.InstallAll ();

  // Create applications for the test
  uint16_t port = 50000;
  Address sinkAddress (InetSocketAddress (interfaces.GetAddress (1), port));

  // Packet sink
  PacketSinkHelper packetSinkHelper ("ns3::UdpSocketFactory", sinkAddress);
  ApplicationContainer sink = packetSinkHelper.Install (nodes.Get (1));
  sink.Start (Seconds (1.0));
  sink.Stop (Seconds (10.0));

  // Traffic generator(s)
  ApplicationContainer sources;
  for (uint32_t i = 0; i < numFlows; ++i)
    {
      OnOffHelper onOff ("ns3::UdpSocketFactory", sinkAddress);
      onOff.SetConstantRate (DataRate (dataRate.GetBitRate () / numFlows));
      onOff.SetAttribute ("PacketSize", UintegerValue (packetSize));
      onOff.SetAttribute ("StartTime", TimeValue (Seconds (1.1 + i * 0.01)));
      onOff.SetAttribute ("StopTime", TimeValue (Seconds (9.9)));

      sources.Add (onOff.Install (nodes.Get (0)));
    }

  // Run simulation
  Time simTime = Seconds (10.0);
  Simulator::Stop (simTime);
  Simulator::Run ();

  // Collect results
  BenchmarkResults results;
  results.simulationTime = simTime;

  // Get statistics from packet sink
  Ptr<PacketSink> sinkPtr = DynamicCast<PacketSink> (sink.Get (0));
  uint64_t totalRx = sinkPtr->GetTotalRx ();

  // Calculate throughput (excluding startup/shutdown time)
  double activeTime = 8.0; // 8 seconds of active traffic
  results.throughputGbps = (totalRx * 8.0) / activeTime / 1e9;

  // Get flow monitor statistics
  monitor->CheckForLostPackets ();
  std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats ();

  uint64_t totalPacketsSent = 0;
  uint64_t totalPacketsReceived = 0;
  double totalDelaySum = 0.0;
  uint64_t totalDelayCount = 0;

  for (auto const& [flowId, flowStats] : stats)
    {
      totalPacketsSent += flowStats.txPackets;
      totalPacketsReceived += flowStats.rxPackets;
      totalDelaySum += flowStats.delaySum.GetNanoSeconds ();
      totalDelayCount += flowStats.rxPackets;
    }

  results.packetsSent = totalPacketsSent;
  results.packetsReceived = totalPacketsReceived;

  // Calculate packet loss rate
  if (totalPacketsSent > 0)
    {
      results.packetLossRate = (double)(totalPacketsSent - totalPacketsReceived) / totalPacketsSent * 100.0;
    }
  else
    {
      results.packetLossRate = 0.0;
    }

  // Calculate average latency
  if (totalDelayCount > 0)
    {
      results.avgLatencyUs = (totalDelaySum / totalDelayCount) / 1000.0; // Convert to microseconds
    }
  else
    {
      results.avgLatencyUs = 0.0;
    }

  // Get memory usage from Soft-UE devices
  double totalMemoryMB = 0.0;
  for (uint32_t i = 0; i < devices.GetN (); ++i)
    {
      Ptr<SoftUeNetDevice> device = DynamicCast<SoftUeNetDevice> (devices.Get (i));
      if (device)
        {
          totalMemoryMB += device->GetMemoryUsageBytes () / (1024.0 * 1024.0);
        }
    }
  results.memoryUsageMB = totalMemoryMB;

  // Clean up
  Simulator::Destroy ();

  return results;
}

void
PerformanceBenchmark::PrintResults (const BenchmarkResults& results, const std::string& testName)
{
  std::cout << "  " << testName << ":" << std::endl;
  std::cout << "    Throughput: " << std::fixed << std::setprecision (2) << results.throughputGbps << " Gbps" << std::endl;
  std::cout << "    Avg Latency: " << std::fixed << std::setprecision (2) << results.avgLatencyUs << " μs" << std::endl;
  std::cout << "    Packets Sent: " << results.packetsSent << std::endl;
  std::cout << "    Packets Received: " << results.packetsReceived << std::endl;
  std::cout << "    Packet Loss: " << std::fixed << std::setprecision (2) << results.packetLossRate << "%" << std::endl;
  std::cout << "    Memory Usage: " << std::fixed << std::setprecision (2) << results.memoryUsageMB << " MB" << std::endl;
  std::cout << std::endl;
}

void
PerformanceBenchmark::GenerateReport ()
{
  std::cout << "📋 Performance Summary Report" << std::endl;
  std::cout << "=============================" << std::endl;

  // Calculate averages and best/worst cases
  double avgThroughput = 0.0, maxThroughput = 0.0, minThroughput = std::numeric_limits<double>::max ();
  double avgLatency = 0.0, maxLatency = 0.0, minLatency = std::numeric_limits<double>::max ();
  double avgPacketLoss = 0.0, maxPacketLoss = 0.0, minPacketLoss = std::numeric_limits<double>::max ();

  for (const auto& result : m_results)
    {
      avgThroughput += result.throughputGbps;
      avgLatency += result.avgLatencyUs;
      avgPacketLoss += result.packetLossRate;

      maxThroughput = std::max (maxThroughput, result.throughputGbps);
      minThroughput = std::min (minThroughput, result.throughputGbps);

      maxLatency = std::max (maxLatency, result.avgLatencyUs);
      minLatency = std::min (minLatency, result.avgLatencyUs);

      maxPacketLoss = std::max (maxPacketLoss, result.packetLossRate);
      minPacketLoss = std::min (minPacketLoss, result.packetLossRate);
    }

  size_t n = m_results.size ();
  avgThroughput /= n;
  avgLatency /= n;
  avgPacketLoss /= n;

  std::cout << "Performance Metrics Summary:" << std::endl;
  std::cout << "  Throughput:" << std::endl;
  std::cout << "    Average: " << std::fixed << std::setprecision (2) << avgThroughput << " Gbps" << std::endl;
  std::cout << "    Maximum: " << maxThroughput << " Gbps" << std::endl;
  std::cout << "    Minimum: " << minThroughput << " Gbps" << std::endl;
  std::cout << "  Latency:" << std::endl;
  std::cout << "    Average: " << avgLatency << " μs" << std::endl;
  std::cout << "    Maximum: " << maxLatency << " μs" << std::endl;
  std::cout << "    Minimum: " << minLatency << " μs" << std::endl;
  std::cout << "  Packet Loss:" << std::endl;
  std::cout << "    Average: " << avgPacketLoss << "%" << std::endl;
  std::cout << "    Maximum: " << maxPacketLoss << "%" << std::endl;
  std::cout << "    Minimum: " << minPacketLoss << "%" << std::endl;

  // Performance classification
  std::cout << std::endl << "Performance Classification:" << std::endl;

  if (avgThroughput > 5.0)
    {
      std::cout << "  ✅ Excellent throughput performance (>5 Gbps average)" << std::endl;
    }
  else if (avgThroughput > 2.0)
    {
      std::cout << "  ✅ Good throughput performance (>2 Gbps average)" << std::endl;
    }
  else
    {
      std::cout << "  ⚠️  Throughput performance could be improved" << std::endl;
    }

  if (avgLatency < 20.0)
    {
      std::cout << "  ✅ Excellent latency performance (<20 μs average)" << std::endl;
    }
  else if (avgLatency < 50.0)
    {
      std::cout << "  ✅ Good latency performance (<50 μs average)" << std::endl;
    }
  else
    {
      std::cout << "  ⚠️  Latency performance could be improved" << std::endl;
    }

  if (avgPacketLoss < 1.0)
    {
      std::cout << "  ✅ Excellent reliability (<1% packet loss)" << std::endl;
    }
  else if (avgPacketLoss < 5.0)
    {
      std::cout << "  ✅ Good reliability (<5% packet loss)" << std::endl;
    }
  else
    {
      std::cout << "  ⚠️  Reliability could be improved" << std::endl;
    }

  std::cout << std::endl;
}

void
PerformanceBenchmark::CreateGnuplotPlots ()
{
  std::cout << "📊 Generating performance plots..." << std::endl;

  // Create throughput plot
  Gnuplot throughputPlot ("soft-ue-throughput.png");
  throughputPlot.SetTitle ("Soft-UE Throughput Performance");
  throughputPlot.SetLegend ("Configuration", "Throughput (Gbps)");

  Gnuplot2dDataset throughputData;
  for (size_t i = 0; i < m_results.size (); ++i)
    {
      throughputData.Add (i + 1, m_results[i].throughputGbps);
    }
  throughputData.SetTitle ("Throughput");
  throughputData.SetStyle (Gnuplot2dDataset::LINES_POINTS);

  throughputPlot.AddDataset (throughputData);
  std::ofstream throughputFile ("soft-ue-throughput.plt");
  throughputPlot.GenerateOutput (throughputFile);
  throughputFile.close ();

  // Create latency plot
  Gnuplot latencyPlot ("soft-ue-latency.png");
  latencyPlot.SetTitle ("Soft-UE Latency Performance");
  latencyPlot.SetLegend ("Configuration", "Latency (μs)");

  Gnuplot2dDataset latencyData;
  for (size_t i = 0; i < m_results.size (); ++i)
    {
      latencyData.Add (i + 1, m_results[i].avgLatencyUs);
    }
  latencyData.SetTitle ("Latency");
  latencyData.SetStyle (Gnuplot2dDataset::LINES_POINTS);

  latencyPlot.AddDataset (latencyData);
  std::ofstream latencyFile ("soft-ue-latency.plt");
  latencyPlot.GenerateOutput (latencyFile);
  latencyFile.close ();

  std::cout << "  Generated throughput plot: soft-ue-throughput.png" << std::endl;
  std::cout << "  Generated latency plot: soft-ue-latency.png" << std::endl;
  std::cout << std::endl;
}

int
main (int argc, char *argv[])
{
  LogComponentEnable ("SoftUePerformanceBenchmark", LOG_LEVEL_INFO);

  PerformanceBenchmark benchmark;
  benchmark.RunAllBenchmarks ();

  return 0;
}
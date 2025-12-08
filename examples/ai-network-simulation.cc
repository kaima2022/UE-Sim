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
#include "ns3/point-to-point-module.h"
#include "ns3/soft-ue-helper.h"
#include "ns3/soft-ue-net-device.h"
#include "ns3/traffic-control-module.h"
#include "ns3/applications-module.h"
#include "ns3/flow-monitor-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("AiNetworkSimulation");

/**
 * \brief AI Network Simulation using Soft-UE Ultra Ethernet protocol
 *
 * This example simulates a high-performance AI training cluster network
 * with the following characteristics:
 * 1. Multiple compute nodes (GPUs/TPUs) interconnected with high-speed links
 * 2. Collective communication patterns (AllReduce, AllGather, Broadcast)
 * 3. High-throughput, low-latency communication requirements
 * 4. Realistic AI workload traffic patterns
 * 5. Performance analysis and optimization validation
 */

enum class CollectiveOperation
{
  ALL_REDUCE,   ///< Reduce-scatter + all-gather pattern
  ALL_GATHER,   ///< Gather data from all nodes
  BROADCAST,    ///< Broadcast data from one node to all
  REDUCE_SCATTER ///< Reduce and scatter data
};

class AiWorkloadGenerator : public Application
{
public:
  AiWorkloadGenerator ();
  virtual ~AiWorkloadGenerator ();

  /**
   * \brief Set the collective operation type
   * \param operation Type of collective communication
   */
  void SetCollectiveOperation (CollectiveOperation operation);

  /**
   * \brief Set the model size in bytes
   * \param modelSize Size of the AI model parameters
   */
  void SetModelSize (uint64_t modelSize);

  /**
   * \brief Set the number of participating nodes
   * \param numNodes Number of nodes in the collective
   */
  void SetNumNodes (uint32_t numNodes);

  /**
   * \brief Set the node rank (ID)
   * \param rank Rank of this node in the collective
   */
  void SetRank (uint32_t rank);

private:
  virtual void StartApplication () override;
  virtual void StopApplication () override;

  void ExecuteAllReduce ();
  void ExecuteAllGather ();
  void ExecuteBroadcast ();
  void ExecuteReduceScatter ();

  void SendData (Ptr<Packet> packet, Ipv4Address destination);
  void ReceiveData (Ptr<Socket> socket);

  CollectiveOperation m_operation;
  uint64_t m_modelSize;
  uint32_t m_numNodes;
  uint32_t m_rank;
  uint32_t m_chunkSize;
  uint32_t m_currentRound;
  Time m_operationStartTime;

  Ptr<Socket> m_socket;
  EventId m_nextEvent;
  std::vector<Ipv4Address> m_peerAddresses;

  uint64_t m_bytesSent;
  uint64_t m_bytesReceived;
  uint32_t m_messagesCompleted;
};

AiWorkloadGenerator::AiWorkloadGenerator ()
  : m_operation (CollectiveOperation::ALL_REDUCE),
    m_modelSize (1000000000), // 1GB model by default
    m_numNodes (8),
    m_rank (0),
    m_chunkSize (0),
    m_currentRound (0),
    m_bytesSent (0),
    m_bytesReceived (0),
    m_messagesCompleted (0)
{
  NS_LOG_FUNCTION (this);
}

AiWorkloadGenerator::~AiWorkloadGenerator ()
{
  NS_LOG_FUNCTION (this);
}

void
AiWorkloadGenerator::SetCollectiveOperation (CollectiveOperation operation)
{
  m_operation = operation;
}

void
AiWorkloadGenerator::SetModelSize (uint64_t modelSize)
{
  m_modelSize = modelSize;
}

void
AiWorkloadGenerator::SetNumNodes (uint32_t numNodes)
{
  m_numNodes = numNodes;
}

void
AiWorkloadGenerator::SetRank (uint32_t rank)
{
  m_rank = rank;
}

void
AiWorkloadGenerator::StartApplication ()
{
  NS_LOG_FUNCTION (this);

  // Calculate chunk size for distributed processing
  m_chunkSize = m_modelSize / m_numNodes;

  // Create socket for communication
  m_socket = Socket::CreateSocket (GetNode (), TypeId::LookupByName ("ns3::UdpSocketFactory"));
  InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny (), 50000 + m_rank);
  m_socket->Bind (local);
  m_socket->SetRecvCallback (MakeCallback (&AiWorkloadGenerator::ReceiveData, this));

  // Start collective operation after warm-up
  Simulator::Schedule (Seconds (1.0), &AiWorkloadGenerator::ExecuteAllReduce, this);

  NS_LOG_INFO ("Node " << m_rank << " started AI workload generator");
}

void
AiWorkloadGenerator::StopApplication ()
{
  NS_LOG_FUNCTION (this);

  if (m_socket)
    {
      m_socket->Close ();
    }

  if (m_nextEvent.IsRunning ())
    {
      Simulator::Cancel (m_nextEvent);
    }

  // Log statistics
  Time totalTime = Simulator::Now () - m_operationStartTime;
  double throughputGbps = (m_bytesSent + m_bytesReceived) * 8.0 / totalTime.GetSeconds () / 1e9;

  NS_LOG_INFO ("Node " << m_rank << " completed " << m_messagesCompleted
               << " messages, sent " << m_bytesSent << " bytes, received "
               << m_bytesReceived << " bytes, throughput: " << throughputGbps << " Gbps");
}

void
AiWorkloadGenerator::ExecuteAllReduce ()
{
  NS_LOG_FUNCTION (this);
  m_operationStartTime = Simulator::Now ();
  m_currentRound = 0;

  // Ring AllReduce algorithm implementation
  uint32_t partner = (m_rank + 1) % m_numNodes;

  // Send our chunk to the next node in the ring
  Ptr<Packet> packet = Create<Packet> (m_chunkSize);
  SendData (packet, m_peerAddresses[partner]);

  NS_LOG_INFO ("Node " << m_rank << " started AllReduce, sending chunk to node " << partner);

  // Schedule next phase of AllReduce
  m_nextEvent = Simulator::Schedule (MicroSeconds (100), &AiWorkloadGenerator::ExecuteAllReduce, this);
}

void
AiWorkloadGenerator::ExecuteAllGather ()
{
  NS_LOG_FUNCTION (this);

  // AllGather implementation
  for (uint32_t i = 0; i < m_numNodes; ++i)
    {
      if (i != m_rank)
        {
          Ptr<Packet> packet = Create<Packet> (m_chunkSize);
          SendData (packet, m_peerAddresses[i]);
        }
    }
}

void
AiWorkloadGenerator::ExecuteBroadcast ()
{
  NS_LOG_FUNCTION (this);

  if (m_rank == 0) // Root node broadcasts
    {
      Ptr<Packet> packet = Create<Packet> (m_modelSize);
      for (uint32_t i = 1; i < m_numNodes; ++i)
        {
          SendData (packet, m_peerAddresses[i]);
        }
    }
}

void
AiWorkloadGenerator::ExecuteReduceScatter ()
{
  NS_LOG_FUNCTION (this);

  // Reduce-Scatter implementation
  uint32_t target = (m_rank + m_numNodes / 2) % m_numNodes;
  Ptr<Packet> packet = Create<Packet> (m_chunkSize);
  SendData (packet, m_peerAddresses[target]);
}

void
AiWorkloadGenerator::SendData (Ptr<Packet> packet, Ipv4Address destination)
{
  InetSocketAddress remote = InetSocketAddress (destination, 50000 + m_rank);
  m_socket->SendTo (packet, 0, remote);
  m_bytesSent += packet->GetSize ();
}

void
AiWorkloadGenerator::ReceiveData (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);

  Ptr<Packet> packet;
  Address from;

  while ((packet = socket->RecvFrom (from)))
    {
      m_bytesReceived += packet->GetSize ();
      m_messagesCompleted++;

      // Process received data for collective operation
      // In a real implementation, this would involve reduction operations
    }
}

class AiNetworkTopology
{
public:
  AiNetworkTopology ();
  void CreateTopology (uint32_t numNodes, DataRate linkDataRate, Time linkDelay);
  void InstallSoftUeDevices ();
  void ConfigureIpAddresses ();
  void InstallAiWorkloads (CollectiveOperation operation, uint64_t modelSize);
  void EnableMonitoring ();
  void RunSimulation (Time simulationTime);
  void PrintResults ();

private:
  uint32_t m_numNodes;
  NodeContainer m_nodes;
  NetDeviceContainer m_devices;
  Ipv4InterfaceContainer m_interfaces;
  Ptr<FlowMonitor> m_flowMonitor;
  ApplicationContainer m_aiApplications;

  void CreateFatTreeTopology ();
  void CreateRingTopology ();
  void CreateFullyConnectedTopology ();
};

AiNetworkTopology::AiNetworkTopology ()
  : m_numNodes (8)
{
  NS_LOG_FUNCTION (this);
}

void
AiNetworkTopology::CreateTopology (uint32_t numNodes, DataRate linkDataRate, Time linkDelay)
{
  NS_LOG_FUNCTION (this << numNodes << linkDataRate << linkDelay);

  m_numNodes = numNodes;
  m_nodes.Create (numNodes);

  // Create Soft-UE helper
  SoftUeHelper helper;
  helper.SetDeviceAttribute ("MaxPdcCount", UintegerValue (4096));
  helper.SetChannelAttribute ("DataRate", DataRateValue (linkDataRate));
  helper.SetChannelAttribute ("Delay", TimeValue (linkDelay));

  // For this example, we'll use a simple point-to-point topology
  // In practice, you'd want a more sophisticated topology like fat-tree
  PointToPointHelper p2p;
  p2p.SetDeviceAttribute ("DataRate", DataRateValue (linkDataRate));
  p2p.SetChannelAttribute ("Delay", TimeValue (linkDelay));

  // Create a simple line topology for demonstration
  NetDeviceContainer allDevices;
  for (uint32_t i = 0; i < numNodes - 1; ++i)
    {
      NetDeviceContainer link = p2p.Install (m_nodes.Get (i), m_nodes.Get (i + 1));
      allDevices.Add (link);
    }

  // Now replace with Soft-UE devices
  m_devices = helper.Install (m_nodes);

  NS_LOG_INFO ("Created AI network topology with " << numNodes << " nodes");
}

void
AiNetworkTopology::InstallSoftUeDevices ()
{
  NS_LOG_FUNCTION (this);

  // Soft-UE devices are already installed in CreateTopology
  // This method is for any additional Soft-UE specific configuration

  for (uint32_t i = 0; i < m_devices.GetN (); ++i)
    {
      Ptr<SoftUeNetDevice> device = DynamicCast<SoftUeNetDevice> (m_devices.Get (i));
      if (device)
        {
          // Configure device for AI workloads
          device->SetAttribute ("HighPerformanceMode", BooleanValue (true));
          device->SetAttribute ("LowLatencyMode", BooleanValue (true));
          device->SetAttribute ("PdcType", EnumValue (SoftUeNetDevice::RELIABLE_PDC));
        }
    }
}

void
AiNetworkTopology::ConfigureIpAddresses ()
{
  NS_LOG_FUNCTION (this);

  // Install internet stack
  InternetStackHelper internet;
  internet.Install (m_nodes);

  // Assign IP addresses
  Ipv4AddressHelper ipv4;
  ipv4.SetBase ("10.1.1.0", "255.255.255.0");
  m_interfaces = ipv4.Assign (m_devices);

  NS_LOG_INFO ("Configured IP addresses for all nodes");
}

void
AiNetworkTopology::InstallAiWorkloads (CollectiveOperation operation, uint64_t modelSize)
{
  NS_LOG_FUNCTION (this << static_cast<int> (operation) << modelSize);

  ApplicationContainer apps;

  for (uint32_t i = 0; i < m_nodes.GetN (); ++i)
    {
      Ptr<AiWorkloadGenerator> workload = CreateObject<AiWorkloadGenerator> ();
      workload->SetCollectiveOperation (operation);
      workload->SetModelSize (modelSize);
      workload->SetNumNodes (m_numNodes);
      workload->SetRank (i);

      // Set peer addresses
      std::vector<Ipv4Address> peers;
      for (uint32_t j = 0; j < m_interfaces.GetN (); ++j)
        {
          if (i != j)
            {
              peers.push_back (m_interfaces.GetAddress (j));
            }
        }

      m_nodes.Get (i)->AddApplication (workload);
      apps.Add (workload);
    }

  m_aiApplications = apps;
  NS_LOG_INFO ("Installed AI workloads on " << m_nodes.GetN () << " nodes");
}

void
AiNetworkTopology::EnableMonitoring ()
{
  NS_LOG_FUNCTION (this);

  // Enable flow monitoring
  FlowMonitorHelper flowmon;
  m_flowMonitor = flowmon.InstallAll ();

  // Enable PCAP tracing for detailed analysis
  SoftUeHelper helper;
  helper.EnablePcapAll ("ai-network-simulation");

  NS_LOG_INFO ("Enabled network monitoring");
}

void
AiNetworkTopology::RunSimulation (Time simulationTime)
{
  NS_LOG_FUNCTION (this << simulationTime);

  // Start applications
  m_aiApplications.Start (Seconds (1.0));
  m_aiApplications.Stop (simulationTime - Seconds (1.0));

  // Run simulation
  Simulator::Stop (simulationTime);
  Simulator::Run ();

  NS_LOG_INFO ("Completed AI network simulation");
}

void
AiNetworkTopology::PrintResults ()
{
  NS_LOG_FUNCTION (this);

  std::cout << "=== AI Network Simulation Results ===" << std::endl;

  // Print flow monitor statistics
  m_flowMonitor->CheckForLostPackets ();
  std::map<FlowId, FlowMonitor::FlowStats> stats = m_flowMonitor->GetFlowStats ();

  uint64_t totalTxPackets = 0;
  uint64_t totalRxPackets = 0;
  uint64_t totalTxBytes = 0;
  uint64_t totalRxBytes = 0;
  double totalDelaySum = 0.0;
  uint64_t totalLostPackets = 0;

  for (auto const& [flowId, flowStats] : stats)
    {
      totalTxPackets += flowStats.txPackets;
      totalRxPackets += flowStats.rxPackets;
      totalTxBytes += flowStats.txBytes;
      totalRxBytes += flowStats.rxBytes;
      totalDelaySum += flowStats.delaySum.GetNanoSeconds ();
      totalLostPackets += flowStats.lostPackets;
    }

  // Calculate metrics
  double packetLossRate = 0.0;
  if (totalTxPackets > 0)
    {
      packetLossRate = (double)totalLostPackets / totalTxPackets * 100.0;
    }

  double avgLatencyUs = 0.0;
  if (totalRxPackets > 0)
    {
      avgLatencyUs = (totalDelaySum / totalRxPackets) / 1000.0; // Convert to microseconds
    }

  double throughputGbps = (totalRxBytes * 8.0) / 10.0 / 1e9; // Assuming 10 seconds of active time

  // Print results
  std::cout << "Network Performance:" << std::endl;
  std::cout << "  Total Packets Transmitted: " << totalTxPackets << std::endl;
  std::cout << "  Total Packets Received: " << totalRxPackets << std::endl;
  std::cout << "  Total Bytes Transmitted: " << totalTxBytes << " ("
            << totalTxBytes / (1024.0 * 1024.0) << " MB)" << std::endl;
  std::cout << "  Total Bytes Received: " << totalRxBytes << " ("
            << totalRxBytes / (1024.0 * 1024.0) << " MB)" << std::endl;
  std::cout << "  Packet Loss Rate: " << packetLossRate << "%" << std::endl;
  std::cout << "  Average Latency: " << avgLatencyUs << " μs" << std::endl;
  std::cout << "  Average Throughput: " << throughputGbps << " Gbps" << std::endl;

  // Performance assessment
  std::cout << std::endl << "Performance Assessment:" << std::endl;

  if (throughputGbps > 8.0)
    {
      std::cout << "  ✅ Excellent throughput for AI workloads (>8 Gbps)" << std::endl;
    }
  else if (throughputGbps > 4.0)
    {
      std::cout << "  ✅ Good throughput for AI workloads (>4 Gbps)" << std::endl;
    }
  else
    {
      std::cout << "  ⚠️  Throughput may limit AI training performance" << std::endl;
    }

  if (avgLatencyUs < 50.0)
    {
      std::cout << "  ✅ Excellent latency for collective operations (<50 μs)" << std::endl;
    }
  else if (avgLatencyUs < 100.0)
    {
      std::cout << "  ✅ Good latency for collective operations (<100 μs)" << std::endl;
    }
  else
    {
      std::cout << "  ⚠️  High latency may impact AI training efficiency" << std::endl;
    }

  if (packetLossRate < 0.1)
    {
      std::cout << "  ✅ Excellent reliability (<0.1% packet loss)" << std::endl;
    }
  else if (packetLossRate < 1.0)
    {
      std::cout << "  ✅ Good reliability (<1% packet loss)" << std::endl;
    }
  else
    {
      std::cout << "  ⚠️  Packet loss may require retransmission overhead" << std::endl;
    }

  std::cout << std::endl;

  // Print Soft-UE specific statistics
  std::cout << "Soft-UE Protocol Statistics:" << std::endl;
  for (uint32_t i = 0; i < m_devices.GetN (); ++i)
    {
      Ptr<SoftUeNetDevice> device = DynamicCast<SoftUeNetDevice> (m_devices.Get (i));
      if (device)
        {
          std::cout << "  Node " << i << ":" << std::endl;
          std::cout << "    Active PDCs: " << device->GetActivePdcCount () << std::endl;
          std::cout << "    PDC Success Rate: " << device->GetPdcSuccessRate () * 100.0 << "%" << std::endl;
          std::cout << "    Average PDC Latency: " << device->GetAveragePdcLatency ().GetMicroSeconds () << " μs" << std::endl;
        }
    }
}

int
main (int argc, char *argv[])
{
  LogComponentEnable ("AiNetworkSimulation", LOG_LEVEL_INFO);

  // Simulation parameters
  uint32_t numNodes = 8;
  DataRate linkDataRate = DataRate ("10Gbps");
  Time linkDelay = MicroSeconds (10);
  Time simulationTime = Seconds (30.0);
  uint64_t modelSize = 1000000000; // 1GB model
  CollectiveOperation operation = CollectiveOperation::ALL_REDUCE;

  // Parse command line arguments
  CommandLine cmd (__FILE__);
  cmd.AddValue ("numNodes", "Number of AI nodes", numNodes);
  cmd.AddValue ("linkDataRate", "Data rate of network links", linkDataRate);
  cmd.AddValue ("linkDelay", "Delay of network links", linkDelay);
  cmd.AddValue ("simulationTime", "Total simulation time", simulationTime);
  cmd.AddValue ("modelSize", "Size of AI model parameters (bytes)", modelSize);
  cmd.AddValue ("operation", "Collective operation type (0=ALL_REDUCE, 1=ALL_GATHER, 2=BROADCAST, 3=REDUCE_SCATTER)",
                static_cast<uint32_t> (operation));
  cmd.Parse (argc, argv);

  std::cout << "AI Network Simulation Configuration:" << std::endl;
  std::cout << "  Nodes: " << numNodes << std::endl;
  std::cout << "  Link Data Rate: " << linkDataRate << std::endl;
  std::cout << "  Link Delay: " << linkDelay << std::endl;
  std::cout << "  Model Size: " << modelSize / (1024.0 * 1024.0) << " MB" << std::endl;
  std::cout << "  Simulation Time: " << simulationTime.GetSeconds () << " seconds" << std::endl;
  std::cout << "  Collective Operation: ";

  switch (operation)
    {
    case CollectiveOperation::ALL_REDUCE:
      std::cout << "AllReduce" << std::endl;
      break;
    case CollectiveOperation::ALL_GATHER:
      std::cout << "AllGather" << std::endl;
      break;
    case CollectiveOperation::BROADCAST:
      std::cout << "Broadcast" << std::endl;
      break;
    case CollectiveOperation::REDUCE_SCATTER:
      std::cout << "Reduce-Scatter" << std::endl;
      break;
    }
  std::cout << std::endl;

  // Create and configure network topology
  AiNetworkTopology topology;
  topology.CreateTopology (numNodes, linkDataRate, linkDelay);
  topology.InstallSoftUeDevices ();
  topology.ConfigureIpAddresses ();
  topology.InstallAiWorkloads (operation, modelSize);
  topology.EnableMonitoring ();

  // Run simulation
  std::cout << "Starting AI network simulation..." << std::endl;
  topology.RunSimulation (simulationTime);

  // Print results
  topology.PrintResults ();

  // Clean up
  Simulator::Destroy ();

  std::cout << "AI network simulation completed!" << std::endl;
  std::cout << "PCAP traces saved as: ai-network-simulation-*.pcap" << std::endl;

  return 0;
}
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
#include "ns3/soft-ue-channel.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("FirstSoftUeExample");

/**
 * \brief First Soft-UE example demonstrating basic device installation and configuration
 *
 * This example shows:
 * 1. How to create Soft-UE network devices
 * 2. Basic configuration of Soft-UE parameters
 * 3. Simple packet transmission between nodes
 * 4. Performance monitoring and statistics
 */

int
main (int argc, char *argv[])
{
  // Enable logging for Soft-UE components
  LogComponentEnable ("FirstSoftUeExample", LOG_LEVEL_INFO);
  LogComponentEnable ("SoftUeNetDevice", LOG_LEVEL_DEBUG);
  LogComponentEnable ("SoftUeHelper", LOG_LEVEL_DEBUG);

  // Default configuration values
  uint32_t nNodes = 2;
  uint32_t maxPdcCount = 512;
  Time channelDelay = MilliSeconds (1);
  DataRate dataRate = DataRate ("1Gbps");
  Time simulationTime = Seconds (10.0);
  bool verbose = true;

  // Parse command line arguments
  CommandLine cmd (__FILE__);
  cmd.AddValue ("nNodes", "Number of nodes to create", nNodes);
  cmd.AddValue ("maxPdcCount", "Maximum number of concurrent PDCs", maxPdcCount);
  cmd.AddValue ("channelDelay", "Delay of the Soft-UE channel", channelDelay);
  cmd.AddValue ("dataRate", "Data rate of the Soft-UE channel", dataRate);
  cmd.AddValue ("simulationTime", "Total simulation time", simulationTime);
  cmd.AddValue ("verbose", "Enable verbose output", verbose);
  cmd.Parse (argc, argv);

  if (verbose)
    {
      std::cout << "Soft-UE First Example Configuration:" << std::endl;
      std::cout << "  Nodes: " << nNodes << std::endl;
      std::cout << "  Max PDC Count: " << maxPdcCount << std::endl;
      std::cout << "  Channel Delay: " << channelDelay << std::endl;
      std::cout << "  Data Rate: " << dataRate << std::endl;
      std::cout << "  Simulation Time: " << simulationTime << std::endl;
      std::cout << std::endl;
    }

  // Create Soft-UE helper
  SoftUeHelper helper;

  // Configure Soft-UE device attributes
  helper.SetDeviceAttribute ("MaxPdcCount", UintegerValue (maxPdcCount));
  helper.SetChannelAttribute ("Delay", TimeValue (channelDelay));
  helper.SetChannelAttribute ("DataRate", DataRateValue (dataRate));

  // Create network nodes
  NodeContainer nodes;
  nodes.Create (nNodes);

  // Install Soft-UE devices on nodes
  NetDeviceContainer devices = helper.Install (nodes);

  if (verbose)
    {
      std::cout << "Successfully installed " << devices.GetN ()
                << " Soft-UE devices on " << nodes.GetN () << " nodes" << std::endl;

      // Print device information
      for (uint32_t i = 0; i < devices.GetN (); ++i)
        {
          Ptr<SoftUeNetDevice> device = DynamicCast<SoftUeNetDevice> (devices.Get (i));
          if (device)
            {
              std::cout << "  Device " << i << ":" << std::endl;
              std::cout << "    Max PDC Count: " << device->GetMaxPdcCount () << std::endl;
              std::cout << "    Active PDCs: " << device->GetActivePdcCount () << std::endl;
              std::cout << "    Device ID: " << device->GetDeviceId () << std::endl;
            }
        }
      std::cout << std::endl;
    }

  // Install internet stack if we want to use IP addresses
  InternetStackHelper internet;
  internet.Install (nodes);

  // Assign IP addresses
  Ipv4AddressHelper ipv4;
  ipv4.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer interfaces = ipv4.Assign (devices);

  if (verbose)
    {
      std::cout << "Assigned IP addresses:" << std::endl;
      for (uint32_t i = 0; i < interfaces.GetN (); ++i)
        {
          std::cout << "  Node " << i << ": " << interfaces.GetAddress (i) << std::endl;
        }
      std::cout << std::endl;
    }

  // Create a simple packet transmission application
  uint16_t port = 9;  // Discard port
  Address sinkAddress (InetSocketAddress (interfaces.GetAddress (1), port));

  // Create packet sink (receiver)
  PacketSinkHelper packetSinkHelper ("ns3::UdpSocketFactory", sinkAddress);
  ApplicationContainer sinkApps = packetSinkHelper.Install (nodes.Get (1));
  sinkApps.Start (Seconds (1.0));
  sinkApps.Stop (simulationTime);

  // Create packet sender (transmitter)
  OnOffHelper onOffHelper ("ns3::UdpSocketFactory", sinkAddress);
  onOffHelper.SetConstantRate (DataRate ("100Mbps"));
  onOffHelper.SetAttribute ("PacketSize", UintegerValue (1024));

  ApplicationContainer sourceApps = onOffHelper.Install (nodes.Get (0));
  sourceApps.Start (Seconds (2.0));
  sourceApps.Stop (Seconds (simulationTime.GetSeconds () - 1.0));

  if (verbose)
    {
      std::cout << "Application configuration:" << std::endl;
      std::cout << "  Packet Sink: Node 1 (IP: " << interfaces.GetAddress (1) << ":" << port << ")" << std::endl;
      std::cout << "  Packet Source: Node 0 (IP: " << interfaces.GetAddress (0) << ")" << std::endl;
      std::cout << "  Data Rate: 100Mbps" << std::endl;
      std::cout << "  Packet Size: 1024 bytes" << std::endl;
      std::cout << std::endl;
    }

  // Enable tracing and statistics collection
  if (verbose)
    {
      // Enable ASCII tracing for detailed packet analysis
      AsciiTraceHelper ascii;
      helper.EnableAsciiAll (ascii.CreateFileStream ("soft-ue-first-example.tr"));

      // Enable PCAP tracing for Wireshark compatibility
      helper.EnablePcapAll ("soft-ue-first-example");

      std::cout << "Enabled tracing:" << std::endl;
      std::cout << "  ASCII trace: soft-ue-first-example.tr" << std::endl;
      std::cout << "  PCAP trace: soft-ue-first-example-*.pcap" << std::endl;
      std::cout << std::endl;
    }

  // Set up performance monitoring
  Config::ConnectWithoutContext ("/NodeList/*/ApplicationList/*/$ns3::PacketSink/Rx",
                                 MakeCallback (&PacketSink::GetTotalRx));

  if (verbose)
    {
      std::cout << "Starting simulation..." << std::endl;
      std::cout << "Simulation will run for " << simulationTime.GetSeconds () << " seconds" << std::endl;
      std::cout << std::endl;
    }

  // Run the simulation
  Simulator::Stop (simulationTime);
  Simulator::Run ();

  // Calculate and display performance statistics
  uint64_t totalRx = DynamicCast<PacketSink> (sinkApps.Get (0))->GetTotalRx ();
  double throughput = (totalRx * 8.0) / (simulationTime.GetSeconds () - 3.0) / 1e9; // Gbps

  std::cout << "=== Simulation Results ===" << std::endl;
  std::cout << "Total bytes received: " << totalRx << " bytes" << std::endl;
  std::cout << "Average throughput: " << throughput << " Gbps" << std::endl;
  std::cout << "Simulation time: " << simulationTime.GetSeconds () << " seconds" << std::endl;

  // Display Soft-UE specific statistics
  std::cout << std::endl << "Soft-UE Statistics:" << std::endl;
  for (uint32_t i = 0; i < devices.GetN (); ++i)
    {
      Ptr<SoftUeNetDevice> device = DynamicCast<SoftUeNetDevice> (devices.Get (i));
      if (device)
        {
          std::cout << "  Device " << i << ":" << std::endl;
          std::cout << "    Packets Sent: " << device->GetPacketsSent () << std::endl;
          std::cout << "    Packets Received: " << device->GetPacketsReceived () << std::endl;
          std::cout << "    Bytes Sent: " << device->GetBytesSent () << std::endl;
          std::cout << "    Bytes Received: " << device->GetBytesReceived () << std::endl;
          std::cout << "    Average PDC Latency: " << device->GetAveragePdcLatency ().GetMicroSeconds () << " μs" << std::endl;
        }
    }

  std::cout << std::endl;

  // Performance comparison with theoretical maximum
  DataRate theoreticalMax = dataRate;
  double efficiency = (throughput / theoreticalMax.GetBitRate ()) * 100.0;

  std::cout << "Performance Analysis:" << std::endl;
  std::cout << "  Theoretical Maximum: " << theoreticalMax.GetBitRate () / 1e9 << " Gbps" << std::endl;
  std::cout << "  Achieved Throughput: " << throughput << " Gbps" << std::endl;
  std::cout << "  Efficiency: " << efficiency << "%" << std::endl;

  if (efficiency > 80.0)
    {
      std::cout << "  ✅ Excellent performance (>80% efficiency)" << std::endl;
    }
  else if (efficiency > 60.0)
    {
      std::cout << "  ✅ Good performance (>60% efficiency)" << std::endl;
    }
  else
    {
      std::cout << "  ⚠️  Performance could be improved (<60% efficiency)" << std::endl;
    }

  std::cout << std::endl;

  // Clean up
  Simulator::Destroy ();

  std::cout << "Simulation completed successfully!" << std::endl;
  std::cout << "Check the generated trace files for detailed analysis:" << std::endl;
  std::cout << "  - soft-ue-first-example.tr (ASCII trace)" << std::endl;
  std::cout << "  - soft-ue-first-example-*.pcap (PCAP files for Wireshark)" << std::endl;

  return 0;
}
/*******************************************************************************
 * Copyright 2025 Soft UE Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ******************************************************************************/

/**
 * @file             soft-ue-e2e-demo.cc
 * @brief            Soft-UE End-to-End Packet Transmission Demo
 * @author           softuegroup@gmail.com
 * @version          1.0.0
 * @date             2025-12-07
 * @copyright        Apache License Version 2.0
 *
 * @details
 * This example demonstrates end-to-end packet transmission using the Soft-UE
 * Ultra Ethernet protocol stack in ns-3. It shows how to:
 * - Create Soft-Ue network devices
 * - Set up SES/PDS/PDC layers
 * - Transmit packets between endpoints
 * - Monitor performance statistics
 */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/internet-apps-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/traffic-control-module.h"
#include "ns3/packet-sink.h"
#include "soft-ue-helper.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("SoftUeE2eDemo");

// Statistics tracking structure
struct DemoStats
{
    uint64_t totalPacketsSent = 0;
    uint64_t totalPacketsReceived = 0;
    uint64_t totalBytesSent = 0;
    uint64_t totalBytesReceived = 0;
    Time firstPacketTime = Seconds (0);
    Time lastPacketTime = Seconds (0);
    uint32_t activePdcCount = 0;

    double GetAverageLatency () const
    {
        if (totalPacketsReceived == 0) return 0.0;
        return (lastPacketTime - firstPacketTime).GetMilliSeconds () / totalPacketsReceived;
    }

    double GetThroughput () const
    {
        if (totalBytesReceived == 0) return 0.0;
        double timeSpan = (lastPacketTime - firstPacketTime).GetSeconds ();
        return (timeSpan > 0) ? (totalBytesReceived * 8.0 / timeSpan / 1e6) : 0.0; // Mbps
    }
};

DemoStats g_stats;

void
PacketReceivedCallback (Ptr<const Packet> packet, const Address& from, const Address& to, const TcpHeader& header)
{
    g_stats.totalPacketsReceived++;
    g_stats.totalBytesReceived += packet->GetSize ();

    if (g_stats.firstPacketTime == Seconds (0))
    {
        g_stats.firstPacketTime = Simulator::Now ();
    }
    g_stats.lastPacketTime = Simulator::Now ();

    NS_LOG_INFO ("Received packet " << g_stats.totalPacketsReceived <<
                 " Size: " << packet->GetSize () << " bytes from " << from);
}

void
PacketSentCallback (Ptr<const Packet> packet)
{
    g_stats.totalPacketsSent++;
    g_stats.totalBytesSent += packet->GetSize ();

    NS_LOG_INFO ("Sent packet " << g_stats.totalPacketsSent <<
                 " Size: " << packet->GetSize () << " bytes");
}

void
SoftUePacketTraceCallback (Ptr<const Packet> packet, const Address& address)
{
    NS_LOG_DEBUG ("Soft-Ue packet trace: " << packet << " to/from " << address);
}

void
SoftUeStatsTraceCallback (const SoftUeStats& stats)
{
    g_stats.activePdcCount = stats.activePdcCount;
    NS_LOG_DEBUG ("Soft-Ue statistics - Active PDCs: " << stats.activePdcCount <<
                  " Throughput: " << stats.throughput << " Mbps");
}

void
PrintFinalStatistics ()
{
    std::cout << "\n=== Soft-UE End-to-End Demo Results ===" << std::endl;
    std::cout << "Total Packets Sent: " << g_stats.totalPacketsSent << std::endl;
    std::cout << "Total Packets Received: " << g_stats.totalPacketsReceived << std::endl;
    std::cout << "Total Bytes Sent: " << g_stats.totalBytesSent << std::endl;
    std::cout << "Total Bytes Received: " << g_stats.totalBytesReceived << std::endl;
    std::cout << "Packet Loss Rate: " <<
        ((g_stats.totalPacketsSent > 0) ?
         (100.0 * (g_stats.totalPacketsSent - g_stats.totalPacketsReceived) / g_stats.totalPacketsSent) : 0.0)
        << "%" << std::endl;
    std::cout << "Average Latency: " << g_stats.GetAverageLatency () << " ms" << std::endl;
    std::cout << "Throughput: " << g_stats.GetThroughput () << " Mbps" << std::endl;
    std::cout << "Active PDC Count: " << g_stats.activePdcCount << std::endl;
    std::cout << "========================================" << std::endl;
}

int
main (int argc, char *argv[])
{
    // Configure logging
    LogComponentEnable ("SoftUeE2eDemo", LOG_LEVEL_INFO);
    LogComponentEnable ("SoftUeNetDevice", LOG_LEVEL_DEBUG);

    // Simulation parameters
    uint32_t numNodes = 4;
    uint32_t packetSize = 1024; // bytes
    uint32_t numPackets = 1000;
    DataRate dataRate ("1Mbps");
    Time simTime = Seconds (30);
    bool enableTracing = false;
    bool detailedLogging = false;

    // Command line arguments
    CommandLine cmd;
    cmd.AddValue ("numNodes", "Number of nodes in the simulation");
    cmd.AddValue ("packetSize", "Size of each packet in bytes");
    cmd.AddValue ("numPackets", "Number of packets to send");
    cmd.AddValue ("dataRate", "Data rate for packet transmission");
    cmd.AddValue ("simTime", "Total simulation time");
    cmd.AddValue ("enableTracing", "Enable detailed tracing");
    cmd.AddValue ("detailedLogging", "Enable detailed logging");
    cmd.Parse (argc, argv);

    NS_LOG_INFO ("Starting Soft-UE End-to-End Demo");
    NS_LOG_INFO ("Nodes: " << numNodes << ", Packet Size: " << packetSize <<
                 " bytes, Packets: " << numPackets);
    NS_LOG_INFO ("Data Rate: " << dataRate << ", Simulation Time: " << simTime);

    // Create nodes
    NodeContainer nodes;
    nodes.Create (numNodes);

    // Create Soft-UE helper
    SoftUeHelper softUeHelper;

    // Configure Soft-UE devices
    NetDeviceContainer devices = softUeHelper.Install (nodes);

    // Set up device callbacks
    for (uint32_t i = 0; i < devices.GetN (); ++i)
    {
        Ptr<SoftUeNetDevice> device = DynamicCast<SoftUeNetDevice> (devices.Get (i));
        if (device)
        {
            // Configure device if needed
            SoftUeConfig config = device->GetConfiguration ();
            config.enableDetailedLogging = detailedLogging;
            device->UpdateConfiguration (config);

            // Connect tracing callbacks
            device->TraceConnectWithoutContext ("MacTx",
                MakeCallback (SoftUePacketTraceCallback));
            device->TraceConnectWithoutContext ("MacRx",
                MakeCallback (SoftUePacketTraceCallback));
            device->TraceConnectWithoutContext ("Statistics",
                MakeCallback (SoftUeStatsTraceCallback));
        }
    }

    // Install Internet stack
    InternetStackHelper internet;
    internet.Install (nodes);

    // Assign IP addresses
    Ipv4AddressHelper ipv4;
    ipv4.SetBase ("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer interfaces = ipv4.Assign (devices);

    // Create a simple packet source on node 0
    uint16_t port = 9; // Discard port
    ApplicationContainer apps;

    // Create packet source on node 0
    OnOffHelper onoff ("ns3::UdpSocketFactory",
                         InetSocketAddress (interfaces.GetAddress (numNodes - 1), port));
    onoff.SetAttribute ("DataRate", DataRateValue (dataRate));
    onoff.SetAttribute ("PacketSize", UintegerValue (packetSize));
    onoff.SetAttribute ("MaxBytes", UintegerValue (packetSize * numPackets));
    onoff.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1.0]"));
    onoff.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0.0]"));

    ApplicationContainer sourceApp = onoff.Install (nodes.Get (0));
    sourceApp.Start (Seconds (1.0));
    sourceApp.Stop (simTime - Seconds (1.0));

    // Create packet sink on last node
    PacketSinkHelper sink ("ns3::UdpSocketFactory",
                           InetSocketAddress (Ipv4Address::GetAny (), port));
    ApplicationContainer sinkApp = sink.Install (nodes.Get (numNodes - 1));
    sinkApp.Start (Seconds (0.0));
    sinkApp.Stop (simTime);

    // Connect sink callback
    Ptr<PacketSink> packetSink = DynamicCast<PacketSink> (sinkApp.Get (0));
    if (packetSink)
    {
        packetSink->TraceConnectWithoutContext ("Rx",
            MakeCallback (&PacketReceivedCallback));
    }

    // Set up additional packet flows between node pairs
    for (uint32_t i = 1; i < numNodes - 1; ++i)
    {
        // Create traffic from node i to node (i+1)
        OnOffHelper flow ("ns3::UdpSocketFactory",
                         InetSocketAddress (interfaces.GetAddress (i + 1), port + i));
        flow.SetAttribute ("DataRate", DataRateValue (dataRate / 2));
        flow.SetAttribute ("PacketSize", UintegerValue (packetSize));
        flow.SetAttribute ("MaxBytes", UintegerValue (packetSize * numPackets / 2));
        flow.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1.0]"));
        flow.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0.5]"));

        ApplicationContainer flowApp = flow.Install (nodes.Get (i));
        flowApp.Start (Seconds (1.0 + i * 0.5));
        flowApp.Stop (simTime - Seconds (1.0));
    }

    // Enable ASCII tracing if requested
    if (enableTracing)
    {
        AsciiTraceHelper ascii;
        ascii.Create ("soft-ue-e2e-demo.tr", devices);
    }

    // Schedule final statistics printing
    Simulator::Schedule (simTime, &PrintFinalStatistics);

    // Print device configurations
    std::cout << "\n=== Soft-UE Device Configurations ===" << std::endl;
    for (uint32_t i = 0; i < devices.GetN (); ++i)
    {
        Ptr<SoftUeNetDevice> device = DynamicCast<SoftUeNetDevice> (devices.Get (i));
        if (device)
        {
            SoftUeConfig config = device->GetConfiguration ();
            std::cout << "Node " << i << ":" << std::endl;
            std::cout << "  MAC Address: " << config.address << std::endl;
            std::cout << "  Local FEP: " << config.localFep << std::endl;
            std::cout << "  Max PDC Count: " << config.maxPdcCount << std::endl;
            std::cout << "  MTU: " << config.maxPacketSize << std::endl;
            std::cout << "  Detailed Logging: " << (config.enableDetailedLogging ? "Enabled" : "Disabled") << std::endl;
            std::cout << "  Statistics: " << (config.enableStatistics ? "Enabled" : "Disabled") << std::endl;
        }
    }
    std::cout << "=====================================" << std::endl;

    // Run simulation
    std::cout << "\nRunning simulation for " << simTime.GetSeconds () << " seconds..." << std::endl;
    Simulator::Stop (simTime);
    Simulator::Run ();

    // Print PDC statistics from devices
    std::cout << "\n=== PDC Statistics ===" << std::endl;
    uint32_t totalActivePdcs = 0;
    for (uint32_t i = 0; i < devices.GetN (); ++i)
    {
        Ptr<SoftUeNetDevice> device = DynamicCast<SoftUeNetDevice> (devices.Get (i));
        if (device)
        {
            SoftUeStats stats = device->GetStatistics ();
            std::cout << "Node " << i << " - Active PDCs: " << stats.activePdcCount
                      << ", Total Sent: " << stats.totalPacketsTransmitted
                      << ", Total Received: " << stats.totalPacketsReceived << std::endl;
            totalActivePdcs += stats.activePdcCount;
        }
    }
    std::cout << "Total Active PDCs across all nodes: " << totalActivePdcs << std::endl;
    std::cout << "========================" << std::endl;

    Simulator::Destroy ();

    std::cout << "\nSoft-UE End-to-End Demo completed successfully!" << std::endl;

    return 0;
}
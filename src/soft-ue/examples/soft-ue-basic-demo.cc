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
 * @file             soft-ue-basic-demo.cc
 * @brief            Basic Soft-UE Demo
 * @author           softuegroup@gmail.com
 * @version          1.0.0
 * @date             2025-12-07
 * @copyright        Apache License Version 2.0
 *
 * @details
 * This is a simplified demonstration of the Soft-UE Ultra Ethernet protocol
 * integration with ns-3. It shows basic device creation and configuration.
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

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("SoftUeBasicDemo");

int
main (int argc, char *argv[])
{
    // Configure logging
    LogComponentEnable ("SoftUeBasicDemo", LOG_LEVEL_INFO);

    // Simulation parameters
    uint32_t numNodes = 2;
    DataRate dataRate ("1Mbps");
    Time simTime = Seconds (10);

    // Command line arguments
    CommandLine cmd;
    cmd.AddValue ("numNodes", "Number of nodes in the simulation", numNodes);
    cmd.AddValue ("dataRate", "Data rate for packet transmission", dataRate);
    cmd.AddValue ("simTime", "Total simulation time", simTime);
    cmd.Parse (argc, argv);

    NS_LOG_INFO ("Starting Soft-UE Basic Demo");
    NS_LOG_INFO ("Nodes: " << numNodes << ", Data Rate: " << dataRate <<
                 ", Simulation Time: " << simTime);

    // Create nodes
    NodeContainer nodes;
    nodes.Create (numNodes);

    // Create point-to-point links between nodes
    NodeContainer::Iterator nodeIt = nodes.Begin ();
    Ptr<Node> node1 = *nodeIt++;
    Ptr<Node> node2 = *nodeIt;

    // Install point-to-point channel between node1 and node2
    PointToPointHelper pointToPoint;
    pointToPoint.SetDeviceAttribute ("DataRate", DataRateValue (dataRate));
    pointToPoint.SetChannelAttribute ("Delay", TimeValue (MilliSeconds (2)));

    NetDeviceContainer devices = pointToPoint.Install (node1, node2);

    // Install Internet stack
    InternetStackHelper internet;
    internet.Install (nodes);

    // Assign IP addresses
    Ipv4AddressHelper ipv4;
    ipv4.SetBase ("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer interfaces = ipv4.Assign (devices);

    // Create a simple UDP echo server on node 2
    uint16_t port = 9; // Discard port
    ApplicationContainer serverApps;

    // Create packet sink on node 2 (receiver)
    PacketSinkHelper sink ("ns3::UdpSocketFactory",
                           InetSocketAddress (Ipv4Address::GetAny (), port));
    serverApps = sink.Install (node2);
    serverApps.Start (Seconds (1.0));
    serverApps.Stop (simTime);

    // Create UDP client on node 1 (sender)
    OnOffHelper client ("ns3::UdpSocketFactory",
                       InetSocketAddress (interfaces.GetAddress (1), port));
    client.SetAttribute ("DataRate", DataRateValue (dataRate));
    client.SetAttribute ("PacketSize", UintegerValue (512));
    client.SetAttribute ("MaxBytes", UintegerValue (1024 * 10)); // Send 10KB total
    client.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1.0]"));
    client.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0.0]"));

    ApplicationContainer clientApps = client.Install (node1);
    clientApps.Start (Seconds (2.0));
    clientApps.Stop (simTime - Seconds (1.0));

    // Enable ASCII tracing for basic network traffic
    AsciiTraceHelper ascii;
    pointToPoint.EnableAsciiAll (ascii.CreateFileStream ("soft-ue-basic-demo.tr"));

    // Print configuration information
    std::cout << "\n=== Soft-UE Basic Demo Configuration ===" << std::endl;
    std::cout << "Number of Nodes: " << nodes.GetN () << std::endl;
    std::cout << "Data Rate: " << dataRate << std::endl;
    std::cout << "Link Delay: 2 ms" << std::endl;
    std::cout << "Simulation Time: " << simTime.GetSeconds () << " seconds" << std::endl;

    for (uint32_t i = 0; i < devices.GetN (); ++i)
    {
        Ptr<NetDevice> device = devices.Get (i);
        Ptr<Node> node = device->GetNode ();
        std::cout << "Node " << node->GetId () << " - Device: " << device << std::endl;
    }
    std::cout << "========================================" << std::endl;

    // Set up FlowMonitor to collect statistics
    FlowMonitorHelper flowmon;
    Ptr<FlowMonitor> monitor = flowmon.InstallAll();

    // Run simulation
    std::cout << "\nRunning simulation for " << simTime.GetSeconds () << " seconds..." << std::endl;
    Simulator::Stop (simTime);
    Simulator::Run ();

    // Print FlowMonitor statistics
    monitor->CheckForLostPackets ();
    Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmon.GetClassifier ());
    std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats ();

    std::cout << "\n=== Flow Statistics ===" << std::endl;
    for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin ();
         i != stats.end (); ++i)
    {
        Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (i->first);
        std::cout << "Flow " << i->first << " (" << t.sourceAddress << " -> "
                  << t.destinationAddress << ")\n";
        std::cout << "  Tx Packets: " << i->second.txPackets << "\n";
        std::cout << "  Rx Packets: " << i->second.rxPackets << "\n";
        std::cout << "  Lost Packets: " << i->second.lostPackets << "\n";
        std::cout << "  Tx Bytes: " << i->second.txBytes << "\n";
        std::cout << "  Rx Bytes: " << i->second.rxBytes << "\n";
        std::cout << "  Throughput: " << i->second.rxBytes * 8.0 / (i->second.timeLastRxPacket.GetSeconds() - i->second.timeFirstTxPacket.GetSeconds()) / 1024 / 1024 << " Mbps\n";
    }
    std::cout << "=========================" << std::endl;

    // Get PacketSink statistics
    Ptr<PacketSink> packetSink = DynamicCast<PacketSink> (serverApps.Get (0));
    if (packetSink)
    {
        std::cout << "\n=== Packet Sink Statistics ===" << std::endl;
        std::cout << "Total Received: " << packetSink->GetTotalRx () << " bytes" << std::endl;
        std::cout << "Total Packets: " << packetSink->GetTotalRx () / 512 << std::endl; // Assuming 512 byte packets
        std::cout << "=============================" << std::endl;
    }

    Simulator::Destroy ();

    std::cout << "\nSoft-UE Basic Demo completed successfully!" << std::endl;
    std::cout << "\nNote: This demo shows basic network connectivity." << std::endl;
    std::cout << "Full Soft-UE Ultra Ethernet protocol features are being implemented." << std::endl;

    return 0;
}
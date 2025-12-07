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
 * @file             soft-ue-simple-example.cc
 * @brief            Basic Soft-UE Example Application
 * @author           softuegroup@gmail.com
 * @version          1.0.0
 * @date             2025-12-07
 * @copyright        Apache License Version 2.0
 *
 * @details
 * This example demonstrates the basic setup and usage of Soft-UE in ns-3.
 * It creates a simple two-node topology with Soft-UE devices and shows
 * how to send and receive packets using the Ultra Ethernet protocol.
 */

#include <ns3/core-module.h>
#include <ns3/network-module.h>
#include <ns3/internet-module.h>
#include <ns3/applications-module.h>
#include <ns3/point-to-point-module.h>

// Include Soft-UE headers (will be implemented later)
// #include <ns3/soft-ue-helper.h>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("SoftUeSimpleExample");

/**
 * @brief Print packet information
 * @param prefix Prefix string for logging
 * @param p Packet to print information about
 */
void PrintPacketInfo (std::string prefix, Ptr<Packet> p)
{
    NS_LOG_INFO (prefix << " Packet: size=" << p->GetSize () << " bytes");
}

/**
 * @brief Main function
 * @param argc Command line argument count
 * @param argv Command line arguments
 * @return 0 on success
 */
int
main (int argc, char *argv[])
{
    // Log component enablement
    LogComponentEnable ("SoftUeSimpleExample", LOG_LEVEL_INFO);

    // Parse command line arguments
    CommandLine cmd (__FILE__);
    cmd.Parse (argc, argv);

    // Set simulation parameters
    Time::SetResolution (Time::NS);

    NS_LOG_INFO ("Starting Soft-UE Simple Example");

    // Create nodes
    NodeContainer nodes;
    nodes.Create (2);

    // Create point-to-point link
    PointToPointHelper pointToPoint;
    pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("1Gbps"));
    pointToPoint.SetChannelAttribute ("Delay", StringValue ("1ms"));

    // Install point-to-point devices
    NetDeviceContainer devices;
    devices = pointToPoint.Install (nodes);

    // Install internet stack
    InternetStackHelper internet;
    internet.Install (nodes);

    // Assign IP addresses
    Ipv4AddressHelper ipv4;
    ipv4.SetBase ("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer interfaces = ipv4.Assign (devices);

    // Note: Soft-UE integration will be added in later stages
    // For now, this demonstrates the basic ns-3 setup

    NS_LOG_INFO ("Node 0 IP: " << interfaces.GetAddress (0));
    NS_LOG_INFO ("Node 1 IP: " << interfaces.GetAddress (1));

    // Create a simple UDP echo application for testing
    uint16_t port = 9;  // Discard port

    UdpEchoServerHelper echoServer (port);
    ApplicationContainer serverApp = echoServer.Install (nodes.Get (1));
    serverApp.Start (Seconds (1.0));
    serverApp.Stop (Seconds (10.0));

    UdpEchoClientHelper echoClient (interfaces.GetAddress (1), port);
    echoClient.SetAttribute ("MaxPackets", UintegerValue (5));
    echoClient.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
    echoClient.SetAttribute ("PacketSize", UintegerValue (1024));

    ApplicationContainer clientApp = echoClient.Install (nodes.Get (0));
    clientApp.Start (Seconds (2.0));
    clientApp.Stop (Seconds (10.0));

    // Enable packet capture for analysis (optional)
    // pointToPoint.EnablePcapAll ("soft-ue-simple");

    // Run simulation
    NS_LOG_INFO ("Running simulation...");
    Simulator::Stop (Seconds (11.0));
    Simulator::Run ();

    NS_LOG_INFO ("Simulation completed successfully");

    // Clean up
    Simulator::Destroy ();

    return 0;
}
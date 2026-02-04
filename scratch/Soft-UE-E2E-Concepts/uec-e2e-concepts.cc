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
 * @file             uec-e2e-concepts.cc
 * @brief            UEC end-to-end concepts: SES/PDS/PDC/FEP/MSN/SOM/EOM/RTO
 * @author           softuegroup@gmail.com
 * @date             2025-02-04
 *
 * @details
 * This program prints an annotated end-to-end flow for the UEC protocol stack.
 *
 * Concepts covered:
 * - FEP (Fabric Endpoint)
 * - SES (Semantic Sub-layer): OperationMetadata + ProcessSendRequest
 * - PDS (Packet Delivery Sub-layer): PDSHeader (pdc_id, seq_num, SOM, EOM)
 * - PDC (Packet Delivery Context)
 * - IPDC / TPDC: unreliable vs reliable (RTO is in TPDC)
 * - MSN (Message Sequence Number)
 * - SOM / EOM (Start/End of Message)
 */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/soft-ue-module.h"
#include "ns3/soft-ue-packet-tag.h"
#include "ns3/packet.h"
#include <iostream>
#include <iomanip>
#include <string>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("UecE2EConcepts");

// ---------------------------------------------------------------------------
// Config: keep small for readability
// ---------------------------------------------------------------------------
struct E2EConceptsConfig
{
  uint32_t nodeCount = 2;
  uint32_t transactionSize = 256; // payload size per transaction (bytes)
  uint32_t packetCount = 20;      // number of transactions to send
  Time sendInterval = MilliSeconds (10);
  uint32_t maxPdcCount = 64;
  DataRate channelDataRate = DataRate ("1Gbps");
  Time propagationDelay = NanoSeconds (100);

  uint16_t clientEndpointId = 1001;
  uint16_t serverEndpointId = 8000;
  uint32_t baseClientJobId = 12345;
  uint32_t baseServerJobId = 54321;
  uint64_t baseMemoryAddress = 0x1000;
};

// ---------------------------------------------------------------------------
// Demo app: client sends, server receives, prints an annotated flow
// ---------------------------------------------------------------------------
class ConceptsDemoApp : public Application
{
public:
  ConceptsDemoApp ();
  virtual ~ConceptsDemoApp ();

  void Setup (uint32_t transactionSize, uint32_t numPackets, Address destination,
              uint16_t port, bool isServer);
  void SetConfig (const E2EConceptsConfig& config);
  uint32_t GetPacketsSent () const { return m_packetsSent; }
  uint32_t GetPacketsReceived () const { return m_packetsReceived; }

private:
  void StartApplication () override;
  void StopApplication () override;
  void SendPacket ();
  void ScheduleSend ();
  bool HandleRead (Ptr<NetDevice> device, Ptr<const Packet> packet,
                  uint16_t protocolType, const Address& source);

  uint32_t m_transactionSize;
  uint32_t m_numPackets;
  uint32_t m_packetsSent;
  uint32_t m_packetsReceived;
  Address m_destination;
  uint16_t m_port;
  bool m_isServer;
  EventId m_sendEvent;
  Ptr<SesManager> m_sesManager;
  Ptr<PdsManager> m_pdsManager;
  E2EConceptsConfig m_config;
};

ConceptsDemoApp::ConceptsDemoApp ()
  : m_transactionSize (0), m_numPackets (0), m_packetsSent (0), m_packetsReceived (0),
    m_port (0), m_isServer (false)
{
}

ConceptsDemoApp::~ConceptsDemoApp ()
{
}

void
ConceptsDemoApp::Setup (uint32_t transactionSize, uint32_t numPackets,
                       Address destination, uint16_t port, bool isServer)
{
  m_transactionSize = transactionSize;
  m_numPackets = numPackets;
  m_destination = destination;
  m_port = port;
  m_isServer = isServer;
}

void
ConceptsDemoApp::SetConfig (const E2EConceptsConfig& config)
{
  m_config = config;
}

void
ConceptsDemoApp::StartApplication ()
{
  if (GetNode ()->GetNDevices () == 0)
    return;

  for (uint32_t i = 0; i < GetNode ()->GetNDevices (); ++i)
    {
      Ptr<SoftUeNetDevice> device =
          GetNode ()->GetDevice (i)->GetObject<SoftUeNetDevice> ();
      if (device)
        {
          m_sesManager = device->GetSesManager ();
          m_pdsManager = device->GetPdsManager ();
          if (m_sesManager && m_pdsManager)
            m_sesManager->SetPdsManager (m_pdsManager);
          if (m_isServer)
            device->SetReceiveCallback (
                MakeCallback (&ConceptsDemoApp::HandleRead, this));
          break;
        }
    }

  if (!m_isServer)
    {
      NS_LOG_INFO ("[Phase 3] Client: start sending " << m_numPackets
                   << " transactions (each " << m_transactionSize << " B) (SES+PDS+PDC end-to-end)");
      ScheduleSend ();
    }
}

void
ConceptsDemoApp::StopApplication ()
{
  if (m_sendEvent.IsPending ())
    Simulator::Cancel (m_sendEvent);
}

void
ConceptsDemoApp::ScheduleSend ()
{
  if (m_packetsSent < m_numPackets)
    m_sendEvent =
        Simulator::Schedule (m_config.sendInterval, &ConceptsDemoApp::SendPacket, this);
}

void
ConceptsDemoApp::SendPacket ()
{
  if (!m_sesManager || !m_pdsManager)
    return;
  if (m_packetsSent >= m_numPackets)
    return;

  Ptr<SoftUeNetDevice> device;
  for (uint32_t i = 0; i < GetNode ()->GetNDevices (); ++i)
    {
      device = GetNode ()->GetDevice (i)->GetObject<SoftUeNetDevice> ();
      if (device)
        break;
    }
  if (!device)
    {
      ScheduleSend ();
      return;
    }

  uint32_t k = m_packetsSent + 1;
  uint32_t n = m_numPackets;
  if (n > 1)
    NS_LOG_INFO ("[UEC-E2E] [App] Tx " << k << "/" << n << " size=" << m_transactionSize
                 << " B -> device->Send() (SES sends as single packet or fragments by MTU)");

  Ptr<Packet> packet = Create<Packet> (m_transactionSize);
  if (!packet)
    {
      ScheduleSend ();
      return;
    }
  packet->AddPacketTag (SoftUeTimingTag (Simulator::Now ()));
  if (m_numPackets > 1)
    packet->AddPacketTag (SoftUeTransactionTag (k, n));

  bool success = device->Send (packet, m_destination, 0x0800);
  if (success)
    m_packetsSent++;

  ScheduleSend ();
}

bool
ConceptsDemoApp::HandleRead (Ptr<NetDevice> device, Ptr<const Packet> packet,
                            uint16_t protocolType, const Address& source)
{
  if (!packet)
    return false;
  m_packetsReceived++;

  Ptr<Packet> copy = packet->Copy ();
  PDSHeader pdsHeader;
  copy->RemoveHeader (pdsHeader);
  NS_LOG_INFO ("[UEC-E2E] [App] (8) Receive: seq=" << pdsHeader.GetSequenceNumber ()
               << " pdc_id=" << pdsHeader.GetPdcId () << " size=" << packet->GetSize ()
               << " (total received " << m_packetsReceived << ")");

  return true;
}

// ---------------------------------------------------------------------------
// Print FEPs (Phase 0)
// ---------------------------------------------------------------------------
void
PrintFepPhase (NetDeviceContainer& devices)
{
  std::cout << "\n" << std::string (60, '=') << "\n";
  std::cout << "  Phase 0: FEP (Fabric Endpoint)\n";
  std::cout << std::string (60, '=') << "\n";

  for (uint32_t i = 0; i < devices.GetN (); ++i)
    {
      Ptr<SoftUeNetDevice> dev = DynamicCast<SoftUeNetDevice> (devices.Get (i));
      if (dev)
        {
          SoftUeConfig cfg = dev->GetConfiguration ();
          std::cout << "  Node " << i << ": FEP=" << cfg.localFep << " (unique endpoint id)\n";
        }
    }
  std::cout << "  Diagrams: attachment/SUETArchitecture.png and attachment/CoreComponents.png\n";
  std::cout << std::string (60, '-') << "\n";
}

// ---------------------------------------------------------------------------
// Print concept checklist (Phase 4)
// ---------------------------------------------------------------------------
void
PrintConceptsChecklist (Ptr<ConceptsDemoApp> clientApp, Ptr<ConceptsDemoApp> serverApp,
                       Ptr<SoftUeNetDevice> dev0, Ptr<SoftUeNetDevice> dev1)
{
  std::cout << "\n" << std::string (60, '=') << "\n";
  std::cout << "  Phase 4: UEC concept checklist covered by this program\n";
  std::cout << std::string (60, '=') << "\n";

  std::cout << "  [x] FEP     - Fabric Endpoint (see Phase 0)\n";
  std::cout << "  [x] SES     - Semantic Sub-layer: OperationMetadata + ProcessSendRequest\n";
  std::cout << "  [x] PDS     - Packet Delivery Sub-layer: PDSHeader(pdc_id, seq, SOM, EOM)\n";
  std::cout << "  [x] PDC     - Packet Delivery Context: selected by pdc_id (IPDC/TPDC)\n";
  std::cout << "  [x] IPDC    - Unreliable PDC (pdc_id 1..maxPdcCount in this program)\n";
  std::cout << "  [x] TPDC    - Reliable PDC (pdc_id 513..1024), includes RTO (not exercised here)\n";
  std::cout << "  [x] MSN     - Message Sequence Number: metadata.messages_id + PDS seq_num\n";
  std::cout << "  [x] SOM/EOM - Start/End of Message: SOM=1 on first, EOM=1 on last\n";
  std::cout << "  [x] RTO     - Retransmission Timeout (TPDC)\n";

  if (clientApp && serverApp)
    std::cout << "\n  Traffic: client sent " << clientApp->GetPacketsSent ()
              << " transactions, server received " << serverApp->GetPacketsReceived () << " packets\n";

  if (dev0 && dev1)
    {
      Ptr<PdsStatistics> st0 = dev0->GetPdsManager ()->GetStatistics ();
      Ptr<PdsStatistics> st1 = dev1->GetPdsManager ()->GetStatistics ();
      std::cout << "\n  PDS stats (Node 0):\n" << st0->GetStatistics () << "\n";
      std::cout << "  PDS stats (Node 1):\n" << st1->GetStatistics () << "\n";
    }

  std::cout << std::string (60, '=') << "\n";
}

// ---------------------------------------------------------------------------
// main
// ---------------------------------------------------------------------------
int
main (int argc, char* argv[])
{
  E2EConceptsConfig config;
  CommandLine cmd;
  cmd.AddValue ("transactionSize", "Payload size per transaction (bytes)", config.transactionSize);
  cmd.AddValue ("packetCount", "Number of transactions", config.packetCount);
  cmd.AddValue ("maxPdcCount", "Max PDC count per device", config.maxPdcCount);
  cmd.Parse (argc, argv);

  // Enable INFO logs for the end-to-end annotated flow (all lines start with [UEC-E2E])
  LogComponentEnable ("UecE2EConcepts", LOG_LEVEL_INFO);
  LogComponentEnable ("SesManager", LOG_LEVEL_INFO);
  LogComponentEnable ("SoftUeNetDevice", LOG_LEVEL_INFO);
  LogComponentEnable ("SoftUeChannel", LOG_LEVEL_INFO);
  LogComponentEnable ("PdsManager", LOG_LEVEL_INFO);
  LogComponentEnable ("Ipdc", LOG_LEVEL_INFO);

  std::cout << "\n" << std::string (60, '=') << "\n";
  std::cout << "   UEC end-to-end concepts (annotated logs)\n";
  std::cout << "   (The [UEC-E2E] lines highlight key steps in the send/receive path)\n";
  std::cout << std::string (60, '=') << "\n";
  std::cout << "  transactionSize=" << config.transactionSize
            << " packetCount=" << config.packetCount
            << " maxPdcCount=" << config.maxPdcCount << "\n";

  NodeContainer nodes;
  nodes.Create (config.nodeCount);

  SoftUeHelper helper;
  helper.SetDeviceAttribute ("MaxPdcCount", UintegerValue (config.maxPdcCount));
  helper.SetDeviceAttribute ("EnableStatistics", BooleanValue (true));
  helper.SetDeviceAttribute ("MaxPacketSize", UintegerValue (65535));

  NetDeviceContainer devices = helper.Install (nodes);

  for (uint32_t i = 0; i < devices.GetN (); ++i)
    devices.Get (i)->SetMtu (65535);

  Ptr<SoftUeNetDevice> dev0 = DynamicCast<SoftUeNetDevice> (devices.Get (0));
  if (dev0)
    {
      Ptr<SoftUeChannel> ch = DynamicCast<SoftUeChannel> (dev0->GetChannel ());
      if (ch)
        {
          ch->SetDataRate (config.channelDataRate);
          ch->SetDelay (config.propagationDelay);
        }
    }

  Ptr<SoftUeNetDevice> device0 = DynamicCast<SoftUeNetDevice> (devices.Get (0));
  Ptr<SoftUeNetDevice> device1 = DynamicCast<SoftUeNetDevice> (devices.Get (1));
  Ptr<PdsManager> pds0 = device0->GetPdsManager ();
  Ptr<PdsManager> pds1 = device1->GetPdsManager ();
  Ptr<SesManager> ses0 = device0->GetSesManager ();
  Ptr<SesManager> ses1 = device1->GetSesManager ();
  pds0->Initialize ();
  pds1->Initialize ();
  ses0->Initialize ();
  ses1->Initialize ();

  InternetStackHelper internet;
  internet.Install (nodes);
  Ipv4AddressHelper address;
  address.SetBase ("10.1.1.0", "255.255.255.0");
  address.Assign (devices);

  SoftUeConfig serverCfg = device1->GetConfiguration ();
  Address serverAddress = serverCfg.address;

  PrintFepPhase (devices);

  std::cout << "\n  Phase 1 & 2: each packet goes through SES (OperationMetadata + ProcessSendRequest)\n";
  std::cout << "               and PDS (PDSHeader: pdc_id, seq_num, SOM, EOM) in SendPacket()\n";
  std::cout << std::string (60, '-') << "\n";

  Time simTime = Seconds (1.0) + config.sendInterval * config.packetCount + MilliSeconds (200);

  Ptr<ConceptsDemoApp> serverApp = CreateObject<ConceptsDemoApp> ();
  serverApp->SetConfig (config);
  serverApp->Setup (0, 0, Address (), 8000, true);
  serverApp->SetStartTime (MilliSeconds (1));
  serverApp->SetStopTime (simTime);
  nodes.Get (1)->AddApplication (serverApp);

  Ptr<ConceptsDemoApp> clientApp = CreateObject<ConceptsDemoApp> ();
  clientApp->SetConfig (config);
  clientApp->Setup (config.transactionSize, config.packetCount, serverAddress, 8000, false);
  clientApp->SetStartTime (MilliSeconds (50));
  clientApp->SetStopTime (simTime);
  nodes.Get (0)->AddApplication (clientApp);

  Simulator::Stop (simTime);
  Simulator::Run ();

  PrintConceptsChecklist (clientApp, serverApp, device0, device1);

  Simulator::Destroy ();
  return 0;
}

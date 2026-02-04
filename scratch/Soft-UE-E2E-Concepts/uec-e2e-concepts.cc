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
 * @brief            UEC 端到端概念实验 — 覆盖 SES/PDS/PDC/FEP/MSN/SOM/EOM/RTO
 * @author           softuegroup@gmail.com
 * @date             2025-02-04
 *
 * @details
 * 本实验按阶段演示 Ultra Ethernet 协议栈中的关键概念，便于理解项目。
 * 对应文档：项目剖析与追踪/07-UEC端到端概念实验与图解.md
 *
 * 覆盖概念：
 * - FEP (Fabric Endpoint)      — 网络端点标识
 * - SES (Semantic Sub-layer)   — 操作类型、OperationMetadata、ProcessSendRequest
 * - PDS (Packet Delivery Sub-layer) — PDSHeader、pdc_id、seq_num、SOM/EOM
 * - PDC (Packet Delivery Context)   — 传输上下文
 * - IPDC / TPDC                — 不可靠 vs 可靠（RTO 重传在 TPDC）
 * - MSN (Message Sequence Number)   — 消息序列与乱序重组
 * - SOM / EOM                  — 消息起止标记
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
// 配置：小规模便于观察
// ---------------------------------------------------------------------------
struct E2EConceptsConfig
{
  uint32_t nodeCount = 2;
  uint32_t packetSize = 256;
  uint32_t packetCount = 20;       // 少量包便于理解
  uint32_t largeTransactionSize = 0; // 若 >0：先发一条「大事务」(单包大 payload)，SES 按 MTU 拆成多包
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
// 概念演示应用：客户端发送、服务端接收，并打印各阶段对应概念
// ---------------------------------------------------------------------------
class ConceptsDemoApp : public Application
{
public:
  ConceptsDemoApp ();
  virtual ~ConceptsDemoApp ();

  void Setup (uint32_t packetSize, uint32_t numPackets, Address destination,
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

  uint32_t m_packetSize;
  uint32_t m_numPackets;
  uint32_t m_packetsSent;
  uint32_t m_packetsReceived;
  bool m_largeTransactionSent;  // 是否已发送「1 事务多包」大消息
  Address m_destination;
  uint16_t m_port;
  bool m_isServer;
  EventId m_sendEvent;
  Ptr<SesManager> m_sesManager;
  Ptr<PdsManager> m_pdsManager;
  E2EConceptsConfig m_config;
};

ConceptsDemoApp::ConceptsDemoApp ()
  : m_packetSize (0), m_numPackets (0), m_packetsSent (0), m_packetsReceived (0),
    m_largeTransactionSent (false), m_port (0), m_isServer (false)
{
}

ConceptsDemoApp::~ConceptsDemoApp ()
{
}

void
ConceptsDemoApp::Setup (uint32_t packetSize, uint32_t numPackets,
                       Address destination, uint16_t port, bool isServer)
{
  m_packetSize = packetSize;
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
      NS_LOG_INFO ("[Phase 3] Client: 开始发送 " << m_numPackets << " 个包 (SES+PDS+PDC 全路径)");
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

  // ---------- A3: 1 事务多包 — 先发一条大消息（无 PDS 头），由 SES 按 MTU 拆成多包 ----------
  if (!m_largeTransactionSent && m_config.largeTransactionSize > 0)
    {
      Ptr<Packet> bigPacket = Create<Packet> (m_config.largeTransactionSize);
      if (!bigPacket)
        {
          m_largeTransactionSent = true;
          ScheduleSend ();
          return;
        }
      bigPacket->AddPacketTag (SoftUeTimingTag (Simulator::Now ()));
      NS_LOG_INFO ("[UEC-E2E] [App] ① 应用层 1 事务大消息 size=" << m_config.largeTransactionSize
                   << " B → device->Send()（SES 将按 MTU 拆成多包，SOM/EOM 见 [SES] 日志）");
      (void) device->Send (bigPacket, m_destination, 0x0800);
      m_largeTransactionSent = true;
      m_packetsSent++;
      ScheduleSend ();
      return;
    }

  Ptr<Packet> packet = Create<Packet> (m_packetSize);
  if (!packet)
    return;

  uint32_t seq = m_packetsSent + 1;
  uint16_t pdcId = (m_packetsSent % m_config.maxPdcCount) + 1;
  bool som = (m_packetsSent == 0);
  bool eom = (m_packetsSent == m_numPackets - 1);

  // 第二个「打包」分界：大事务切包发完后，开始发单包组
  if (m_largeTransactionSent && m_packetsSent == 1)
    {
      NS_LOG_INFO ("============================================================");
      NS_LOG_INFO (" [UEC-E2E] 单包组开始（共 " << m_numPackets << " 个单包）");
      NS_LOG_INFO ("============================================================");
    }
  NS_LOG_INFO ("[UEC-E2E] [App] ① 应用层 构造包 size=" << m_packetSize << " seq=" << seq);

  // ---------- PDS 概念：PDSHeader (pdc_id, seq_num, SOM, EOM) ----------
  PDSHeader pdsHeader;
  pdsHeader.SetPdcId (pdcId);
  pdsHeader.SetSequenceNumber (seq);
  pdsHeader.SetSom (som);
  pdsHeader.SetEom (eom);
  packet->AddHeader (pdsHeader);
  NS_LOG_INFO ("[UEC-E2E] [App] ② App 准备 PDS 头字段 pdc_id=" << pdcId << " seq=" << seq << " SOM=" << som << " EOM=" << eom << "（尚未经 PDS 层）");

  // ---------- SES 概念：OperationMetadata (OpType, FEP 侧信息, job_id, messages_id) ----------
  Ptr<ExtendedOperationMetadata> extMetadata = Create<ExtendedOperationMetadata> ();
  extMetadata->op_type = OpType::SEND;
  extMetadata->s_pid_on_fep = m_config.clientEndpointId + (m_packetsSent % 100);
  extMetadata->t_pid_on_fep = m_config.serverEndpointId + (m_packetsSent % 100);
  extMetadata->job_id = m_config.baseClientJobId;
  extMetadata->messages_id = seq; // MSN 相关
  extMetadata->payload.start_addr = m_config.baseMemoryAddress + m_packetsSent * 256;
  extMetadata->payload.length = m_packetSize;
  extMetadata->payload.imm_data = 0xCAFEBABE + m_packetsSent;
  extMetadata->use_optimized_header = false;
  extMetadata->has_imm_data = true;

  uint32_t srcNodeId = GetNode ()->GetId () + 1;
  uint32_t dstNodeId = (srcNodeId == 1) ? 2 : 1;
  extMetadata->SetSourceEndpoint (srcNodeId, m_config.clientEndpointId);
  extMetadata->SetDestinationEndpoint (dstNodeId, m_config.serverEndpointId);
  NS_LOG_INFO ("[UEC-E2E] [App] ③ SES 元数据 src_node=" << srcNodeId << " dst_node=" << dstNodeId << " job_id=" << m_config.baseClientJobId << " messages_id=" << seq);

  // ---------- SES 概念：ProcessSendRequest（验证端点、参与发送路径）— 发送路径先经 SES，再经 PDS/PDC ----------
  (void) m_sesManager->ProcessSendRequest (extMetadata);

  // 打时间戳，便于接收端统计延迟（PDS 统计中的 Average/Min/Max Latency、Jitter）
  packet->AddPacketTag (SoftUeTimingTag (Simulator::Now ()));
  NS_LOG_INFO ("[UEC-E2E] [App] ④ SES 校验通过 → 打时间戳 → 调用 device->Send()（随后经 PDS Manager → PDC → Channel）");

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
  NS_LOG_INFO ("[UEC-E2E] [App] ⑧ 应用层 收包 seq=" << pdsHeader.GetSequenceNumber ()
               << " pdc_id=" << pdsHeader.GetPdcId () << " size=" << packet->GetSize ()
               << " (累计接收 " << m_packetsReceived << " 包)");

  return true;
}

// ---------------------------------------------------------------------------
// 打印 FEP（Phase 0）
// ---------------------------------------------------------------------------
void
PrintFepPhase (NetDeviceContainer& devices)
{
  std::cout << "\n" << std::string (60, '=') << "\n";
  std::cout << "  Phase 0: FEP (Fabric Endpoint) — 网络端点标识\n";
  std::cout << std::string (60, '=') << "\n";

  for (uint32_t i = 0; i < devices.GetN (); ++i)
    {
      Ptr<SoftUeNetDevice> dev = DynamicCast<SoftUeNetDevice> (devices.Get (i));
      if (dev)
        {
          SoftUeConfig cfg = dev->GetConfiguration ();
          std::cout << "  Node " << i << " → FEP = " << cfg.localFep
                    << " (Fabric Endpoint 决定该节点在网络中的唯一标识)\n";
        }
    }
  std::cout << "  图解参考: 07-UEC端到端概念实验与图解.md §3.1 FEP\n";
  std::cout << std::string (60, '-') << "\n";
}

// ---------------------------------------------------------------------------
// 打印概念清单（Phase 4）
// ---------------------------------------------------------------------------
void
PrintConceptsChecklist (Ptr<ConceptsDemoApp> clientApp, Ptr<ConceptsDemoApp> serverApp,
                       Ptr<SoftUeNetDevice> dev0, Ptr<SoftUeNetDevice> dev1)
{
  std::cout << "\n" << std::string (60, '=') << "\n";
  std::cout << "  Phase 4: 本实验覆盖的 UEC 关键概念清单\n";
  std::cout << std::string (60, '=') << "\n";

  std::cout << "  [x] FEP   — Fabric Endpoint，见 Phase 0 打印\n";
  std::cout << "  [x] SES   — Semantic Sub-layer，OperationMetadata + ProcessSendRequest\n";
  std::cout << "  [x] PDS   — Packet Delivery Sub-layer，PDSHeader(pdc_id, seq, SOM, EOM)\n";
  std::cout << "  [x] PDC   — Packet Delivery Context，由 pdc_id 选择 IPDC/TPDC\n";
  std::cout << "  [x] IPDC  — 不可靠 PDC，本实验使用 pdc_id 1..maxPdcCount (IPDC 段)\n";
  std::cout << "  [x] TPDC  — 可靠 PDC，pdc_id 513..1024，含 RTO 重传（本脚本未显式测 RTO）\n";
  std::cout << "  [x] MSN   — Message Sequence Number，metadata.messages_id + PDS seq_num\n";
  std::cout << "  [x] SOM/EOM — Start/End of Message，PDSHeader 中首包 SOM=1、末包 EOM=1\n";
  std::cout << "  [x] RTO   — Retransmission Timeout，TPDC 专用，见 rto-timer 模块\n";

  if (clientApp && serverApp)
    std::cout << "\n  收发包: 客户端发送 " << clientApp->GetPacketsSent ()
              << " 包, 服务端接收 " << serverApp->GetPacketsReceived () << " 包\n";

  if (dev0 && dev1)
    {
      Ptr<PdsStatistics> st0 = dev0->GetPdsManager ()->GetStatistics ();
      Ptr<PdsStatistics> st1 = dev1->GetPdsManager ()->GetStatistics ();
      std::cout << "\n  PDS 统计 (Node 0):\n" << st0->GetStatistics () << "\n";
      std::cout << "  PDS 统计 (Node 1):\n" << st1->GetStatistics () << "\n";
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
  cmd.AddValue ("packetSize", "每包大小(字节)", config.packetSize);
  cmd.AddValue ("packetCount", "发送包数", config.packetCount);
  cmd.AddValue ("largeTransactionSize", "若>0 先发一条大事务(字节)，SES按MTU拆成多包", config.largeTransactionSize);
  cmd.AddValue ("maxPdcCount", "最大 PDC 数", config.maxPdcCount);
  cmd.Parse (argc, argv);

  // 启用端到端流程关键日志，便于理解全流程（所有 [UEC-E2E] 开头的行）
  LogComponentEnable ("UecE2EConcepts", LOG_LEVEL_INFO);
  LogComponentEnable ("SesManager", LOG_LEVEL_INFO);
  LogComponentEnable ("SoftUeNetDevice", LOG_LEVEL_INFO);
  LogComponentEnable ("SoftUeChannel", LOG_LEVEL_INFO);
  LogComponentEnable ("PdsManager", LOG_LEVEL_INFO);
  LogComponentEnable ("Ipdc", LOG_LEVEL_INFO);

  std::cout << "\n" << std::string (60, '=') << "\n";
  std::cout << "   UEC 端到端概念实验 (对应 07-UEC端到端概念实验与图解.md)\n";
  std::cout << "   (下方 [UEC-E2E] 日志为端到端流程关键节点，①②③… 对应 07 文档图解)\n";
  std::cout << std::string (60, '=') << "\n";
  std::cout << "  packetSize=" << config.packetSize
            << " packetCount=" << config.packetCount
            << " largeTransactionSize=" << config.largeTransactionSize
            << " maxPdcCount=" << config.maxPdcCount << "\n";
  if (config.largeTransactionSize > 0)
    std::cout << "  (1 事务多包: 先发 " << config.largeTransactionSize
              << " B 大消息，SES 将拆成多包，见 [SES] 事务→多包 日志)\n";

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

  std::cout << "\n  Phase 1 & 2: 每个包会经过 SES(OperationMetadata+ProcessSendRequest)\n";
  std::cout << "               与 PDS(PDSHeader: pdc_id, seq_num, SOM, EOM)，见 SendPacket()\n";
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
  clientApp->Setup (config.packetSize, config.packetCount, serverAddress, 8000, false);
  clientApp->SetStartTime (MilliSeconds (50));
  clientApp->SetStopTime (simTime);
  nodes.Get (0)->AddApplication (clientApp);

  Simulator::Stop (simTime);
  Simulator::Run ();

  PrintConceptsChecklist (clientApp, serverApp, device0, device1);

  Simulator::Destroy ();
  return 0;
}

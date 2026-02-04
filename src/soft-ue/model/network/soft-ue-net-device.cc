#include "soft-ue-net-device.h"
#include "soft-ue-channel.h"
#include "../ses/ses-manager.h"
#include "../ses/operation-metadata.h"
#include "../pds/pds-manager.h"
#include "../pdc/pdc-base.h"
#include "../common/soft-ue-packet-tag.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/nstime.h"
#include "ns3/pointer.h"
#include "ns3/uinteger.h"
#include "ns3/boolean.h"
#include "ns3/mac48-address.h"
#include "ns3/ipv4-header.h"
#include "ns3/ethernet-header.h"
#include <iostream>
#include <sstream>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("SoftUeNetDevice");
NS_OBJECT_ENSURE_REGISTERED (SoftUeNetDevice);

TypeId
SoftUeNetDevice::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SoftUeNetDevice")
    .SetParent<NetDevice> ()
    .SetGroupName ("Soft-Ue")
    .AddConstructor<SoftUeNetDevice> ()
    .AddAttribute ("EnableStatistics",
                   "Enable statistics collection",
                   BooleanValue (true),
                   MakeBooleanAccessor (&SoftUeNetDevice::SetEnableStatistics, &SoftUeNetDevice::GetEnableStatistics),
                   MakeBooleanChecker ())
    .AddAttribute ("MaxPdcCount",
                   "Maximum number of PDCs per type",
                   UintegerValue (512),
                   MakeUintegerAccessor (&SoftUeNetDevice::SetMaxPdcCount, &SoftUeNetDevice::GetMaxPdcCount),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("ProcessingInterval",
                   "Packet processing interval (Data center level latency)",
                   TimeValue (NanoSeconds (50)), // 50ns - Data center packet processing delay
                   MakeTimeAccessor (&SoftUeNetDevice::SetProcessingInterval, &SoftUeNetDevice::GetProcessingInterval),
                   MakeTimeChecker ())
    .AddAttribute ("MaxPacketSize",
                   "Maximum packet size",
                   UintegerValue (1500),
                   MakeUintegerAccessor (&SoftUeNetDevice::SetMaxPacketSize, &SoftUeNetDevice::GetMaxPacketSize),
                   MakeUintegerChecker<uint16_t> ())
    ;
  return tid;
}

SoftUeNetDevice::SoftUeNetDevice ()
  : m_ifIndex (0),
    m_mtu (1500),
    m_linkUp (false),
    m_promisc (false),
    m_localFep (1)
{
  NS_LOG_FUNCTION (this);

  // Initialize random number generator
  m_rng = CreateObject<UniformRandomVariable> ();

  // Initialize ns-3 receive queue
  m_receiveQueue = CreateObject<DropTailQueue<Packet>> ();
  // Use default queue configuration (size is set by MaxSize attribute)

  // Set default configuration
  m_config = SoftUeConfig ();
  m_statistics = SoftUeStats ();
}

SoftUeNetDevice::~SoftUeNetDevice ()
{
  NS_LOG_FUNCTION (this);
}

void
SoftUeNetDevice::DoDispose (void)
{
  NS_LOG_FUNCTION (this);

  // Cancel statistics update
  if (m_statsEventId.IsPending ())
    {
      Simulator::Cancel (m_statsEventId);
    }

  // Clear receive queue using ns-3 interface
  if (m_receiveQueue)
    {
      m_receiveQueue->Flush ();
    }

  // Clear PDCs
  m_pdcs.clear ();

  // Clear components
  m_sesManager = nullptr;
  m_pdsManager = nullptr;
  m_channel = nullptr;
  m_node = nullptr;
  m_rng = nullptr;

  NetDevice::DoDispose ();
}

void
SoftUeNetDevice::DoInitialize (void)
{
  NS_LOG_FUNCTION (this);

  // Initialize protocol stack
  InitializeProtocolStack ();

  // Setup callbacks
  SetupCallbacks ();

  // Start statistics update if enabled
  if (m_config.enableStatistics)
    {
      ScheduleStatisticsUpdate ();
    }

  NetDevice::DoInitialize ();
}

bool
SoftUeNetDevice::Initialize (const SoftUeConfig& config)
{
  NS_LOG_FUNCTION (this);

  m_config = config;
  m_address = config.address;
  m_localFep = config.localFep;
  m_mtu = config.maxPacketSize;

  // Initialize protocol stack if not already done
  if (!m_sesManager || !m_pdsManager)
    {
      InitializeProtocolStack ();
    }

  SetLinkUp (true);

  NS_LOG_INFO ("Soft-Ue network device initialized successfully");
  return true;
}

void
SoftUeNetDevice::SetIfIndex (const uint32_t index)
{
  NS_LOG_FUNCTION (this << index);
  m_ifIndex = index;
}

uint32_t
SoftUeNetDevice::GetIfIndex (void) const
{
  return m_ifIndex;
}

Ptr<Channel>
SoftUeNetDevice::GetChannel (void) const
{
  return m_channel;
}

void
SoftUeNetDevice::SetAddress (Address address)
{
  NS_LOG_FUNCTION (this << address);
  m_address = Mac48Address::ConvertFrom (address);
}

Address
SoftUeNetDevice::GetAddress (void) const
{
  return m_address;
}

bool
SoftUeNetDevice::SetMtu (const uint16_t mtu)
{
  NS_LOG_FUNCTION (this << mtu);
  if (mtu > 0)
    {
      m_mtu = mtu;
      if (mtu > m_config.maxPacketSize)
        {
          m_config.maxPacketSize = mtu;
        }
      return true;
    }
  return false;
}

uint16_t
SoftUeNetDevice::GetMtu (void) const
{
  return m_mtu;
}

bool
SoftUeNetDevice::IsLinkUp (void) const
{
  return m_linkUp;
}

void
SoftUeNetDevice::AddLinkChangeCallback (Callback<void> callback)
{
  NS_LOG_FUNCTION (this << &callback);
  m_linkChangeCallbacks = callback;
}

bool
SoftUeNetDevice::IsBroadcast (void) const
{
  return true;
}

Address
SoftUeNetDevice::GetBroadcast (void) const
{
  return Mac48Address::GetBroadcast ();
}

bool
SoftUeNetDevice::IsMulticast (void) const
{
  return true;
}

Address
SoftUeNetDevice::GetMulticast (Ipv4Address multicastGroup) const
{
  return Mac48Address::GetMulticast (multicastGroup);
}

Address
SoftUeNetDevice::GetMulticast (Ipv6Address addr) const
{
  return Mac48Address::GetMulticast (addr);
}

bool
SoftUeNetDevice::IsPointToPoint (void) const
{
  return false;
}

bool
SoftUeNetDevice::IsBridge (void) const
{
  return false;
}

bool
SoftUeNetDevice::Send (Ptr<Packet> packet, const Address& dest, uint16_t protocolNumber)
{
  NS_LOG_FUNCTION (this << packet << dest << protocolNumber);

  if (!m_linkUp)
    {
      NS_LOG_WARN ("Link is down, dropping packet");
      m_statistics.droppedPackets++;
      return false;
    }

  // Validate packet
  if (!ValidatePacket (packet))
    {
      NS_LOG_WARN ("Invalid packet, dropping");
      m_statistics.droppedPackets++;
      return false;
    }

  // Extract destination FEP from address
  uint32_t destFep = ExtractFepFromAddress (dest);
  if (destFep == 0)
    {
      NS_LOG_WARN ("Invalid destination address, dropping packet");
      m_statistics.droppedPackets++;
      return false;
    }

  // Build metadata and send through SES (SES may fragment transaction into multiple packets)
  Ptr<ExtendedOperationMetadata> meta = Create<ExtendedOperationMetadata> ();
  meta->SetSourceEndpoint (m_localFep, 1);
  meta->SetDestinationEndpoint (destFep, 1);
  meta->payload.length = packet->GetSize ();

  NS_LOG_INFO ("[UEC-E2E] [Device] ⑤ 设备层 Send: FEP " << m_localFep << " → FEP " << destFep
               << " size=" << packet->GetSize () << " B → SES ProcessSendRequest(metadata, packet)");

  bool success = false;
  if (m_sesManager)
    {
      success = m_sesManager->ProcessSendRequest (meta, packet);
    }

  if (!success)
    {
      m_statistics.droppedPackets++;
      NS_LOG_ERROR ("Failed to send packet via SES ProcessSendRequest");
    }

  return success;
}

bool
SoftUeNetDevice::SendFrom (Ptr<Packet> packet, const Address& source, const Address& dest, uint16_t protocolNumber)
{
  NS_LOG_FUNCTION (this << packet << source << dest << protocolNumber);
  return Send (packet, dest, protocolNumber);
}

Ptr<Node>
SoftUeNetDevice::GetNode (void) const
{
  return m_node;
}

void
SoftUeNetDevice::SetNode (Ptr<Node> node)
{
  NS_LOG_FUNCTION (this << node);
  m_node = node;
}

bool
SoftUeNetDevice::NeedsArp (void) const
{
  return true;
}

void
SoftUeNetDevice::SetReceiveCallback (ReceiveCallback cb)
{
  NS_LOG_FUNCTION (this << &cb);
  m_receiveCallback = cb;
}

void
SoftUeNetDevice::SetPromiscReceiveCallback (PromiscReceiveCallback cb)
{
  NS_LOG_FUNCTION (this << &cb);
  m_promiscReceiveCallback = cb;
}

bool
SoftUeNetDevice::SupportsSendFrom () const
{
  return false;
}

bool
SoftUeNetDevice::IsPromisc (void) const
{
  return m_promisc;
}

void
SoftUeNetDevice::SetPromisc (bool promisc)
{
  NS_LOG_FUNCTION (this << promisc);
  m_promisc = promisc;
}

// Soft-Ue specific methods

void
SoftUeNetDevice::SetChannel (Ptr<SoftUeChannel> channel)
{
  NS_LOG_FUNCTION (this << channel);
  m_channel = channel;
}

Ptr<SesManager>
SoftUeNetDevice::GetSesManager (void) const
{
  return m_sesManager;
}

Ptr<PdsManager>
SoftUeNetDevice::GetPdsManager (void) const
{
  return m_pdsManager;
}

uint16_t
SoftUeNetDevice::AllocatePdc (uint32_t destFep, uint8_t tc, uint8_t dm, PDSNextHeader nextHdr)
{
  NS_LOG_FUNCTION (this << destFep << static_cast<int> (tc) << static_cast<int> (dm));

  if (!m_pdsManager)
    {
      return 0;
    }

  return m_pdsManager->AllocatePdc (destFep, tc, dm, nextHdr, 0, 0);
}

bool
SoftUeNetDevice::ReleasePdc (uint16_t pdcId)
{
  NS_LOG_FUNCTION (this << pdcId);

  if (!m_pdsManager)
    {
      return false;
    }

  return m_pdsManager->ReleasePdc (pdcId);
}

bool
SoftUeNetDevice::SendPacketThroughPdc (uint16_t pdcId, Ptr<Packet> packet, bool som, bool eom)
{
  NS_LOG_FUNCTION (this << pdcId << packet << som << eom);

  if (!m_pdsManager)
    {
      return false;
    }

  return m_pdsManager->SendPacketThroughPdc (pdcId, packet, som, eom);
}

void
SoftUeNetDevice::ReceivePacket (Ptr<Packet> packet, uint32_t sourceFep, uint32_t destFep)
{
  NS_LOG_FUNCTION (this << packet << sourceFep << destFep);

  if (destFep != m_localFep)
    {
      NS_LOG_DEBUG ("Packet not for this device, ignoring");
      return;
    }

  NS_LOG_INFO ("[UEC-E2E] [Device] ⑥ 设备层 ReceivePacket: FEP " << sourceFep << " → FEP " << destFep
               << " size=" << packet->GetSize () << " B → 交 PDS Manager ProcessReceivedPacket");

  // Receive path through PDS Manager: parse PDS header, dispatch to PDC, then deliver to app
  if (m_pdsManager)
    {
      m_pdsManager->ProcessReceivedPacket (packet, sourceFep, destFep);
    }
  else
    {
      m_statistics.droppedPackets++;
      NS_LOG_WARN ("No PDS Manager, dropping packet");
    }
}

SoftUeStats
SoftUeNetDevice::GetStatistics (void) const
{
  SoftUeStats stats = m_statistics;
  stats.activePdcCount = m_pdsManager ? m_pdsManager->GetTotalActivePdcCount () : 0;
  return stats;
}

void
SoftUeNetDevice::ResetStatistics (void)
{
  NS_LOG_FUNCTION (this);
  m_statistics = SoftUeStats ();
  if (m_pdsManager)
    {
      m_pdsManager->ResetStatistics ();
    }
}

SoftUeConfig
SoftUeNetDevice::GetConfiguration (void) const
{
  return m_config;
}

bool
SoftUeNetDevice::UpdateConfiguration (const SoftUeConfig& config)
{
  NS_LOG_FUNCTION (this);

  m_config = config;
  m_address = config.address;
  m_localFep = config.localFep;

  return true;
}

uint32_t
SoftUeNetDevice::GetActivePdcCount (void) const
{
  return m_pdsManager ? m_pdsManager->GetTotalActivePdcCount () : 0;
}

void
SoftUeNetDevice::SetLinkUp (bool up)
{
  NS_LOG_FUNCTION (this << up);

  if (m_linkUp != up)
    {
      m_linkUp = up;
      NotifyLinkChange ();
    }
}

// Private helper methods

void
SoftUeNetDevice::InitializeProtocolStack (void)
{
  NS_LOG_FUNCTION (this);

  // Create SES manager
  m_sesManager = CreateObject<SesManager> ();
  m_sesManager->SetNetDevice (this);

  // Create PDS manager
  m_pdsManager = CreateObject<PdsManager> ();
  m_pdsManager->SetSesManager (m_sesManager);
  m_pdsManager->SetNetDevice (this);

  // Initialize managers
  m_sesManager->Initialize ();
  m_pdsManager->Initialize ();

  NS_LOG_INFO ("Ultra Ethernet protocol stack initialized");
}

void
SoftUeNetDevice::SetupCallbacks (void)
{
  NS_LOG_FUNCTION (this);

  if (m_pdsManager)
    {
      // Setup PDC creation callback
      m_pdcCreatedCallback = MakeCallback (&SoftUeNetDevice::OnPdcCreated, this);
      m_pdcDestroyedCallback = MakeCallback (&SoftUeNetDevice::OnPdcDestroyed, this);
    }
}

void
SoftUeNetDevice::ProcessReceiveQueue (void)
{
  NS_LOG_FUNCTION (this);

  while (m_receiveQueue->GetNPackets () > 0)
    {
      Ptr<Packet> packet = m_receiveQueue->Dequeue ();
      if (!packet)
        {
          break;
        }

      // Call receive callback if set
      if (!m_receiveCallback.IsNull ())
        {
          NS_LOG_INFO ("[UEC-E2E] [Device] ⑦ 设备层 ProcessReceiveQueue: 递交应用层 HandleRead");
          m_receiveCallback (this, packet, 0x0800, CreateAddressFromFep (m_localFep));
        }
    }
}

void
SoftUeNetDevice::UpdateStatistics (void)
{
  NS_LOG_FUNCTION (this);

  // Calculate throughput based on recent activity
  Time now = Simulator::Now ();
  if (m_statistics.lastActivity > Seconds (0))
    {
      Time deltaTime = now - m_statistics.lastActivity;
      if (deltaTime > Seconds (0))
        {
          double deltaTimeSeconds = deltaTime.GetSeconds ();
          if (deltaTimeSeconds > 0.001) // Avoid division by very small numbers
            {
              m_statistics.throughput = (m_statistics.totalBytesTransmitted * 8.0)
                                      / deltaTimeSeconds / 1e6; // Mbps
            }
          else
            {
              m_statistics.throughput = 0.0; // Default if time interval too small
            }
        }
    }

  // Update average latency (simplified)
  if (m_statistics.totalPacketsReceived + m_statistics.totalPacketsTransmitted > 0)
    {
      m_statistics.averageLatency = 0.5; // ms (placeholder calculation)
    }

  // Update active PDC count
  if (m_pdsManager)
    {
      m_statistics.activePdcCount = m_pdsManager->GetTotalActivePdcCount ();
    }

  // Trace statistics
  m_statsTrace (m_statistics);
}

void
SoftUeNetDevice::ScheduleStatisticsUpdate (void)
{
  if (m_statsEventId.IsPending ())
    {
      Simulator::Cancel (m_statsEventId);
    }

  m_statsEventId = Simulator::Schedule (m_config.statsUpdateInterval,
                                         &SoftUeNetDevice::UpdateStatistics, this);
}

bool
SoftUeNetDevice::ValidatePacket (Ptr<Packet> packet) const
{
  if (!packet)
    {
      return false;
    }

  uint32_t packetSize = packet->GetSize ();
  if (packetSize == 0 || packetSize > m_mtu)
    {
      return false;
    }

  return true;
}

void
SoftUeNetDevice::LogDetailed (const std::string& function, const std::string& message) const
{
  if (m_config.enableDetailedLogging)
    {
      NS_LOG_DEBUG ("SoftUeNetDevice::" << function << ": " << message);
    }
}

bool
SoftUeNetDevice::HandleProtocolError (const std::string& error, const std::string& details)
{
  NS_LOG_FUNCTION (this << error << details);

  m_errorTrace (error, details);

  LogDetailed ("HandleProtocolError", "Error: " + error + " - " + details);

  return true;
}

void
SoftUeNetDevice::NotifyLinkChange (void)
{
  if (!m_linkChangeCallbacks.IsNull ())
    {
      m_linkChangeCallbacks ();
    }
}

uint32_t
SoftUeNetDevice::GetLocalFep (void) const
{
  return m_localFep;
}

bool
SoftUeNetDevice::TransmitToChannel (Ptr<Packet> packet, uint32_t srcFep, uint32_t destFep)
{
  NS_LOG_FUNCTION (this << packet << srcFep << destFep);
  if (!packet || !m_channel)
    {
      return false;
    }
  m_channel->Transmit (packet, this, srcFep, destFep);
  m_statistics.totalPacketsTransmitted++;
  m_statistics.totalBytesTransmitted += packet->GetSize ();
  m_statistics.lastActivity = Simulator::Now ();
  // PDS stats updated in SendPacketThroughPdc / ProcessReceivedPacket to avoid double-count
  m_macTxTrace (packet, CreateAddressFromFep (destFep));
  return true;
}

void
SoftUeNetDevice::DeliverReceivedPacket (Ptr<Packet> packet)
{
  NS_LOG_FUNCTION (this << packet);
  if (!packet)
    {
      return;
    }
  m_statistics.totalPacketsReceived++;
  m_statistics.totalBytesReceived += packet->GetSize ();
  m_statistics.lastActivity = Simulator::Now ();
  m_macRxTrace (packet, CreateAddressFromFep (m_localFep));
  bool enqueued = m_receiveQueue->Enqueue (packet);
  if (!enqueued)
    {
      m_statistics.droppedPackets++;
      NS_LOG_WARN ("DeliverReceivedPacket: receive queue full, dropping packet");
      return;
    }
  ProcessReceiveQueue ();
}

uint16_t
SoftUeNetDevice::ExtractFepFromAddress (const Address& addr) const
{
  // Simple mapping: use last 2 bytes of MAC address as FEP
  Mac48Address macAddr = Mac48Address::ConvertFrom (addr);
  uint8_t buffer[6];
  macAddr.CopyTo (buffer);

  return (static_cast<uint16_t> (buffer[4]) << 8) | buffer[5];
}

Address
SoftUeNetDevice::CreateAddressFromFep (uint32_t fep) const
{
  // Simple mapping: embed FEP in MAC address
  uint8_t buffer[6];

  // Use base MAC address and replace last 2 bytes with FEP
  m_address.CopyTo (buffer);
  buffer[4] = (fep >> 8) & 0xFF;
  buffer[5] = fep & 0xFF;

  Mac48Address targetAddr;
  targetAddr.CopyFrom (buffer);
  return targetAddr;
}

void
SoftUeNetDevice::OnPdcCreated (uint16_t pdcId, PdcType type)
{
  NS_LOG_FUNCTION (this << pdcId << static_cast<int> (type));
  LogDetailed ("OnPdcCreated", "PDC created: " + std::to_string (pdcId)
               + ", type: " + std::to_string (static_cast<int> (type)));
}

void
SoftUeNetDevice::OnPdcDestroyed (uint16_t pdcId)
{
  NS_LOG_FUNCTION (this << pdcId);
  LogDetailed ("OnPdcDestroyed", "PDC destroyed: " + std::to_string (pdcId));
}

// Attribute getter/setter methods
bool
SoftUeNetDevice::GetEnableStatistics (void) const
{
  return m_config.enableStatistics;
}

void
SoftUeNetDevice::SetEnableStatistics (bool enable)
{
  m_config.enableStatistics = enable;
}

uint32_t
SoftUeNetDevice::GetMaxPdcCount (void) const
{
  return m_config.maxPdcCount;
}

void
SoftUeNetDevice::SetMaxPdcCount (uint32_t count)
{
  m_config.maxPdcCount = count;
}

Time
SoftUeNetDevice::GetProcessingInterval (void) const
{
  return m_config.processingInterval;
}

void
SoftUeNetDevice::SetProcessingInterval (Time interval)
{
  m_config.processingInterval = interval;
}

uint16_t
SoftUeNetDevice::GetMaxPacketSize (void) const
{
  return m_config.maxPacketSize;
}

void
SoftUeNetDevice::SetMaxPacketSize (uint16_t size)
{
  m_config.maxPacketSize = size;
  m_mtu = size;
}

} // namespace ns3

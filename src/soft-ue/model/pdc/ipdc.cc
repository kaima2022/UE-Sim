#include "ipdc.h"
#include "../pds/pds-common.h"
#include "../network/soft-ue-net-device.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/nstime.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include "ns3/boolean.h"
#include <algorithm>
#include <queue>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("Ipdc");
NS_OBJECT_ENSURE_REGISTERED (Ipdc);

TypeId
Ipdc::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::Ipdc")
    .SetParent<PdcBase> ()
    .SetGroupName ("Soft-Ue")
    .AddConstructor<Ipdc> ()
    // TODO: Simplify attributes to avoid complex struct accessors
    // For now, we'll skip these attributes to focus on basic compilation
    .AddAttribute ("EnableFlowControl",
                    "Enable flow control",
                    BooleanValue (true),
                    MakeBooleanAccessor (&Ipdc::IsFlowControlEnabled,
                                        &Ipdc::SetFlowControlEnabled),
                    MakeBooleanChecker ())
    .AddAttribute ("PacketLifetime",
                    "Lifetime of queued packets",
                    TimeValue (Seconds (10)),
                    MakeTimeAccessor (&Ipdc::GetPacketLifetime,
                                      &Ipdc::SetPacketLifetime),
                    MakeTimeChecker ())
    // TODO: Add proper trace sources later after basic compilation works
    ;
  return tid;
}

TypeId
Ipdc::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

Ipdc::Ipdc ()
  : PdcBase (),
    m_nextSequenceNumber (1),
    m_sendCredits (64),
    m_receiveCredits (64),
    m_flowControlActive (false),
    m_queueCleanupInterval (MilliSeconds (100))
{
  NS_LOG_FUNCTION (this);

  // Initialize IPDC-specific configuration
  m_ipdcConfig = IpdcConfig ();
  m_ipdcStatistics = IpdcStatistics ();
}

Ipdc::~Ipdc ()
{
  NS_LOG_FUNCTION (this);
}

void
Ipdc::DoDispose (void)
{
  NS_LOG_FUNCTION (this);

  // Cancel queue cleanup
  if (m_queueCleanupEventId.IsPending ())
  {
    Simulator::Cancel (m_queueCleanupEventId);
  }

  // Clear queues
  ClearSendQueue ();
  ClearReceiveQueue ();

  PdcBase::DoDispose ();
}

bool
Ipdc::Initialize (const PdcConfig& config)
{
  NS_LOG_FUNCTION (this << "Initializing IPDC " << config.pdcId);

  // Initialize base class
  if (!PdcBase::Initialize (config))
  {
    return false;
  }

  // Convert to IPDC config
  try
    {
      m_ipdcConfig = static_cast<const IpdcConfig&>(config);
    }
  catch (const std::bad_cast& e)
    {
      NS_LOG_WARN ("Config is not IpdcConfig, using defaults");
      m_ipdcConfig = IpdcConfig ();
      m_ipdcConfig.pdcId = config.pdcId;
      m_ipdcConfig.localFep = config.localFep;
      m_ipdcConfig.remoteFep = config.remoteFep;
      m_ipdcConfig.tc = config.tc;
      m_ipdcConfig.deliveryMode = config.deliveryMode;
      m_ipdcConfig.maxPacketSize = config.maxPacketSize;
      m_ipdcConfig.rtoPdcContext = config.rtoPdcContext;
      m_ipdcConfig.rtoCccContext = config.rtoCccContext;
    }

  // Initialize flow control credits
  m_sendCredits = m_ipdcConfig.sendWindow;
  m_receiveCredits = m_ipdcConfig.receiveWindow;
  m_flowControlActive = false;

  // Start queue cleanup
  ScheduleQueueCleanup ();

  NS_LOG_INFO ("IPDC " << m_ipdcConfig.pdcId << " initialized successfully");
  return true;
}

bool
Ipdc::SendPacket (Ptr<Packet> packet, bool som, bool eom)
{
  NS_LOG_FUNCTION (this << "Sending packet, size: " << packet->GetSize ());

  if (!packet || !IsActive ())
    {
      HandleError (PdsErrorCode::INVALID_PACKET, "Null packet or inactive IPDC");
      return false;
    }

  // Validate packet
  if (!ValidatePacket (packet, true))
    {
      HandleError (PdsErrorCode::INVALID_PACKET, "Packet validation failed");
      return false;
    }

  // Check flow control
  if (!CheckFlowControl ())
    {
      m_ipdcStatistics.flowControlEvents++;
      m_packetDropTrace (packet, "Flow control active");
      return false;
    }

  // Immediate send path: transmit directly (no queue drain in simulation)
  QueuedPacket qp (packet, som, eom, GenerateSequenceNumber ());
  bool success = TransmitPacket (qp);
  if (success)
    {
      UpdateStatistics (true, packet);
      m_packetTxTrace (packet, m_ipdcConfig.pdcId);
    }
  return success;
}

bool
Ipdc::HandleReceivedPacket (Ptr<Packet> packet, uint32_t sourceFep)
{
  NS_LOG_FUNCTION (this << "Handling received packet from FEP " << sourceFep);

  if (!packet || !IsActive ())
    {
      HandleError (PdsErrorCode::INVALID_PACKET, "Null packet or inactive IPDC");
      return false;
    }

  // Let base class handle basic validation
  if (!PdcBase::HandleReceivedPacket (packet, sourceFep))
    {
      return false;
    }

  // Validate packet for IPDC
  if (!ValidatePacket (packet, false))
    {
      HandleError (PdsErrorCode::INVALID_PACKET, "IPDC packet validation failed");
      return false;
    }

  // Enqueue packet for receiving
  return EnqueuePacket (m_receiveQueue, packet, false, false,
                       m_ipdcConfig.maxReceiveQueueSize);
}

IpdcStatistics
Ipdc::GetIpdcStatistics (void) const
{
  // Update current queue sizes
  IpdcStatistics stats = m_ipdcStatistics;
  stats.currentSendQueueSize = m_sendQueue.size ();
  stats.currentReceiveQueueSize = m_receiveQueue.size ();

  return stats;
}

void
Ipdc::ResetIpdcStatistics (void)
{
  NS_LOG_FUNCTION (this);
  m_ipdcStatistics = IpdcStatistics ();
}

uint32_t
Ipdc::GetSendQueueSize (void) const
{
  return m_sendQueue.size ();
}

uint32_t
Ipdc::GetReceiveQueueSize (void) const
{
  return m_receiveQueue.size ();
}

IpdcConfig
Ipdc::GetIpdcConfiguration (void) const
{
  return m_ipdcConfig;
}

bool
Ipdc::UpdateIpdcConfiguration (const IpdcConfig& config)
{
  NS_LOG_FUNCTION (this << "Updating IPDC configuration");

  if (config.pdcId != m_ipdcConfig.pdcId)
    {
      NS_LOG_ERROR ("Cannot change PDC ID after initialization");
      return false;
    }

  m_ipdcConfig = config;

  // Update flow control credits
  m_sendCredits = std::min (m_sendCredits, m_ipdcConfig.sendWindow);
  m_receiveCredits = std::min (m_receiveCredits, m_ipdcConfig.receiveWindow);

  return true;
}

bool
Ipdc::SendPacketDirect (Ptr<Packet> packet, bool som, bool eom)
{
  NS_LOG_FUNCTION (this << "Sending packet directly");

  if (!packet || !IsActive ())
    {
      return false;
    }

  // Create queued packet for transmission
  QueuedPacket qp (packet, som, eom, GenerateSequenceNumber ());

  // Transmit immediately
  bool success = TransmitPacket (qp);

  if (success)
    {
      UpdateStatistics (true, packet);
      m_packetTxTrace (packet, m_ipdcConfig.pdcId);
    }

  return success;
}

uint32_t
Ipdc::ClearSendQueue (void)
{
  uint32_t count = m_sendQueue.size ();
  while (!m_sendQueue.empty ())
    {
      m_sendQueue.pop ();
    }
  return count;
}

uint32_t
Ipdc::ClearReceiveQueue (void)
{
  uint32_t count = m_receiveQueue.size ();
  while (!m_receiveQueue.empty ())
    {
      m_receiveQueue.pop ();
    }
  return count;
}

bool
Ipdc::ValidatePacket (Ptr<Packet> packet, bool isSend) const
{
  if (!PdcBase::ValidatePacket (packet, isSend))
    {
      return false;
    }

  uint32_t packetSize = packet->GetSize ();

  // Additional IPDC-specific validation
  if (packetSize > m_ipdcConfig.maxPacketSize)
    {
      LogDetailed ("ValidatePacket", "Packet size exceeds IPDC limit: " +
                   std::to_string (packetSize));
      return false;
    }

  return true;
}

bool
Ipdc::HandleError (PdsErrorCode error, const std::string& details)
{
  NS_LOG_FUNCTION (this << "IPDC Error: " << static_cast<int> (error) << " - " << details);

  // Update IPDC-specific error statistics
  m_ipdcStatistics.errors++;

  // Handle specific IPDC errors
  switch (error)
    {
    case PdsErrorCode::PDC_FULL:
      NS_LOG_WARN ("IPDC queue full, consider increasing queue sizes");
      break;
    case PdsErrorCode::RESOURCE_EXHAUSTED:
      NS_LOG_WARN ("IPDC resources exhausted, system may need scaling");
      break;
    default:
      break;
    }

  // Let base class handle common error processing
  return PdcBase::HandleError (error, details);
}

bool
Ipdc::EnqueuePacket (std::queue<QueuedPacket>& queue, Ptr<Packet> packet,
                    bool som, bool eom, uint32_t maxSize)
{
  if (queue.size () >= maxSize)
    {
      // Queue is full, drop oldest packet if enabled
      if (m_ipdcConfig.enableFlowControl)
        {
          // Drop oldest packet
          queue.pop ();
          m_ipdcStatistics.packetsDropped++;
          m_packetDropTrace (packet, "Queue overflow - dropped oldest");
        }
      else
        {
          // Reject new packet
          m_ipdcStatistics.packetsDropped++;
          m_packetDropTrace (packet, "Queue overflow - packet rejected");
          return false;
        }
    }

  // Add packet to queue
  QueuedPacket qp (packet, som, eom, GenerateSequenceNumber ());
  queue.push (qp);

  // Trace queue size changes
  if (&queue == &m_sendQueue)
    {
      m_queueTrace (queue.size (), m_receiveQueue.size ());
    }
  else
    {
      m_queueTrace (m_sendQueue.size (), queue.size ());
    }

  LogDetailed ("EnqueuePacket", "Packet enqueued, queue size: " +
               std::to_string (queue.size ()));

  return true;
}

bool
Ipdc::DequeuePacket (std::queue<QueuedPacket>& queue, QueuedPacket& qp)
{
  if (queue.empty ())
    {
      return false;
    }

  qp = queue.front ();
  queue.pop ();

  // Update average queue time
  Time queueTime = Simulator::Now () - qp.timestamp;
  m_ipdcStatistics.averageQueueTime =
      (m_ipdcStatistics.averageQueueTime + queueTime) / 2;

  return true;
}

void
Ipdc::ProcessSendQueue (void)
{
  NS_LOG_FUNCTION (this);

  while (!m_sendQueue.empty () && CheckFlowControl ())
    {
      QueuedPacket qp;
      if (!DequeuePacket (m_sendQueue, qp))
        {
          break;
        }

      // Transmit packet
      bool success = TransmitPacket (qp);

      if (success)
        {
          UpdateStatistics (true, qp.packet);
          m_packetTxTrace (qp.packet, m_ipdcConfig.pdcId);
          m_sendCredits--;
        }
      else
        {
          NS_LOG_WARN ("Packet transmission failed");
          HandleError (PdsErrorCode::PROTOCOL_ERROR, "Packet transmission failed");
        }
    }

  // Trace queue sizes
  m_queueTrace (m_sendQueue.size (), m_receiveQueue.size ());
}

void
Ipdc::ProcessReceiveQueue (void)
{
  NS_LOG_FUNCTION (this);

  while (!m_receiveQueue.empty ())
    {
      QueuedPacket qp;
      if (!DequeuePacket (m_receiveQueue, qp))
        {
          break;
        }

      // Process received packet
      UpdateStatistics (false, qp.packet);
      m_packetRxTrace (qp.packet, m_ipdcConfig.pdcId);

      // Update receive credits
      if (m_receiveCredits < m_ipdcConfig.receiveWindow)
        {
          m_receiveCredits++;
        }

      LogDetailed ("ProcessReceiveQueue", "Processed received packet, seq: " +
                   std::to_string (qp.sequenceNumber));
    }

  // Update flow control
  UpdateFlowControl ();

  // Trace queue sizes
  m_queueTrace (m_sendQueue.size (), m_receiveQueue.size ());
}

void
Ipdc::UpdateFlowControl (void)
{
  bool wasActive = m_flowControlActive;

  // Simple flow control: activate when send credits are low
  m_flowControlActive = (m_sendCredits < (m_ipdcConfig.sendWindow / 4));

  if (m_flowControlActive != wasActive)
    {
      NS_LOG_INFO ("Flow control " << (m_flowControlActive ? "activated" : "deactivated"));
      m_ipdcStatistics.flowControlEvents++;
    }

  // Gradually restore send credits
  if (!m_flowControlActive && m_sendCredits < m_ipdcConfig.sendWindow)
    {
      m_sendCredits++;
    }
}

void
Ipdc::CleanupExpiredPackets (void)
{
  NS_LOG_FUNCTION (this);

  Time now = Simulator::Now ();
  uint32_t cleaned = 0;

  // Clean send queue
  std::queue<QueuedPacket> newSendQueue;
  while (!m_sendQueue.empty ())
    {
      QueuedPacket qp = m_sendQueue.front ();
      m_sendQueue.pop ();

      if ((now - qp.timestamp) < m_ipdcConfig.packetLifetime)
        {
          newSendQueue.push (qp);
        }
      else
        {
          cleaned++;
          m_ipdcStatistics.packetsExpired++;
          m_packetDropTrace (qp.packet, "Packet expired");
        }
    }
  m_sendQueue = newSendQueue;

  // Clean receive queue
  std::queue<QueuedPacket> newReceiveQueue;
  while (!m_receiveQueue.empty ())
    {
      QueuedPacket qp = m_receiveQueue.front ();
      m_receiveQueue.pop ();

      if ((now - qp.timestamp) < m_ipdcConfig.packetLifetime)
        {
          newReceiveQueue.push (qp);
        }
      else
        {
          cleaned++;
          m_ipdcStatistics.packetsExpired++;
          m_packetDropTrace (qp.packet, "Packet expired");
        }
    }
  m_receiveQueue = newReceiveQueue;

  if (cleaned > 0)
    {
      LogDetailed ("CleanupExpiredPackets", "Cleaned " + std::to_string (cleaned) + " expired packets");
    }
}

void
Ipdc::ScheduleQueueCleanup (void)
{
  if (!m_queueCleanupEventId.IsPending ())
    {
      m_queueCleanupEventId = Simulator::Schedule (m_queueCleanupInterval,
                                                 &Ipdc::CleanupExpiredPackets, this);
    }
}

bool
Ipdc::CheckFlowControl (void) const
{
  if (!m_ipdcConfig.enableFlowControl)
    {
      return true;
    }

  return !m_flowControlActive && (m_sendCredits > 0);
}

void
Ipdc::UpdateQueueStatistics (void)
{
  m_ipdcStatistics.currentSendQueueSize = m_sendQueue.size ();
  m_ipdcStatistics.currentReceiveQueueSize = m_receiveQueue.size ();
}

bool
Ipdc::TransmitPacket (const QueuedPacket& qp)
{
  NS_LOG_FUNCTION (this << "Transmitting packet, seq: " << qp.sequenceNumber);

  if (!GetNetDevice ())
    {
      NS_LOG_WARN ("No network device available for transmission");
      return false;
    }

  // Create PDS header
  PDSHeader header = CreatePdsHeader (qp.packet, qp.som, qp.eom);
  NS_LOG_DEBUG ("Created PDS header for packet transmission");

  Ptr<Packet> packetToSend = qp.packet->Copy ();
  packetToSend->AddHeader (header);

  NS_LOG_INFO ("[UEC-E2E] [PDC] PDC pdc_id=" << m_ipdcConfig.pdcId
               << " TransmitPacket → TransmitToChannel（到信道）");

  // Transmit directly to channel (bypasses PDS Manager / DispatchPacket to avoid recursion)
  bool success = GetNetDevice ()->TransmitToChannel (packetToSend, GetLocalFep (), m_ipdcConfig.remoteFep);

  if (success)
    {
      LogDetailed ("TransmitPacket", "Packet transmitted successfully to FEP " +
                   std::to_string (m_ipdcConfig.remoteFep));
    }
  else
    {
      LogDetailed ("TransmitPacket", "Packet transmission failed");
    }

  return success;
}

void
Ipdc::HandlePacketDelivery (Ptr<Packet> packet, bool success)
{
  NS_LOG_FUNCTION (this << "Packet delivery result: " << (success ? "success" : "failed"));

  if (success)
    {
      // Packet delivered successfully
      UpdateStatistics (true, packet);
      m_packetTxTrace (packet, m_ipdcConfig.pdcId);
    }
  else
    {
      // Packet delivery failed
      m_ipdcStatistics.packetsDropped++;
      HandleError (PdsErrorCode::PROTOCOL_ERROR, "Packet delivery failed");
    }
}

uint32_t
Ipdc::GenerateSequenceNumber (void)
{
  return m_nextSequenceNumber++;
}

bool
Ipdc::IsFlowControlEnabled (void) const
{
  return m_ipdcConfig.enableFlowControl;
}

void
Ipdc::SetFlowControlEnabled (bool enabled)
{
  m_ipdcConfig.enableFlowControl = enabled;
}

Time
Ipdc::GetPacketLifetime (void) const
{
  return m_ipdcConfig.packetLifetime;
}

void
Ipdc::SetPacketLifetime (Time lifetime)
{
  m_ipdcConfig.packetLifetime = lifetime;
}

Address
Ipdc::ConvertFepToAddress (uint32_t fep)
{
  NS_LOG_FUNCTION (this << fep);

  // Convert FEP to MAC address format
  // For simplicity, we'll use the FEP as the lower 32 bits of a 48-bit MAC address
  uint8_t macAddr[6];

  // Use a fixed OUI (Organizationally Unique Identifier) for Soft-UE
  macAddr[0] = 0x02; // Locally administered
  macAddr[1] = 0x00; // Soft-UE OUI part 1
  macAddr[2] = 0x5E; // Soft-UE OUI part 2

  // Encode the FEP in the remaining 4 bytes (use upper 32 bits of FEP)
  macAddr[3] = (fep >> 24) & 0xFF;
  macAddr[4] = (fep >> 16) & 0xFF;
  macAddr[5] = (fep >> 8) & 0xFF;

  Mac48Address mac48Addr = Mac48Address ();
  mac48Addr.CopyFrom (macAddr);
  Address address = mac48Addr;

  NS_LOG_DEBUG ("Converted FEP " << fep << " to MAC address " << address);

  return address;
}

} // namespace ns3

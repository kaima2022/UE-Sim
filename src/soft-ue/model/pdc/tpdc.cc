#include "tpdc.h"
#include "../network/soft-ue-net-device.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/nstime.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include "ns3/boolean.h"
#include <algorithm>
#include <unordered_map>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("Tpdc");
NS_OBJECT_ENSURE_REGISTERED (Tpdc);

TypeId
Tpdc::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::Tpdc")
    .SetParent<PdcBase> ()
    .SetGroupName ("Soft-Ue")
    .AddConstructor<Tpdc> ()
    // TODO: Add proper getter/setter methods for TPDC attributes
    // For now, we'll skip these attributes to focus on basic compilation
    // TODO: Add proper trace sources later after basic compilation works
    ;
  return tid;
}

TypeId
Tpdc::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

Tpdc::Tpdc ()
  : PdcBase (),
    m_nextSendSequence (1),
    m_nextReceiveSequence (1),
    m_sendWindowBase (1),
    m_receiveWindowBase (1),
    m_ackInterval (MilliSeconds (50)),
    m_currentRto (MilliSeconds (200)),
    m_currentRtt (MilliSeconds (0)),
    m_rttVariance (MilliSeconds (0))
{
  NS_LOG_FUNCTION (this);

  // Initialize TPDC-specific configuration
  m_tpdcConfig = TpdcConfig ();
  m_tpdcStatistics = TpdcStatistics ();
}

Tpdc::~Tpdc ()
{
  NS_LOG_FUNCTION (this);
}

void
Tpdc::DoDispose (void)
{
  NS_LOG_FUNCTION (this);

  // Cancel ACK event
  if (m_ackEventId.IsPending ())
    {
      Simulator::Cancel (m_ackEventId);
    }

  // Clear buffers
  ClearSendBuffer ();
  ClearReceiveBuffer ();

  // Clear queues
  while (!m_sendQueue.empty ())
    {
      m_sendQueue.pop ();
    }
  while (!m_ackQueue.empty ())
    {
      m_ackQueue.pop ();
    }

  PdcBase::DoDispose ();
}

bool
Tpdc::Initialize (const PdcConfig& config)
{
  NS_LOG_FUNCTION (this << "Initializing TPDC " << config.pdcId);

  // Initialize base class
  if (!PdcBase::Initialize (config))
    {
      return false;
    }

  // Convert to TPDC config
  try
    {
      m_tpdcConfig = static_cast<const TpdcConfig&>(config);
    }
  catch (const std::bad_cast& e)
    {
      NS_LOG_WARN ("Config is not TpdcConfig, using defaults");
      m_tpdcConfig = TpdcConfig ();
      m_tpdcConfig.pdcId = config.pdcId;
      m_tpdcConfig.localFep = config.localFep;
      m_tpdcConfig.remoteFep = config.remoteFep;
      m_tpdcConfig.tc = config.tc;
      m_tpdcConfig.deliveryMode = config.deliveryMode;
      m_tpdcConfig.maxPacketSize = config.maxPacketSize;
      m_tpdcConfig.rtoPdcContext = config.rtoPdcContext;
      m_tpdcConfig.rtoCccContext = config.rtoCccContext;
    }

  // Initialize sequence numbers and windows
  m_nextSendSequence = 1;
  m_nextReceiveSequence = 1;
  m_sendWindowBase = 1;
  m_receiveWindowBase = 1;

  // Initialize RTO
  m_currentRto = m_tpdcConfig.initialRto;

  // Start ACK scheduling
  ScheduleAcknowledgment ();

  NS_LOG_INFO ("TPDC " << m_tpdcConfig.pdcId << " initialized successfully");
  return true;
}

bool
Tpdc::SendPacket (Ptr<Packet> packet, bool som, bool eom)
{
  NS_LOG_FUNCTION (this << "Sending packet, size: " << packet->GetSize ());

  if (!packet || !IsActive ())
    {
      HandleError (PdsErrorCode::INVALID_PACKET, "Null packet or inactive TPDC");
      return false;
    }

  // Validate packet
  if (!ValidatePacket (packet, true))
    {
      HandleError (PdsErrorCode::INVALID_PACKET, "Packet validation failed");
      return false;
    }

  // Buffer packet for sending
  return BufferPacketForSending (packet, som, eom);
}

bool
Tpdc::HandleReceivedPacket (Ptr<Packet> packet, uint32_t sourceFep)
{
  NS_LOG_FUNCTION (this << "Handling received packet from FEP " << sourceFep);

  if (!packet || !IsActive ())
    {
      HandleError (PdsErrorCode::INVALID_PACKET, "Null packet or inactive TPDC");
      return false;
    }

  // Let base class handle basic validation
  if (!PdcBase::HandleReceivedPacket (packet, sourceFep))
    {
      return false;
    }

  // Validate packet for TPDC
  if (!ValidatePacket (packet, false))
    {
      HandleError (PdsErrorCode::INVALID_PACKET, "TPDC packet validation failed");
      return false;
    }

  // Extract sequence number from packet (simplified)
  uint32_t seqNum = m_nextReceiveSequence;

  // Buffer packet for receiving
  return BufferPacketForReceiving (packet, seqNum);
}

TpdcStatistics
Tpdc::GetTpdcStatistics (void) const
{
  // Update current buffer sizes
  TpdcStatistics stats = m_tpdcStatistics;
  stats.currentSendBufferSize = m_sendBuffer.size ();
  stats.currentReceiveBufferSize = m_receiveBuffer.size ();
  stats.averageRto = m_currentRto.GetMilliSeconds ();
  stats.averageRoundTripTime = m_currentRtt;

  return stats;
}

void
Tpdc::ResetTpdcStatistics (void)
{
  NS_LOG_FUNCTION (this);
  m_tpdcStatistics = TpdcStatistics ();
}

uint32_t
Tpdc::GetSendBufferSize (void) const
{
  return m_sendBuffer.size ();
}

uint32_t
Tpdc::GetReceiveBufferSize (void) const
{
  return m_receiveBuffer.size ();
}

TpdcConfig
Tpdc::GetTpdcConfiguration (void) const
{
  return m_tpdcConfig;
}

bool
Tpdc::UpdateTpdcConfiguration (const TpdcConfig& config)
{
  NS_LOG_FUNCTION (this << "Updating TPDC configuration");

  if (config.pdcId != m_tpdcConfig.pdcId)
    {
      NS_LOG_ERROR ("Cannot change PDC ID after initialization");
      return false;
    }

  m_tpdcConfig = config;

  // Update RTO if changed
  if (config.initialRto != m_currentRto)
    {
      m_currentRto = config.initialRto;
    }

  return true;
}

bool
Tpdc::SendAcknowledgment (const Acknowledgment& ack)
{
  NS_LOG_FUNCTION (this << "Sending ACK for sequence " << ack.ackSequence);

  // In a real implementation, this would send the ACK packet
  // For simulation, we just update statistics and trace
  m_tpdcStatistics.acknowledgmentsSent++;
  if (ack.cumulative)
    {
      m_tpdcStatistics.cumulativeAcksSent++;
    }

  m_ackTrace (ack);

  LogDetailed ("SendAcknowledgment", "ACK sent for sequence " +
               std::to_string (ack.ackSequence));

  return true;
}

bool
Tpdc::ProcessReceivedAcknowledgment (const Acknowledgment& ack)
{
  NS_LOG_FUNCTION (this << "Processing ACK for sequence " << ack.ackSequence);

  m_tpdcStatistics.acknowledgmentsReceived++;

  // Check if this is a duplicate ACK
  if (ack.ackSequence < m_sendWindowBase)
    {
      m_tpdcStatistics.duplicateAcksReceived++;
      return false;
    }

  // Update send window base
  uint32_t oldBase = m_sendWindowBase;
  m_sendWindowBase = ack.ackSequence + 1;

  // Mark acknowledged packets
  for (uint32_t seq = oldBase; seq < ack.ackSequence; seq++)
    {
      auto it = m_sendBuffer.find (seq);
      if (it != m_sendBuffer.end ())
        {
          it->second.acknowledged = true;

          // Calculate RTT if this is the first time acknowledging
          Time measuredRtt = Seconds (0);
          if (m_currentRtt.IsZero ())
            {
              measuredRtt = Simulator::Now () - it->second.timestamp;
              UpdateRtoEstimate (measuredRtt);
            }

          m_rttTrace (m_currentRtt, measuredRtt);
        }
    }

  // Cleanup acknowledged packets
  CleanupAcknowledgedPackets ();

  // Update receive window
  m_receiveWindowBase = ack.receiveWindow;

  // Update send window
  UpdateSendWindow ();

  // Process NACKs
  for (uint32_t nackSeq : ack.nackList)
    {
      auto it = m_sendBuffer.find (nackSeq);
      if (it != m_sendBuffer.end ())
        {
          RetransmitPacket (nackSeq);
        }
    }

  LogDetailed ("ProcessReceivedAcknowledgment", "ACK processed, new send window base: " +
               std::to_string (m_sendWindowBase));

  return true;
}

uint32_t
Tpdc::ForceRetransmission (void)
{
  NS_LOG_FUNCTION (this);

  uint32_t retransmitted = 0;

  for (auto& pair : m_sendBuffer)
    {
      if (!pair.second.acknowledged &&
          pair.second.retransmissionCount < m_tpdcConfig.maxRetransmissions)
        {
          RetransmitPacket (pair.first);
          retransmitted++;
        }
    }

  LogDetailed ("ForceRetransmission", "Forced retransmission of " +
               std::to_string (retransmitted) + " packets");

  return retransmitted;
}

uint32_t
Tpdc::ClearSendBuffer (void)
{
  uint32_t count = m_sendBuffer.size ();
  m_sendBuffer.clear ();
  return count;
}

uint32_t
Tpdc::ClearReceiveBuffer (void)
{
  uint32_t count = m_receiveBuffer.size ();
  m_receiveBuffer.clear ();
  return count;
}

Time
Tpdc::GetCurrentRtt (void) const
{
  return m_currentRtt;
}

Time
Tpdc::GetCurrentRto (void) const
{
  return m_currentRto;
}

bool
Tpdc::ValidatePacket (Ptr<Packet> packet, bool isSend) const
{
  if (!PdcBase::ValidatePacket (packet, isSend))
    {
      return false;
    }

  uint32_t packetSize = packet->GetSize ();

  // Additional TPDC-specific validation
  if (packetSize > m_tpdcConfig.maxPacketSize)
    {
      LogDetailed ("ValidatePacket", "Packet size exceeds TPDC limit: " +
                   std::to_string (packetSize));
      return false;
    }

  return true;
}

bool
Tpdc::HandleError (PdsErrorCode error, const std::string& details)
{
  NS_LOG_FUNCTION (this << "TPDC Error: " << static_cast<int> (error) << " - " << details);

  // Update TPDC-specific error statistics
  m_tpdcStatistics.errors++;

  // Handle specific TPDC errors
  switch (error)
    {
    case PdsErrorCode::PDC_FULL:
      if (details.find ("send") != std::string::npos)
        {
          m_tpdcStatistics.sendBufferOverflows++;
        }
      else
        {
          m_tpdcStatistics.receiveBufferOverflows++;
        }
      NS_LOG_WARN ("TPDC buffer full, consider increasing buffer sizes");
      break;
    case PdsErrorCode::RESOURCE_EXHAUSTED:
      NS_LOG_WARN ("TPDC resources exhausted, system may need scaling");
      break;
    default:
      break;
    }

  // Let base class handle common error processing
  return PdcBase::HandleError (error, details);
}

bool
Tpdc::BufferPacketForSending (Ptr<Packet> packet, bool som, bool eom)
{
  if (m_sendBuffer.size () >= m_tpdcConfig.maxSendBufferSize)
    {
      m_tpdcStatistics.sendBufferOverflows++;
      HandleError (PdsErrorCode::PDC_FULL, "Send buffer overflow");
      return false;
    }

  // Check if sequence number is in send window
  uint32_t seq = GenerateSequenceNumber ();
  if (!IsInSendWindow (seq))
    {
      // Queue packet for later sending
      m_sendQueue.push (packet);
      return true;
    }

  // Create buffered packet
  BufferedPacket bp (packet, som, eom, seq);

  // Create RTO timer
  bp.rtoTimer = CreateObject<RtoTimer> ();
  bp.rtoTimer->SetPdcId (m_tpdcConfig.pdcId);
  bp.rtoTimer->SetSequenceNumber (seq);
  bp.rtoTimer->SetTimeout (m_currentRto);

  // Add to send buffer
  m_sendBuffer[seq] = bp;

  // Start RTO timer
  bp.rtoTimer->Start ();

  LogDetailed ("BufferPacketForSending", "Packet buffered, seq: " +
               std::to_string (seq) + ", buffer size: " + std::to_string (m_sendBuffer.size ()));

  return true;
}

bool
Tpdc::BufferPacketForReceiving (Ptr<Packet> packet, uint32_t seq)
{
  if (m_receiveBuffer.size () >= m_tpdcConfig.maxReceiveBufferSize)
    {
      m_tpdcStatistics.receiveBufferOverflows++;
      HandleError (PdsErrorCode::PDC_FULL, "Receive buffer overflow");
      return false;
    }

  // Check if sequence number is in receive window
  if (!IsInReceiveWindow (seq))
    {
      m_tpdcStatistics.outOfOrderPackets++;
      return false;
    }

  // Create buffered packet
  BufferedPacket bp (packet, false, false, seq);

  // Add to receive buffer
  m_receiveBuffer[seq] = bp;

  // Update next expected sequence
  if (seq == m_nextReceiveSequence)
    {
      m_nextReceiveSequence++;
    }

  // Schedule ACK
  ScheduleAcknowledgment ();

  LogDetailed ("BufferPacketForReceiving", "Packet buffered, seq: " +
               std::to_string (seq) + ", buffer size: " + std::to_string (m_receiveBuffer.size ()));

  return true;
}

void
Tpdc::ProcessSendQueue (void)
{
  NS_LOG_FUNCTION (this);

  while (!m_sendQueue.empty () && IsInSendWindow (m_nextSendSequence))
    {
      Ptr<Packet> packet = m_sendQueue.front ();
      m_sendQueue.pop ();

      // Create buffered packet for sending
      BufferedPacket bp (packet, false, false, m_nextSendSequence);

      // Create RTO timer
      bp.rtoTimer = CreateObject<RtoTimer> ();
      bp.rtoTimer->SetPdcId (m_tpdcConfig.pdcId);
      bp.rtoTimer->SetSequenceNumber (m_nextSendSequence);
      bp.rtoTimer->SetTimeout (m_currentRto);

      // Add to send buffer and start timer
      m_sendBuffer[m_nextSendSequence] = bp;
      bp.rtoTimer->Start ();

      m_nextSendSequence++;
    }
}

void
Tpdc::ProcessReceiveQueue (void)
{
  NS_LOG_FUNCTION (this);

  // Process in-order packets
  while (m_receiveBuffer.find (m_nextReceiveSequence) != m_receiveBuffer.end ())
    {
      BufferedPacket& bp = m_receiveBuffer[m_nextReceiveSequence];

      // Process packet
      PdcBase::UpdateStatistics (false, bp.packet);
      m_packetRxTrace (bp.packet, m_tpdcConfig.pdcId);

      // Remove from buffer
      m_receiveBuffer.erase (m_nextReceiveSequence);
      m_nextReceiveSequence++;
    }

  // Update receive window
  UpdateReceiveWindow ();
}

void
Tpdc::RetransmitPacket (uint32_t sequenceNumber)
{
  NS_LOG_FUNCTION (this << "Retransmitting packet " << sequenceNumber);

  auto it = m_sendBuffer.find (sequenceNumber);
  if (it == m_sendBuffer.end () || it->second.acknowledged)
    {
      return;
    }

  BufferedPacket& bp = it->second;

  // Check retransmission limit
  if (bp.retransmissionCount >= m_tpdcConfig.maxRetransmissions)
    {
      NS_LOG_WARN ("Maximum retransmissions reached for packet " << sequenceNumber);
      HandleError (PdsErrorCode::PROTOCOL_ERROR, "Maximum retransmissions exceeded");
      return;
    }

  // Update retransmission info
  bp.retransmissionCount++;
  bp.lastRetransmission = Simulator::Now ();

  // Update RTO with exponential backoff
  Time newRto = m_currentRto * m_tpdcConfig.rtoBackoffMultiplier;
  if (newRto > m_tpdcConfig.maxRto)
    {
      newRto = m_tpdcConfig.maxRto;
    }
  m_currentRto = newRto;

  // Restart RTO timer
  if (bp.rtoTimer)
    {
      bp.rtoTimer->SetTimeout (m_currentRto);
      bp.rtoTimer->Restart ();
    }

  // Update statistics
  m_tpdcStatistics.retransmissions++;
  if (bp.retransmissionCount == 1)
    {
      m_tpdcStatistics.fastRetransmissions++;
    }

  // Trace retransmission
  m_retransmissionTrace (sequenceNumber, bp.retransmissionCount);

  // Actually retransmit packet
  TransmitPacket (bp);

  LogDetailed ("RetransmitPacket", "Retransmitted packet " + std::to_string (sequenceNumber) +
               ", attempt " + std::to_string (bp.retransmissionCount));
}

bool
Tpdc::TransmitPacket (const BufferedPacket& bp)
{
  NS_LOG_FUNCTION (this << "Transmitting packet, seq: " << bp.sequenceNumber);

  // Create PDS header (suppress unused warning with (void))
  PDSHeader header = CreatePdsHeader (bp.packet, bp.som, bp.eom);
  (void)header; // Suppress unused variable warning
  NS_LOG_DEBUG ("Created PDS header for packet transmission");

  // In a real implementation, this would send through the network device
  // For simulation, we assume success
  if (GetNetDevice ())
    {
      PdcBase::UpdateStatistics (true, bp.packet);
      m_packetTxTrace (bp.packet, m_tpdcConfig.pdcId);
      return true;
    }

  return false;
}

void
Tpdc::ScheduleAcknowledgment (void)
{
  if (!m_ackEventId.IsPending ())
    {
      m_ackEventId = Simulator::Schedule (m_ackInterval,
                                         &Tpdc::SendPendingAcknowledgments, this);
    }
}

void
Tpdc::SendPendingAcknowledgments (void)
{
  if (m_receiveBuffer.empty ())
    {
      return;
    }

  // Create acknowledgment
  Acknowledgment ack = CreateAcknowledgment ();

  // Send acknowledgment
  SendAcknowledgment (ack);

  // Clear ACK queue
  while (!m_ackQueue.empty ())
    {
      m_ackQueue.pop ();
    }
}

void
Tpdc::UpdateSendWindow (void)
{
  // Send window is controlled by available buffer space and received ACKs
  // Simple implementation: window size = config - buffer size
}

void
Tpdc::UpdateReceiveWindow (void)
{
  // Receive window is controlled by available buffer space
  // Simple implementation: window size = config - buffer size
}

void
Tpdc::UpdateRtoEstimate (Time measuredRtt)
{
  NS_LOG_FUNCTION (this << "Measured RTT: " << measuredRtt.GetMilliSeconds () << "ms");

  if (m_currentRtt.IsZero ())
    {
      // First measurement
      m_currentRtt = measuredRtt;
      m_rttVariance = measuredRtt / 2;
    }
  else
    {
      // Use Jacobson/Karels algorithm
      Time error = measuredRtt - m_currentRtt;
      m_rttVariance = Time (std::abs (error.GetDouble () * 0.25 + m_rttVariance.GetDouble () * 0.75));
      m_currentRtt = Time (m_currentRtt.GetDouble () + error.GetDouble () * 0.125);
    }

  // Calculate RTO
  Time varianceComponent = std::max (Seconds (1.0), Time (m_rttVariance.GetDouble () * 4));
  m_currentRto = m_currentRtt + varianceComponent;

  // Clamp RTO to configured bounds
  if (m_currentRto < m_tpdcConfig.initialRto)
    {
      m_currentRto = m_tpdcConfig.initialRto;
    }
  if (m_currentRto > m_tpdcConfig.maxRto)
    {
      m_currentRto = m_tpdcConfig.maxRto;
    }
}

void
Tpdc::HandleRtoTimeout (uint32_t sequenceNumber)
{
  NS_LOG_FUNCTION (this << "RTO timeout for packet " << sequenceNumber);

  m_tpdcStatistics.retransmissionTimeouts++;

  // Retransmit the timed-out packet
  RetransmitPacket (sequenceNumber);
}

bool
Tpdc::IsInSendWindow (uint32_t sequence) const
{
  uint32_t windowSize = m_tpdcConfig.sendWindowSize;
  return (sequence >= m_sendWindowBase) && (sequence < m_sendWindowBase + windowSize);
}

bool
Tpdc::IsInReceiveWindow (uint32_t sequence) const
{
  uint32_t windowSize = m_tpdcConfig.receiveWindowSize;
  return (sequence >= m_receiveWindowBase) && (sequence < m_receiveWindowBase + windowSize);
}

void
Tpdc::CleanupAcknowledgedPackets (void)
{
  auto it = m_sendBuffer.begin ();
  while (it != m_sendBuffer.end ())
    {
      if (it->second.acknowledged)
        {
          // Cancel timer
          if (it->second.rtoTimer)
            {
              it->second.rtoTimer->Cancel ();
            }
          it = m_sendBuffer.erase (it);
        }
      else
        {
          it++;
        }
    }
}

Acknowledgment
Tpdc::CreateAcknowledgment (void)
{
  // Find highest in-order sequence
  uint32_t highestInOrder = m_receiveWindowBase;
  while (m_receiveBuffer.find (highestInOrder) != m_receiveBuffer.end ())
    {
      highestInOrder++;
    }

  // Create ACK
  Acknowledgment ack (highestInOrder - 1, m_tpdcConfig.receiveWindowSize,
                     m_tpdcConfig.enableCumulativeAcks);

  // Add NACKs for missing packets
  for (uint32_t seq = m_receiveWindowBase; seq < highestInOrder; seq++)
    {
      if (m_receiveBuffer.find (seq) == m_receiveBuffer.end ())
        {
          ack.nackList.push_back (seq);
        }
    }

  return ack;
}

uint32_t
Tpdc::GenerateSequenceNumber (void)
{
  return m_nextSendSequence++;
}

void
Tpdc::UpdateStatistics (void)
{
  m_tpdcStatistics.currentSendBufferSize = m_sendBuffer.size ();
  m_tpdcStatistics.currentReceiveBufferSize = m_receiveBuffer.size ();
}

} // namespace ns3

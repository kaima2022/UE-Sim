#include "pdc-base.h"
#include "../ses/ses-manager.h"
#include "../network/soft-ue-net-device.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/nstime.h"
#include "ns3/boolean.h"
#include "ns3/integer.h"
#include "ns3/uinteger.h"
#include <algorithm>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("PdcBase");
NS_OBJECT_ENSURE_REGISTERED (PdcBase);

TypeId
PdcBase::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::PdcBase")
    .SetParent<Object> ()
    .SetGroupName ("Soft-Ue")
    .AddAttribute ("DetailedLogging",
                   "Enable detailed logging for PDC operations",
                   BooleanValue (false),
                   MakeBooleanAccessor (&PdcBase::SetDetailedLogging),
                   MakeBooleanChecker ())
    .AddAttribute ("ProcessInterval",
                   "Interval between processing cycles",
                   TimeValue (MilliSeconds (1)),
                   MakeTimeAccessor (&PdcBase::m_processInterval),
                   MakeTimeChecker ())
    .AddAttribute ("PdcId",
                   "PDC identifier",
                   UintegerValue (0),
                   MakeUintegerAccessor (&PdcBase::GetPdcId, &PdcBase::SetPdcId),
                   MakeUintegerChecker<uint16_t> ())
    .AddAttribute ("LocalFep",
                   "Local fabric endpoint",
                   UintegerValue (0),
                   MakeUintegerAccessor (&PdcBase::GetLocalFep, &PdcBase::SetLocalFep),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("RemoteFep",
                   "Remote fabric endpoint",
                   UintegerValue (0),
                   MakeUintegerAccessor (&PdcBase::GetRemoteFep, &PdcBase::SetRemoteFep),
                   MakeUintegerChecker<uint32_t> ())
    .AddTraceSource ("PacketTx",
                     "Trace packet transmission through PDC",
                     MakeTraceSourceAccessor (&PdcBase::m_packetTxTrace),
                     "ns3::Packet::TracedCallback")
    .AddTraceSource ("PacketRx",
                     "Trace packet reception at PDC",
                     MakeTraceSourceAccessor (&PdcBase::m_packetRxTrace),
                     "ns3::Packet::TracedCallback")
    .AddTraceSource ("Error",
                     "Trace PDC error events",
                     MakeTraceSourceAccessor (&PdcBase::m_errorTrace),
                     "ns3::PdcBase::ErrorTracedCallback")
    .AddTraceSource ("Latency",
                     "Trace packet processing latency",
                     MakeTraceSourceAccessor (&PdcBase::m_latencyTrace),
                     "ns3::PdcBase::LatencyTracedCallback")
    ;
  return tid;
}

PdcBase::PdcBase ()
  : m_active (false),
    m_processInterval (MilliSeconds (1))
{
  NS_LOG_FUNCTION (this);
}

PdcBase::~PdcBase ()
{
  NS_LOG_FUNCTION (this);
}

void
PdcBase::DoDispose (void)
{
  NS_LOG_FUNCTION (this);

  // Cancel scheduled processing
  if (m_processEventId.IsPending ())
    {
      Simulator::Cancel (m_processEventId);
    }

  // Clear queues
  while (!m_sendQueue.empty ())
    {
      m_sendQueue.pop ();
    }
  while (!m_receiveQueue.empty ())
    {
      m_receiveQueue.pop ();
    }

  // Clear component references
  m_netDevice = nullptr;
  m_sesManager = nullptr;

  // Clear packet timestamps
  m_packetTimestamps.clear ();

  Object::DoDispose ();
}

bool
PdcBase::Initialize (const PdcConfig& config)
{
  NS_LOG_FUNCTION (this << "Initializing PDC " << config.pdcId);

  m_config = config;
  m_statistics = PdcStatistics ();
  m_active = false;

  // Validate configuration
  if (config.pdcId == 0 || config.localFep == 0 || config.remoteFep == 0)
    {
      NS_LOG_ERROR ("Invalid PDC configuration - missing required parameters");
      return false;
    }

  // Start periodic processing
  ScheduleProcessing ();

  NS_LOG_INFO ("PDC " << config.pdcId << " initialized successfully");
  return true;
}

uint16_t
PdcBase::GetPdcId (void) const
{
  return m_config.pdcId;
}

void
PdcBase::SetPdcId (uint16_t pdcId)
{
  NS_LOG_FUNCTION (this << pdcId);
  m_config.pdcId = pdcId;
}

PdcType
PdcBase::GetPdcType (void) const
{
  return m_config.type;
}

uint32_t
PdcBase::GetLocalFep (void) const
{
  return m_config.localFep;
}

uint32_t
PdcBase::GetRemoteFep (void) const
{
  return m_config.remoteFep;
}

void
PdcBase::SetLocalFep (uint32_t localFep)
{
  m_config.localFep = localFep;
}

void
PdcBase::SetRemoteFep (uint32_t remoteFep)
{
  m_config.remoteFep = remoteFep;
}

void
PdcBase::SetNetDevice (Ptr<SoftUeNetDevice> device)
{
  NS_LOG_FUNCTION (this << device);
  m_netDevice = device;
}

Ptr<SoftUeNetDevice>
PdcBase::GetNetDevice (void) const
{
  return m_netDevice;
}

void
PdcBase::SetSesManager (Ptr<SesManager> sesManager)
{
  NS_LOG_FUNCTION (this << sesManager);
  m_sesManager = sesManager;
}

Ptr<SesManager>
PdcBase::GetSesManager (void) const
{
  return m_sesManager;
}

bool
PdcBase::HandleReceivedPacket (Ptr<Packet> packet, uint32_t sourceFep)
{
  NS_LOG_FUNCTION (this << "Handling received packet from FEP " << sourceFep);

  if (!packet || !m_active)
    {
      HandleError (PdsErrorCode::INVALID_PACKET, "Null packet or inactive PDC");
      return false;
    }

  // Validate packet format
  if (!ValidatePacket (packet, false))
    {
      HandleError (PdsErrorCode::INVALID_PACKET, "Packet validation failed");
      return false;
    }

  // Parse PDS header
  UETPDSHeader header = ParsePdsHeader (packet);
  if (header.dest_fep != m_config.localFep)
    {
      HandleError (PdsErrorCode::PROTOCOL_ERROR, "Packet destination mismatch");
      return false;
    }

  // Record packet entry timestamp for latency measurement
  RecordPacketEntry (packet);

  // Add to receive queue for processing
  m_receiveQueue.push (packet);

  LogDetailed ("HandleReceivedPacket", "Packet queued for processing, queue size: " +
               std::to_string (m_receiveQueue.size ()));

  return true;
}

PdcStatistics
PdcBase::GetStatistics (void) const
{
  return m_statistics;
}

void
PdcBase::ResetStatistics (void)
{
  NS_LOG_FUNCTION (this);
  m_statistics = PdcStatistics ();
}

void
PdcBase::SetDetailedLogging (bool enable)
{
  NS_LOG_FUNCTION (this << enable);
  m_config.detailedLogging = enable;
}

bool
PdcBase::IsActive (void) const
{
  return m_active;
}

void
PdcBase::Activate (void)
{
  NS_LOG_FUNCTION (this);
  if (!m_active)
    {
      m_active = true;
      NS_LOG_INFO ("PDC " << m_config.pdcId << " activated");
    }
}

void
PdcBase::Deactivate (void)
{
  NS_LOG_FUNCTION (this);
  if (m_active)
    {
      m_active = false;

      // Cancel processing
      if (m_processEventId.IsPending ())
        {
          Simulator::Cancel (m_processEventId);
        }

      NS_LOG_INFO ("PDC " << m_config.pdcId << " deactivated");
    }
}

PdcConfig
PdcBase::GetConfiguration (void) const
{
  return m_config;
}

bool
PdcBase::UpdateConfiguration (const PdcConfig& config)
{
  NS_LOG_FUNCTION (this << "Updating PDC configuration");

  // Validate new configuration
  if (config.pdcId != m_config.pdcId ||
      config.type != m_config.type)
    {
      NS_LOG_ERROR ("Cannot change PDC ID or type after initialization");
      return false;
    }

  m_config = config;
  return true;
}

PDSHeader
PdcBase::CreatePdsHeader (Ptr<Packet> packet, bool som, bool eom) const
{
  PDSHeader header;
  header.SetPdcId (GetPdcId ());
  header.SetSequenceNumber (m_config.sequenceNumber);
  header.SetSom (som);
  header.SetEom (eom);

  return header;
}

UETPDSHeader
PdcBase::ParsePdsHeader (Ptr<Packet> packet) const
{
  UETPDSHeader header;

  // For simulation purposes, assume header is properly formatted
  // In a real implementation, this would deserialize the actual header bytes
  header.version = 1;
  header.tc = m_config.tc;
  header.dest_fep = m_config.localFep;
  header.src_fep = m_config.remoteFep;
  header.som = false;
  header.eom = false;
  header.next_hdr = PDSNextHeader::PAYLOAD;
  header.msg_len = packet->GetSize ();

  return header;
}

void
PdcBase::UpdateStatistics (bool isSend, Ptr<Packet> packet)
{
  NS_LOG_FUNCTION (this << (isSend ? "send" : "receive"));

  Time now = Simulator::Now ();

  if (isSend)
    {
      m_statistics.packetsSent++;
      m_statistics.bytesTransmitted += packet->GetSize ();
    }
  else
    {
      m_statistics.packetsReceived++;
      m_statistics.bytesReceived += packet->GetSize ();
    }

  m_statistics.lastActivity = now;

  // Update throughput periodically
  if ((now - m_statistics.startTime).GetSeconds () > 1.0) // Update every second
    {
      m_statistics.UpdateThroughput ();
    }
}

void
PdcBase::LogDetailed (const std::string& function, const std::string& message) const
{
  if (m_config.detailedLogging)
    {
      NS_LOG_DEBUG ("PdcBase::" << function << ": " << message);
    }
}

bool
PdcBase::ValidatePacket (Ptr<Packet> packet, bool isSend) const
{
  if (!packet)
    {
      return false;
    }

  uint32_t packetSize = packet->GetSize ();
  if (packetSize == 0 || packetSize > m_config.maxPacketSize)
    {
      LogDetailed ("ValidatePacket", "Invalid packet size: " + std::to_string (packetSize));
      return false;
    }

  return true;
}

bool
PdcBase::HandleError (PdsErrorCode error, const std::string& details)
{
  NS_LOG_FUNCTION (this << "Error: " << static_cast<int> (error) << " - " << details);

  // Update error statistics
  m_statistics.errors++;
  m_statistics.lastErrorDetails = details;

  // Categorize errors for detailed tracking
  switch (error)
    {
    case PdsErrorCode::INVALID_PACKET:
      m_statistics.validationErrors++;
      break;
    case PdsErrorCode::PROTOCOL_ERROR:
      m_statistics.protocolErrors++;
      break;
    case PdsErrorCode::PDC_FULL:
      m_statistics.bufferErrors++;
      m_statistics.packetsDropped++;
      break;
    default:
      m_statistics.networkErrors++;
      break;
    }

  // Trace error with full details
  std::string errorType = GetErrorTypeString (error);
  std::string fullDetails = errorType + ": " + details;
  m_errorTrace (m_config.pdcId, fullDetails);

  LogDetailed ("HandleError", "PDC error [" + errorType + "]: " + details);

  // Attempt error recovery for non-critical errors
  if (error == PdsErrorCode::PDC_FULL)
    {
      // Trigger queue cleanup
      if (m_receiveQueue.size () > m_config.maxPacketSize)
        {
          NS_LOG_INFO ("Triggering emergency queue cleanup due to buffer overflow");
          // Could implement aggressive packet dropping here
        }
    }

  return true;
}

void
PdcBase::ScheduleProcessing (void)
{
  if (!m_processEventId.IsPending ())
    {
      m_processEventId = Simulator::Schedule (m_processInterval,
                                           &PdcBase::DoPeriodicProcessing, this);
    }
}

void
PdcBase::DoPeriodicProcessing (void)
{
  if (!m_active)
    {
      return;
    }

  // Process queues
  ProcessSendQueue ();
  ProcessReceiveQueue ();

  // Schedule next processing cycle
  ScheduleProcessing ();
}

void
PdcBase::ProcessSendQueue (void)
{
  // This will be implemented by subclasses
  // Base class just clears the queue
  while (!m_sendQueue.empty ())
    {
      Ptr<Packet> packet = m_sendQueue.front ();
      m_sendQueue.pop ();

      LogDetailed ("ProcessSendQueue", "Discarding unimplemented packet send");
    }
}

void
PdcBase::ProcessReceiveQueue (void)
{
  while (!m_receiveQueue.empty ())
    {
      Ptr<Packet> packet = m_receiveQueue.front ();
      m_receiveQueue.pop ();

      UpdateStatistics (false, packet);
      m_packetRxTrace (packet, m_config.pdcId);

      // Measure and trace packet processing latency
      MeasureAndTraceLatency (packet);

      LogDetailed ("ProcessReceiveQueue", "Processed received packet, size: " +
                   std::to_string (packet->GetSize ()));
    }
}

// Latency tracking implementation
uint64_t
PdcBase::GetPacketId (Ptr<Packet> packet)
{
  NS_LOG_FUNCTION (this << packet);

  // Use packet UID as unique identifier
  return packet->GetUid ();
}

void
PdcBase::RecordPacketEntry (Ptr<Packet> packet)
{
  NS_LOG_FUNCTION (this << packet);

  uint64_t packetId = GetPacketId (packet);
  Time entryTime = Simulator::Now ();

  // Store entry timestamp
  m_packetTimestamps[packetId] = entryTime;

  NS_LOG_DEBUG ("Recorded entry time for packet " << packetId << " at " << entryTime.GetMicroSeconds () << "μs");
}

void
PdcBase::MeasureAndTraceLatency (Ptr<Packet> packet)
{
  NS_LOG_FUNCTION (this << packet);

  uint64_t packetId = GetPacketId (packet);
  auto it = m_packetTimestamps.find (packetId);

  if (it != m_packetTimestamps.end ())
    {
      Time entryTime = it->second;
      Time exitTime = Simulator::Now ();
      Time latency = exitTime - entryTime;

      // Update statistics
      if (m_statistics.packetsReceived > 0)
        {
          double latencyMicros = latency.GetMicroSeconds ();
          m_statistics.averageLatency =
            ((m_statistics.averageLatency * (m_statistics.packetsReceived - 1)) + latencyMicros) /
            m_statistics.packetsReceived;

          // Update min/max latency
          if (m_statistics.minLatency == 0.0 || latencyMicros < m_statistics.minLatency)
            {
              m_statistics.minLatency = latencyMicros;
            }
          if (latencyMicros > m_statistics.maxLatency)
            {
              m_statistics.maxLatency = latencyMicros;
            }
        }

      // Trace latency
      m_latencyTrace (latency, m_config.pdcId);

      // Clean up timestamp entry
      m_packetTimestamps.erase (it);

      NS_LOG_DEBUG ("Measured latency for packet " << packetId << ": " << latency.GetMicroSeconds () << "μs");
    }
  else
    {
      NS_LOG_WARN ("No entry timestamp found for packet " << packetId);
    }
}

std::string
PdcBase::GetErrorTypeString (PdsErrorCode error)
{
  switch (error)
    {
    case PdsErrorCode::SUCCESS:
      return "SUCCESS";
    case PdsErrorCode::INVALID_PDC:
      return "INVALID_PDC";
    case PdsErrorCode::PDC_FULL:
      return "BUFFER_OVERFLOW";
    case PdsErrorCode::INVALID_PACKET:
      return "VALIDATION_ERROR";
    case PdsErrorCode::RESOURCE_EXHAUSTED:
      return "RESOURCE_EXHAUSTED";
    case PdsErrorCode::PROTOCOL_ERROR:
      return "PROTOCOL_ERROR";
    default:
      return "UNKNOWN_ERROR";
    }
}

} // namespace ns3

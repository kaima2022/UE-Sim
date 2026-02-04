#include "ses-manager.h"
#include "ns3/log.h"
#include "ns3/assert.h"
#include <sstream>
#include "../pds/pds-manager.h"
#include "../network/soft-ue-net-device.h"
#include "../common/soft-ue-packet-tag.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("SesManager");

NS_OBJECT_ENSURE_REGISTERED (SesManager);

TypeId
SesManager::GetTypeId (void)
{
  NS_LOG_FUNCTION_NOARGS ();

  static TypeId tid = TypeId ("ns3::SesManager")
    .SetParent<Object> ()
    .SetGroupName ("SoftUe")
    .AddConstructor<SesManager> ();

  return tid;
}

TypeId
SesManager::GetInstanceTypeId (void) const
{
  NS_LOG_FUNCTION (this);
  return GetTypeId ();
}

SesManager::SesManager ()
  : m_maxMessageId (65535),
    m_currentMessageId (1),
    m_maxMtu (1500),
    m_detailedLogging (false),
    m_state (SES_IDLE),
    m_totalSendRequests (0),
    m_totalReceiveRequests (0),
    m_totalResponses (0),
    m_totalSuccessfulRequests (0),
    m_totalErrors (0),
    m_totalPacketsGenerated (0),
    m_totalPacketsConsumed (0)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_DEBUG ("SesManager created in IDLE state");
}

SesManager::~SesManager ()
{
  NS_LOG_FUNCTION (this);
  NS_LOG_DEBUG ("SesManager destroyed");
}

void
SesManager::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_DEBUG ("Disposing SES Manager");

  // Clean up member variables
  // TODO: Add specific cleanup logic as needed

  // Call parent class DoDispose
  Object::DoDispose ();

  NS_LOG_DEBUG ("SES Manager disposal completed");
}

void
SesManager::Initialize (void)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_DEBUG ("Initializing SES Manager");

  // Initialize MSN table for message tracking
  m_msnTable = Create<MsnTable> ();
  if (!m_msnTable)
  {
    NS_LOG_ERROR ("Failed to create MSN table");
    return;
  }
  NS_LOG_DEBUG ("MSN table created successfully");

  // Initialize internal components and state
  NS_LOG_DEBUG ("SES Manager initialization completed");
}

void
SesManager::SetPdsManager (Ptr<PdsManager> pdsManager)
{
  NS_LOG_FUNCTION (this << pdsManager);
  m_pdsManager = pdsManager;
  NS_LOG_DEBUG ("PDS Manager set");
}

Ptr<PdsManager>
SesManager::GetPdsManager (void) const
{
    NS_LOG_FUNCTION (this);
    NS_LOG_DEBUG ("Getting PDS Manager");
    return m_pdsManager;
}

void
SesManager::SetNetDevice (Ptr<SoftUeNetDevice> device)
{
    NS_LOG_FUNCTION (this << device);
    m_netDevice = device;
    NS_LOG_DEBUG ("Network device set");
}

Ptr<SoftUeNetDevice>
SesManager::GetNetDevice (void) const
{
    NS_LOG_FUNCTION (this);
    NS_LOG_DEBUG ("Getting network device");
    return m_netDevice;
}

bool
SesManager::ProcessSendRequest (Ptr<ExtendedOperationMetadata> metadata)
{
    return ProcessSendRequest (metadata, nullptr);
}

bool
SesManager::ProcessSendRequest (Ptr<ExtendedOperationMetadata> metadata, Ptr<Packet> packet)
{
    NS_LOG_FUNCTION (this << metadata << packet);

    if (!metadata)
    {
        NS_LOG_WARN ("ProcessSendRequest: metadata is null");
        return false;
    }

    // Check state machine - reject if busy or in error state
    if (m_state == SES_BUSY)
    {
        NS_LOG_WARN ("ProcessSendRequest rejected: SES manager is busy");
        return false;
    }

    if (m_state == SES_ERROR)
    {
        NS_LOG_ERROR ("ProcessSendRequest rejected: SES manager is in error state");
        return false;
    }

    NS_LOG_DEBUG ("Processing send request for operation");
    m_totalSendRequests++;

    // Validate operation metadata
    if (!ValidateOperationMetadata (metadata))
    {
        NS_LOG_ERROR ("Invalid operation metadata");
        m_totalErrors++;
        return false;
    }

    if (!m_pdsManager)
    {
        NS_LOG_ERROR ("PDS Manager not available");
        m_totalErrors++;
        return false;
    }

    // When packet is provided, SES breaks down the transaction into one or multiple packets and submits to PDS
    if (packet)
    {
        metadata->payload.length = packet->GetSize ();
        uint32_t payloadLen = static_cast<uint32_t> (metadata->payload.length);
        uint32_t nPackets = metadata->CalculatePacketCount (m_maxMtu);
        bool fragment = metadata->RequiresFragmentation (m_maxMtu);

        if (fragment && nPackets > 1)
        {
            // Transaction → multiple packets: split by MTU and submit each via PDS
            NS_LOG_INFO ("============================================================");
            NS_LOG_INFO (" [UEC-E2E] 大包开始（切包） 事务 " << payloadLen << " bytes → " << nPackets
                         << " 片 (MTU=" << m_maxMtu << ")");
            NS_LOG_INFO ("============================================================");
            SesPdsRequest baseRequest = InitializeSesHeader (metadata);
            uint32_t messageId = baseRequest.rod_context;
            uint32_t payloadPerPacket = (payloadLen + nPackets - 1) / nPackets;
            for (uint32_t i = 0; i < nPackets; ++i)
            {
                NS_LOG_INFO ("============================================================");
                NS_LOG_INFO (" [UEC-E2E] 小包 " << (i + 1) << "/" << nPackets << " 全流程（发送）");
                NS_LOG_INFO ("============================================================");
                uint32_t offset = i * payloadPerPacket;
                uint32_t fragLen = (i + 1 == nPackets)
                    ? (payloadLen - offset)
                    : payloadPerPacket;
                if (fragLen == 0)
                    continue;
                Ptr<Packet> frag = packet->CreateFragment (offset, fragLen);
                frag->AddPacketTag (SoftUeFragmentTag (i + 1, nPackets));
                SesPdsRequest request = baseRequest;
                request.packet = frag;
                request.pkt_len = static_cast<uint16_t> (fragLen);
                request.som = (i == 0);
                request.eom = (i == nPackets - 1);
                request.rod_context = messageId;
                bool ok = m_pdsManager->DispatchPacket (request);
                if (!ok)
                {
                    m_totalErrors++;
                    m_state = SES_IDLE;
                    return false;
                }
                m_totalPacketsGenerated++;
                NS_LOG_INFO ("[UEC-E2E] [SES] 切包 第 " << (i + 1) << "/" << nPackets << " 片: len="
                             << fragLen << " SOM=" << request.som << " EOM=" << request.eom);
            }
            NS_LOG_INFO ("[UEC-E2E] [SES] ③ SES 层 ProcessSendRequest: 1 事务 → " << nPackets << " 包");
        }
        else
        {
            // Single packet: one SesPdsRequest and submit
            SesPdsRequest request = InitializeSesHeader (metadata);
            request.packet = packet;
            request.pkt_len = static_cast<uint16_t> (payloadLen);
            request.som = true;
            request.eom = true;
            request.rod_context = GenerateMessageId (metadata);
            bool ok = m_pdsManager->DispatchPacket (request);
            if (!ok)
            {
                m_totalErrors++;
                m_state = SES_IDLE;
                return false;
            }
            m_totalPacketsGenerated++;
            NS_LOG_INFO ("[UEC-E2E] [SES] ③ SES 层 ProcessSendRequest: src_node=" << metadata->GetSourceNodeId ()
                         << " dst_node=" << metadata->GetDestinationNodeId () << " job_id=" << metadata->job_id
                         << " messages_id=" << metadata->messages_id << " → 校验通过，单包发送");
        }
    }
    else
    {
        NS_LOG_INFO ("[UEC-E2E] [SES] ③ SES 层 ProcessSendRequest: src_node=" << metadata->GetSourceNodeId ()
                     << " dst_node=" << metadata->GetDestinationNodeId () << " job_id=" << metadata->job_id
                     << " messages_id=" << metadata->messages_id << " → 校验通过，允许发送");
    }

    m_totalSuccessfulRequests++;
    m_state = SES_IDLE;
    return true;
}

bool
SesManager::ProcessReceiveRequest (const PdcSesRequest& request)
{
    NS_LOG_FUNCTION (this);
    NS_LOG_DEBUG ("Processing receive request");
    m_totalReceiveRequests++;

    // Validate input parameters
    if (!ValidatePdcSesRequest (request))
    {
        NS_LOG_ERROR ("Invalid PDC SES request received");
        m_totalErrors++;
        return false;
    }

    try
    {
        // Optional: parse into metadata for queue/processing (B1: receive path via SES)
        Ptr<ExtendedOperationMetadata> metadata = ParseReceivedRequest (request);
        if (metadata)
        {
            m_recvRequestQueue.push (request);
            if (m_state == SES_IDLE)
                ScheduleProcessing ();
        }

        // B1: Drive delivery to app via device (receive path PDS → SES → App)
        if (m_netDevice && request.packet)
        {
            NS_LOG_INFO ("[UEC-E2E] [Control] Rx req/Rx rsp (placeholder) pdc_id=" << request.pdc_id);
            NS_LOG_INFO ("[UEC-E2E] [SES] ProcessReceiveRequest pdc_id=" << request.pdc_id
                         << " → DeliverReceivedPacket（收端经 SES → App）");
            m_netDevice->DeliverReceivedPacket (request.packet);
        }
        m_totalSuccessfulRequests++;
        NS_LOG_DEBUG ("Receive request processed, delivery driven via SES");
        return true;
    }
    catch (const std::exception& e)
    {
        NS_LOG_ERROR ("Exception in ProcessReceiveRequest: " << e.what ());
        m_totalErrors++;
        return false;
    }
}

void
SesManager::NotifyTxResponse (uint16_t pdcId)
{
  NS_LOG_INFO ("[UEC-E2E] [Control] Tx rsp (placeholder) pdc_id=" << pdcId);
}

void
SesManager::NotifyPdsErrorEvent (uint16_t pdcId, int errorCode, const std::string& details)
{
  NS_LOG_INFO ("[UEC-E2E] [Control] Error event (placeholder) pdc_id=" << pdcId
               << " code=" << errorCode << " " << details);
}

void
SesManager::NotifyEagerSize (uint32_t eagerSize)
{
  NS_LOG_INFO ("[UEC-E2E] [Control] Eager size (placeholder) size=" << eagerSize);
}

void
SesManager::NotifyPause (bool paused)
{
  NS_LOG_INFO ("[UEC-E2E] [Control] Pause (placeholder) paused=" << paused);
}

bool
SesManager::ProcessReceiveResponse (const PdcSesResponse& response)
{
    NS_LOG_FUNCTION (this);
    NS_LOG_DEBUG ("Processing receive response from PDC " << response.pdc_id);
    m_totalResponses++;

    // Validate input parameters
    if (!ValidatePdcSesResponse (response))
    {
        NS_LOG_ERROR ("Invalid PDC SES response received");
        m_totalErrors++;
        return false;
    }

    try
    {
        // For now, simply process the response as successful data receipt
        // Future enhancement: add operation type tracking if needed
        NS_LOG_DEBUG ("Successfully processed response from PDC " << response.pdc_id
                     << " with packet length " << response.pkt_len);

        // Update statistics
        m_totalSuccessfulRequests++;

        return true;
    }
    catch (const std::exception& e)
    {
        NS_LOG_ERROR ("Exception in ProcessReceiveResponse: " << e.what ());
        m_totalErrors++;
        return false;
    }
}

bool
SesManager::SendResponseToPds (const SesPdsResponse& response)
{
    NS_LOG_FUNCTION (this);
    NS_LOG_DEBUG ("Sending response to PDS");

    // Validate input parameters
    if (!ValidateSesPdsResponse (response))
    {
        NS_LOG_ERROR ("Invalid SES PDS response");
        m_totalErrors++;
        return false;
    }

    // Check if PDS manager is available
    if (!m_pdsManager)
    {
        NS_LOG_ERROR ("PDS Manager not available");
        m_totalErrors++;
        return false;
    }

    try
    {
        // For now, SES does not directly send responses to PDS
        // This would be handled through the normal PDC processing flow
        NS_LOG_DEBUG ("SES response processed (PDS communication handled through PDC flow)");
        m_totalSuccessfulRequests++;

        return true;
    }
    catch (const std::exception& e)
    {
        NS_LOG_ERROR ("Exception in SendResponseToPds: " << e.what ());
        m_totalErrors++;
        return false;
    }
}

size_t
SesManager::GetRequestQueueSize (void) const
{
    return m_requestQueue.size ();
}

bool
SesManager::HasPendingOperations (void) const
{
    return !m_requestQueue.empty () ||
           !m_recvRequestQueue.empty () ||
           !m_recvResponseQueue.empty ();
}

Ptr<MsnTable>
SesManager::GetMsnTable (void) const
{
    return m_msnTable;
}

void
SesManager::SetJobIdValidator (Callback<bool, uint64_t> callback)
{
    m_jobIdValidator = callback;
}

void
SesManager::SetPermissionChecker (Callback<bool, uint32_t, uint64_t> callback)
{
    m_permissionChecker = callback;
}

void
SesManager::SetMemoryRegionValidator (Callback<bool, uint64_t> callback)
{
    m_memoryRegionValidator = callback;
}

void
SesManager::SetPacketReceivedCallback (Callback<void, Ptr<ExtendedOperationMetadata>> callback)
{
    m_packetReceivedCallback = callback;
}

void
SesManager::SetPacketSentCallback (Callback<void, Ptr<ExtendedOperationMetadata>> callback)
{
    m_packetSentCallback = callback;
}

void
SesManager::SetDetailedLogging (bool enable)
{
    m_detailedLogging = enable;
}

std::string
SesManager::GetStatistics (void) const
{
    std::ostringstream oss;
    oss << "SES Manager Statistics:"
        << "\n  Total Send Requests: " << m_totalSendRequests
        << "\n  Total Receive Requests: " << m_totalReceiveRequests
        << "\n  Total Responses: " << m_totalResponses
        << "\n  Total Errors: " << m_totalErrors
        << "\n  Packets Generated: " << m_totalPacketsGenerated
        << "\n  Packets Consumed: " << m_totalPacketsConsumed;
    return oss.str ();
}

void
SesManager::ResetStatistics (void)
{
    m_totalSendRequests = 0;
    m_totalReceiveRequests = 0;
    m_totalResponses = 0;
    m_totalSuccessfulRequests = 0;
    m_totalErrors = 0;
    m_totalPacketsGenerated = 0;
    m_totalPacketsConsumed = 0;
}

SesPdsRequest
SesManager::InitializeSesHeader (Ptr<ExtendedOperationMetadata> metadata)
{
    NS_LOG_FUNCTION (this << metadata);

    SesPdsRequest request;

    // Get source and destination FEP addresses from metadata
    request.src_fep = metadata->GetSourceNodeId ();
    request.dst_fep = metadata->GetDestinationNodeId ();
    request.mode = 1;     // Default mode
    request.rod_context = GenerateMessageId (metadata);
    request.next_hdr = PDSNextHeader::UET_HDR_REQUEST_STD;
    request.tc = 0;       // Default traffic class
    request.lock_pdc = false;
    request.tx_pkt_handle = 0;
    request.packet = nullptr;  // Packet will be provided by caller

    NS_LOG_DEBUG ("SES header initialized with context: " << request.rod_context);

    return request;
}

uint16_t
SesManager::GenerateMessageId (Ptr<ExtendedOperationMetadata> metadata)
{
    NS_LOG_FUNCTION (this << metadata);

    if (!metadata)
    {
        NS_LOG_WARN ("GenerateMessageId: metadata is null");
        return 0;
    }

    // Check for potential overflow before increment
    if (m_currentMessageId >= m_maxMessageId - 1)
    {
        NS_LOG_WARN ("Message ID counter approaching maximum value, resetting to avoid overflow");
        m_currentMessageId = 1; // Reset to 1 instead of 0 (0 is often reserved)

        // Log the counter reset for debugging
        LogDetailed ("GenerateMessageId", "Message ID counter reset due to overflow protection");
    }
    else
    {
        m_currentMessageId++;
    }

    uint16_t messageId = m_currentMessageId;

    NS_LOG_DEBUG ("Generated message ID: " << messageId);

    return messageId;
}

uint64_t
SesManager::CalculateBufferOffset (uint64_t rkey, uint64_t startAddr)
{
    NS_LOG_FUNCTION (this << rkey << startAddr);

    // Simple implementation: just use the start address
    // TODO: Implement actual buffer offset calculation
    return startAddr;
}

Ptr<ExtendedOperationMetadata>
SesManager::ParseReceivedRequest (const PdcSesRequest& request)
{
    NS_LOG_FUNCTION (this);

    // TODO: Implement actual request parsing
    return nullptr;
}

MemoryRegion
SesManager::DecodeRkeyToMr (uint64_t rkey)
{
    NS_LOG_FUNCTION (this << rkey);

    // TODO: Implement actual RKEY decoding
    MemoryRegion region;
    region.start_addr = rkey;  // Simple mapping for now
    region.length = 4096;      // Default 4KB region
    return region;
}

MemoryRegion
SesManager::LookupMrByKey (uint64_t key)
{
    NS_LOG_FUNCTION (this << key);

    // TODO: Implement actual memory region lookup
    MemoryRegion region;
    region.start_addr = key;  // Simple mapping for now
    region.length = 4096;      // Default 4KB region
    return region;
}

SesPdsResponse
SesManager::GenerateNackResponse (ResponseReturnCode errorCode,
                                 const PdcSesRequest& originalRequest)
{
    NS_LOG_FUNCTION (this << static_cast<uint32_t>(errorCode));

    SesPdsResponse response;
    response.pdc_id = 0;  // Will be set by PDS
    response.src_fep = 0; // Will be set by source
    response.dst_fep = 0; // Will be set by destination
    response.rx_pkt_handle = 0;
    response.gtd_del = false;
    response.ses_nack = true;  // This is a NACK
    response.nack_payload.nack_code = static_cast<uint8_t>(errorCode);
    response.packet = Create<Packet> (0);  // Empty packet for NACK
    response.rsp_len = 0;

    return response;
}

bool
SesManager::ValidateVersion (uint8_t version)
{
    NS_LOG_FUNCTION (this << version);
    return version == 1; // Only support version 1 for now
}

bool
SesManager::ValidateHeaderType (SESHeaderType type)
{
    NS_LOG_FUNCTION (this << static_cast<uint8_t>(type));
    return true; // TODO: Implement proper validation when SESHeaderType is defined
}

bool
SesManager::ValidatePidOnFep (uint32_t pidOnFep, uint32_t jobId, bool relative)
{
    NS_LOG_FUNCTION (this << pidOnFep << jobId << relative);
    return true; // TODO: Implement actual validation
}

bool
SesManager::ValidateOperation (OpType opcode)
{
    NS_LOG_FUNCTION (this << static_cast<uint8_t>(opcode));
    return (opcode >= OpType::SEND && opcode <= OpType::DEFERRABLE);
}

bool
SesManager::ValidateDataLength (size_t expectedLength, size_t actualLength)
{
    NS_LOG_FUNCTION (this << expectedLength << actualLength);
    return expectedLength == actualLength;
}

bool
SesManager::ValidatePdcStatus (uint32_t pdcId, uint64_t psn)
{
    NS_LOG_FUNCTION (this << pdcId << psn);
    return true; // TODO: Implement actual validation
}

bool
SesManager::ValidateMemoryKey (uint64_t rkey, uint32_t messageId)
{
    NS_LOG_FUNCTION (this << rkey << messageId);
    return true; // TODO: Implement actual validation
}

bool
SesManager::ValidateMsn (uint64_t jobId, uint64_t psn, uint64_t expectedLength,
                        uint32_t pdcId, bool isFirstPacket, bool isLastPacket)
{
    NS_LOG_FUNCTION (this << jobId << psn << expectedLength << pdcId
                         << isFirstPacket << isLastPacket);
    return true; // TODO: Implement actual validation
}

bool
SesManager::ShouldSendAck (uint32_t messageId, bool deliveryComplete)
{
    NS_LOG_FUNCTION (this << messageId << deliveryComplete);
    return deliveryComplete; // Simple implementation
}

void
SesManager::DoPeriodicProcessing (void)
{
    NS_LOG_FUNCTION (this);

    // TODO: Implement actual periodic processing
}

void
SesManager::ScheduleProcessing (void)
{
    NS_LOG_FUNCTION (this);

    // TODO: Implement actual scheduling
}

void
SesManager::LogDetailed (const std::string& function, const std::string& message) const
{
    if (m_detailedLogging)
    {
        NS_LOG_DEBUG (function << ": " << message);
    }
}

void
SesManager::LogError (const std::string& function, const std::string& error) const
{
    NS_LOG_ERROR (function << ": " << error);
    m_errorTrace (function + ": " + error);
}

bool
SesManager::ValidateOperationMetadata (Ptr<ExtendedOperationMetadata> metadata) const
{
    NS_LOG_FUNCTION (this << metadata);

    if (!metadata)
    {
        NS_LOG_WARN ("ValidateOperationMetadata: metadata is null");
        return false;
    }

    // Use the built-in validation
    if (!metadata->IsValid ())
    {
        NS_LOG_WARN ("Extended operation metadata is not valid");
        return false;
    }

    // Validate source endpoint
    uint32_t srcNodeId = metadata->GetSourceNodeId ();
    uint16_t srcEndpointId = metadata->GetSourceEndpointId ();
    if (srcNodeId == 0 || srcEndpointId == 0)
    {
        NS_LOG_WARN ("Invalid source endpoint: NodeId=" << srcNodeId
                           << ", EndpointId=" << srcEndpointId);
        return false;
    }

    // Validate destination endpoint
    uint32_t dstNodeId = metadata->GetDestinationNodeId ();
    uint16_t dstEndpointId = metadata->GetDestinationEndpointId ();
    if (dstNodeId == 0 || dstEndpointId == 0)
    {
        NS_LOG_WARN ("Invalid destination endpoint: NodeId=" << dstNodeId
                                 << ", EndpointId=" << dstEndpointId);
        return false;
    }

    // C2: Authorization placeholder (e.g. capability/token check; currently always allow)
    if (!ValidateAuthorization (metadata))
    {
        NS_LOG_WARN ("Authorization check failed (placeholder)");
        return false;
    }

    NS_LOG_DEBUG ("Operation metadata validation successful");
    return true;
}

bool
SesManager::ValidateAuthorization (Ptr<ExtendedOperationMetadata> metadata) const
{
  NS_LOG_FUNCTION (this << metadata);
  // C2: Authorization placeholder (e.g. capability/token check); currently always allow
  (void) metadata;
  return true;
}

SesManager::SesState
SesManager::GetState (void) const
{
    return m_state;
}

bool
SesManager::IsBusy (void) const
{
    return m_state == SES_BUSY;
}

bool
SesManager::IsError (void) const
{
    return m_state == SES_ERROR;
}

void
SesManager::Reset (void)
{
    NS_LOG_FUNCTION (this);

    // Clear queues safely - ensure proper cleanup
    while (!m_requestQueue.empty ())
    {
        // ExtendedOperationMetadata will be cleaned up automatically by ns-3 Ptr
        m_requestQueue.pop ();
    }

    while (!m_recvRequestQueue.empty ())
    {
        // PdcSesRequest objects will be cleaned up automatically when popped
        m_recvRequestQueue.pop ();
    }

    while (!m_recvResponseQueue.empty ())
    {
        // PdcSesResponse objects will be cleaned up automatically when popped
        m_recvResponseQueue.pop ();
    }

    // Reset state
    m_state = SES_IDLE;
    m_currentMessageId = 1;

    // Clear processing event
    if (m_processEventId.IsPending ())
    {
        Simulator::Cancel (m_processEventId);
    }

    NS_LOG_INFO ("SesManager reset to IDLE state");
}

bool
SesManager::ValidatePdcSesRequest (const PdcSesRequest& request) const
{
    NS_LOG_FUNCTION (this);

    // Check if packet exists
    if (!request.packet)
    {
        NS_LOG_WARN ("PDC SES request has null packet");
        return false;
    }

    // Validate packet size
    if (request.packet->GetSize () > m_maxMtu)
    {
        NS_LOG_WARN ("Packet size " << request.packet->GetSize ()
                     << " exceeds MTU " << m_maxMtu);
        return false;
    }

    // Validate PDC ID
    if (request.pdc_id == 0)
    {
        NS_LOG_WARN ("Invalid PDC ID (0) in request");
        return false;
    }

    return true;
}

bool
SesManager::ValidatePdcSesResponse (const PdcSesResponse& response) const
{
    NS_LOG_FUNCTION (this);

    // Validate PDC ID
    if (response.pdc_id == 0)
    {
        NS_LOG_WARN ("Invalid PDC ID (0) in response");
        return false;
    }

    // Validate packet if present
    if (response.packet && response.packet->GetSize () > m_maxMtu)
    {
        NS_LOG_WARN ("Response packet size " << response.packet->GetSize ()
                     << " exceeds MTU " << m_maxMtu);
        return false;
    }

    return true;
}

bool
SesManager::ValidateSesPdsResponse (const SesPdsResponse& response) const
{
    NS_LOG_FUNCTION (this);

    // Validate PDC ID
    if (response.pdc_id == 0)
    {
        NS_LOG_WARN ("Invalid PDC ID (0) in SES PDS response");
        return false;
    }

    // Validate packet if present
    if (response.packet && response.packet->GetSize () > m_maxMtu)
    {
        NS_LOG_WARN ("SES PDS response packet size " << response.packet->GetSize ()
                     << " exceeds MTU " << m_maxMtu);
        return false;
    }

    return true;
}

bool
SesManager::ProcessDataOperationResponse (const PdcSesResponse& response)
{
    NS_LOG_FUNCTION (this);

    try
    {
        // For now, simply log the successful processing of data operation response
        // Future enhancement: add response code analysis when available
        NS_LOG_DEBUG ("Data operation response processed successfully from PDC "
                     << response.pdc_id << " with packet length " << response.pkt_len);

        return true;
    }
    catch (const std::exception& e)
    {
        NS_LOG_ERROR ("Exception in ProcessDataOperationResponse: " << e.what ());
        return false;
    }
}

bool
SesManager::ProcessAtomicOperationResponse (const PdcSesResponse& response)
{
    NS_LOG_FUNCTION (this);

    try
    {
        // For now, simply log the successful processing of atomic operation response
        // Future enhancement: add atomic operation specific logic when needed
        NS_LOG_DEBUG ("Atomic operation response processed successfully from PDC "
                     << response.pdc_id << " with packet length " << response.pkt_len);

        return true;
    }
    catch (const std::exception& e)
    {
        NS_LOG_ERROR ("Exception in ProcessAtomicOperationResponse: " << e.what ());
        return false;
    }
}

} // namespace ns3

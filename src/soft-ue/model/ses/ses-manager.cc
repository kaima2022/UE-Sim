#include "ses-manager.h"
#include "ns3/log.h"
#include "ns3/assert.h"
#include <sstream>
#include "../pds/pds-manager.h"
#include "../network/soft-ue-net-device.h"

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
    : m_totalSendRequests (0),
      m_totalReceiveRequests (0),
      m_totalResponses (0),
      m_totalSuccessfulRequests (0),
      m_totalErrors (0),
      m_totalPacketsGenerated (0),
      m_totalPacketsConsumed (0)
{
    NS_LOG_FUNCTION (this);
    NS_LOG_DEBUG ("SesManager created");
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

    // Initialize internal components and state
    // TODO: Add specific initialization logic as needed

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
    NS_LOG_FUNCTION (this << metadata);

    if (!metadata)
    {
        NS_LOG_WARN ("ProcessSendRequest: metadata is null");
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

    // Create SES header and forward to PDS
    SesPdsRequest sesRequest = InitializeSesHeader (metadata);

    if (!m_pdsManager)
    {
        NS_LOG_ERROR ("PDS Manager not available");
        m_totalErrors++;
        return false;
    }

    // Add entry to MSN table for tracking
    uint64_t messageId = GenerateMessageId (metadata);
    uint32_t estimatedSize = 1024; // Default 1KB estimate
    if (!m_msnTable->AddEntry (messageId, 0, estimatedSize))
    {
        NS_LOG_WARN ("Failed to add MSN entry for message " << messageId);
    }
    sesRequest.rod_context = messageId;

    // Forward request to PDS manager
    bool success = m_pdsManager->ProcessSesRequest (sesRequest);

    if (success)
    {
        NS_LOG_INFO ("Send request processed successfully");
        m_totalSuccessfulRequests++;
    }
    else
    {
        NS_LOG_ERROR ("PDS manager failed to process send request");
        m_totalErrors++;
    }

    return success;
}

bool
SesManager::ProcessReceiveRequest (const PdcSesRequest& request)
{
    NS_LOG_FUNCTION (this);
    NS_LOG_DEBUG ("Processing receive request");
    m_totalReceiveRequests++;

    // TODO: Implement actual receive processing
    return true;
}

bool
SesManager::ProcessReceiveResponse (const PdcSesResponse& response)
{
    NS_LOG_FUNCTION (this);
    NS_LOG_DEBUG ("Processing receive response");
    m_totalResponses++;

    // TODO: Implement actual response processing
    return true;
}

bool
SesManager::SendResponseToPds (const SesPdsResponse& response)
{
    NS_LOG_FUNCTION (this);
    NS_LOG_DEBUG ("Sending response to PDS");

    // TODO: Implement actual response sending
    return true;
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

    // Initialize with default values
    // TODO: Implement actual header initialization logic based on correct struct definition
    request.src_fep = 0;  // Will be set by network device
    request.dst_fep = 0;  // Will be set by destination endpoint
    request.mode = 1;     // Default mode
    request.rod_context = GenerateMessageId (metadata);
    request.next_hdr = PDSNextHeader::UET_HDR_REQUEST_STD;
    request.tc = 0;       // Default traffic class
    request.lock_pdc = false;
    request.tx_pkt_handle = 0;
    request.packet = Create<Packet> (0);  // Empty packet for now

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

    // Simple implementation: use current counter and increment
    uint16_t messageId = m_currentMessageId;
    m_currentMessageId = (m_currentMessageId + 1) % m_maxMessageId;

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
    NS_LOG_FUNCTION (this << jobId << psn << expectedLength << pdcId << isFirstPacket << isLastPacket);
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
        NS_LOG_WARN ("Invalid source endpoint: NodeId=" << srcNodeId << ", EndpointId=" << srcEndpointId);
        return false;
    }

    // Validate destination endpoint
    uint32_t dstNodeId = metadata->GetDestinationNodeId ();
    uint16_t dstEndpointId = metadata->GetDestinationEndpointId ();
    if (dstNodeId == 0 || dstEndpointId == 0)
    {
        NS_LOG_WARN ("Invalid destination endpoint: NodeId=" << dstNodeId << ", EndpointId=" << dstEndpointId);
        return false;
    }

    NS_LOG_DEBUG ("Operation metadata validation successful");
    return true;
}

} // namespace ns3

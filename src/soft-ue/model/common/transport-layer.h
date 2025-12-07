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
 * @file             transport-layer.h
 * @brief            Ultra Ethernet Transport Layer Protocol Definitions
 * @author           softuegroup@gmail.com
 * @version          1.0.0
 * @date             2025-12-07
 * @copyright        Apache License Version 2.0
 *
 * @details
 * This file contains protocol definitions and data structures for the
 * Ultra Ethernet transport layer, adapted for ns-3 simulation.
 */

#ifndef SOFT_UE_TRANSPORT_LAYER_H
#define SOFT_UE_TRANSPORT_LAYER_H

#include <stdint.h>
#include <string>
#include <ns3/object.h>
#include <ns3/packet.h>

namespace ns3 {

//=============================================================================
// Configuration Constants
//=============================================================================

#define MAX_MTU 4096                           // Maximum transmission unit
#define MAX_QUEUE_SIZE 512                      // Maximum queue size
#define MAX_PDC 512                            // Maximum number of PDCs per type
#define Base_RTO 100                           // Base retransmission timeout (ms)
#define Pend_Timeout 100                       // Pending timeout (ms)
#define Close_Thresh 4                         // Connection close threshold

// UET protocol configuration
#define UET_Over_UDP 1                         // UET runs over UDP
#define UDP_Dest_Port 2887                     // UDP port for UET protocol
#define Limit_PSN_Range 1                      // Limit PSN range for security
#define Default_MPR 8                          // Default MPR value

// Retransmission parameters
#define RTO_Init_Time 4                        // Initial RTO multiplier
#define Max_RTO_Retx_Cnt 5                     // Maximum retransmission count
#define NACK_Retx_Time 4                       // NACK retransmission delay
#define Max_NACK_Retx_Cnt 5                    // Maximum NACK retransmission count

//=============================================================================
// Enumeration Types
//=============================================================================

/**
 * @brief Operation types for Ultra Ethernet
 */
enum class OpType : uint8_t
{
    SEND = 1,          // Standard send operation
    READ = 2,          // RMA read operation
    WRITE = 3,         // RMA write operation
    DEFERRABLE = 4     // Deferred send operation
};

/**
 * @brief PDS header type enumeration
 */
enum class PDSHeaderType : uint8_t
{
    ENTROPY_HEADER,    // Entropy header for path selection
    RUOD_REQ_HEADER,   // RUOD request header
    RUOD_ACK_HEADER,   // RUOD acknowledgment header
    RUOD_CP_HEADER,    // RUOD control packet header
    NACK_HEADER        // Negative acknowledgment header
};

/**
 * @brief PDS packet type enumeration
 */
enum class PDSType : uint8_t
{
    RESERVED,          // Reserved type
    TSS,              // UET encryption header
    RUD_REQ,          // RUD request (Reliable Unordered Delivery)
    ROD_REQ,          // ROD request (Reliable Ordered Delivery)
    RUDI_REQ,         // RUDI request (Reliable Unordered Delivery with Immediate response)
    RUDI_RESP,        // RUDI response
    UUD_REQ,          // UUD request (Unreliable Unordered Delivery)
    ACK,              // Acknowledgment packet
    ACK_CC,           // Congestion control acknowledgment packet
    NACK,             // Negative acknowledgment packet
    CP                // Control packet
};

/**
 * @brief PDS next header type enumeration
 */
enum class PDSNextHeader : uint8_t
{
    UET_HDR_REQUEST_SMALL,      // Small request header
    UET_HDR_REQUEST_MEDIUM,     // Medium request header
    UET_HDR_REQUEST_STD,        // Standard request header
    UET_HDR_RESPONSE,           // Response header
    UET_HDR_RESPONSE_DATA,      // Response header with data
    UET_HDR_RESPONSE_DATA_SMALL,// Response header with small data
    PAYLOAD,                    // Raw payload data
    UET_HDR_NONE                // No header
};

/**
 * @brief PDC type enumeration
 */
enum class PdcType : uint8_t
{
    IPDC,              // Unreliable PDC
    TPDC               // Reliable PDC
};

/**
 * @brief SES header type enumeration
 */
enum class SESHeaderType : uint8_t
{
    STANDARD_HEADER,
    OPTIMIZED_HEADER,
    SMALL_MESSAGE_RMA_HEADER,
    SEMANTIC_RESPONSE_HEADER,
    SEMANTIC_RESPONSE_WITH_DATA_HEADER,
    OPTIMIZED_RESPONSE_WITH_DATA_HEADER
};

/**
 * @brief Response return codes
 */
enum class ResponseReturnCode : uint8_t
{
    RC_OK = 0x00,                    // Normal operation
    RC_PARTIAL_WRITE = 0x01,         // Partial data write
    RC_NO_MATCH = 0x02,              // Tagged send no matching buffer
    RC_INVALID_OP = 0x03,            // Invalid operation code
    RC_NO_BUFFER = 0x10,             // No available receive buffer
    RC_INVALID_KEY = 0x11,           // RMA memory key invalid/expired
    RC_ACCESS_DENIED = 0x12,         // JobID no access permission
    RC_ADDR_UNREACHABLE = 0x13,      // Target address unreachable
    RC_SECURITY_DOMAIN_MISMATCH = 0x20, // Security domain verification failed
    RC_INTEGRITY_CHECK_FAIL = 0x21, // Packet integrity check failed
    RC_REPLAY_DETECTED = 0x22,       // Replay attack detected
    RC_INTERNAL_ERROR = 0x30,        // SES internal state abnormal
    RC_RESOURCE_EXHAUST = 0x31,      // System resource exhausted
    RC_PROTOCOL_ERROR = 0x32         // Protocol violation
};

/**
 * @brief Response operation codes
 */
enum class ResponseOpCode : uint8_t
{
    UET_RESPONSE = 0x00,             // General response
    UET_DEFAULT_RESPONSE = 0x01,     // Default success response
    UET_RESPONSE_W_DATA = 0x02,      // Response with data
    UET_NO_RESPONSE = 0x03,          // Empty response
    UET_NACK = 0x04                  // Error response
};

/**
 * @brief NACK codes for error handling
 */
enum class NackCode : uint8_t
{
    SEQ_GAP = 0x01,          // Sequence gap detected
    RESOURCE = 0x02,         // Resource unavailable
    ACCESS_DENIED = 0x03,    // Access denied
    INVALID_OPCODE = 0x04,   // Invalid operation code
    CHECKSUM = 0x05,         // Checksum error
    TTL_EXCEEDED = 0x06,     // TTL exceeded
    PROTOCOL = 0x07          // Protocol error
};

//=============================================================================
// Core Data Structures
//=============================================================================

/**
 * @brief Operation metadata for SES layer
 */
struct OperationMetadata
{
    OpType op_type;                        // Operation type

    // Memory region information
    struct {
        uint64_t rkey;                    // Registered memory key
        bool idempotent_safe;             // Idempotent operation safety flag
    } memory;

    // Data payload information
    struct {
        uint64_t start_addr;              // Data start address
        size_t length;                    // Data length
        uint64_t imm_data;                // Immediate data (optional)
    } payload;

    uint32_t s_pid_on_fep;                // Source endpoint process ID
    uint32_t t_pid_on_fep;                // Target endpoint process ID
    uint32_t job_id;                      // Job identifier
    uint16_t res_index;                   // Resource index
    uint32_t messages_id;                 // Message identifier

    // Operation flags
    bool relative;                        // Relative addressing flag
    bool use_optimized_header;            // Optimized header usage flag
    bool has_imm_data;                    // Immediate data presence flag

    // Constructor
    OperationMetadata()
        : op_type(OpType::SEND)
        , memory({0, false})
        , payload({0, 0, 0})
        , s_pid_on_fep(0)
        , t_pid_on_fep(0)
        , job_id(0)
        , res_index(0)
        , messages_id(0)
        , relative(false)
        , use_optimized_header(false)
        , has_imm_data(false)
    {}
};

/**
 * @brief UET Address structure
 */
struct UETAddress
{
    uint8_t version;                      // Address format version
    uint16_t flags;                       // Valid field flag bits

    // Capability identifiers
    struct {
        bool ai_base : 1;                // AI basic profile support
        bool ai_full : 1;                // AI full profile support
        bool hpc : 1;                    // HPC profile support
    } capabilities;

    uint16_t pid_on_fep;                 // Process ID on endpoint
    struct {
        uint64_t low;                    // Low 64 bits of fabric address
        uint64_t high;                   // High 64 bits of fabric address
    } fabric_addr;
    uint16_t start_res_index;            // Starting resource index
    uint16_t num_res_indices;            // Number of resource indices
    uint32_t initiator_id;               // Initiator ID
};

/**
 * @brief Memory region structure
 */
struct MemoryRegion
{
    uint64_t start_addr;                 // Memory region start address
    size_t length;                       // Memory region length
};

/**
 * @brief MSN (Message Sequence Number) entry for tracking
 */
struct MSNEntry
{
    uint64_t last_psn;                   // Last received packet sequence number
    uint64_t expected_len;               // Message expected total length
    uint32_t pdc_id;                     // Associated PDC
};

/**
 * @brief NACK payload structure
 */
struct NackPayload
{
    uint8_t nack_code;                   // NACK type enumeration value
    uint64_t expected_psn;               // Expected PSN (for SEQ_GAP)
    uint32_t current_window;             // Current receive window size
};

//=============================================================================
// Inter-layer Communication Structures
//=============================================================================

/**
 * @brief Request structure from PDC to SES layer
 */
struct PdcSesRequest
{
    uint16_t pdc_id;                     // PDC identifier
    uint16_t rx_pkt_handle;              // Receive packet handle
    Ptr<Packet> packet;                  // Actual packet data
    uint16_t pkt_len;                    // Packet length
    PDSNextHeader next_hdr;              // Next header type
    uint16_t orig_pdcid;                 // Original destination PDCID
    uint32_t orig_psn;                   // Original packet sequence number
};

/**
 * @brief Response structure from PDC to SES layer
 */
struct PdcSesResponse
{
    uint16_t pdc_id;                     // PDC identifier
    uint16_t rx_pkt_handle;              // Receive packet handle
    Ptr<Packet> packet;                  // Response packet data
    uint16_t pkt_len;                    // Packet length
};

/**
 * @brief Request structure from SES layer to PDS
 */
struct SesPdsRequest
{
    uint32_t src_fep;                    // Source FEP
    uint32_t dst_fep;                    // Destination FEP
    uint8_t mode;                        // Transmission mode
    uint16_t rod_context;                // ROD context
    PDSNextHeader next_hdr;              // Next header type
    uint8_t tc;                          // Traffic control category
    bool lock_pdc;                       // PDC lock flag
    uint16_t tx_pkt_handle;              // Transmit packet handle
    Ptr<Packet> packet;                  // Packet data
    uint16_t pkt_len;                    // Packet length
    uint32_t tss_context;                // TSS context
    uint16_t rsv_pdc_context;            // Reserved PDC context
    uint16_t rsv_ccc_context;            // Reserved CCC context
    bool som;                            // Start of Message flag
    bool eom;                            // End of Message flag
};

/**
 * @brief Response structure from SES layer to PDC
 */
struct SesPdsResponse
{
    uint16_t pdc_id;                     // PDC identifier
    uint32_t src_fep;                    // Source FEP
    uint32_t dst_fep;                    // Destination FEP
    uint16_t rx_pkt_handle;              // Receive packet handle
    bool gtd_del;                        // Guaranteed delivery flag
    bool ses_nack;                       // SES layer NACK flag
    NackPayload nack_payload;            // NACK information payload
    Ptr<Packet> packet;                  // Response packet data
    uint16_t rsp_len;                    // Response length
};

//=============================================================================
// Utility Functions
//=============================================================================

/**
 * @brief Convert operation type to string
 * @param op The operation type
 * @return String representation
 */
inline std::string OperationTypeToString(OpType op)
{
    switch (op) {
        case OpType::SEND: return "SEND";
        case OpType::READ: return "READ";
        case OpType::WRITE: return "WRITE";
        case OpType::DEFERRABLE: return "DEFERRABLE";
        default: return "UNKNOWN";
    }
}

/**
 * @brief Convert PDC type to string
 * @param type The PDC type
 * @return String representation
 */
inline std::string PdcTypeToString(PdcType type)
{
    switch (type) {
        case PdcType::IPDC: return "IPDC";
        case PdcType::TPDC: return "TPDC";
        default: return "UNKNOWN";
    }
}

/**
 * @brief Check if PDC ID is IPDC type
 * @param pdc_id The PDC ID to check
 * @return true if IPDC type
 */
inline bool IsIpdc(uint16_t pdc_id)
{
    return pdc_id < MAX_PDC;
}

/**
 * @brief Check if PDC ID is TPDC type
 * @param pdc_id The PDC ID to check
 * @return true if TPDC type
 */
inline bool IsTpdc(uint16_t pdc_id)
{
    return pdc_id >= MAX_PDC;
}

/**
 * @brief Get PDC type from ID
 * @param pdc_id The PDC ID
 * @return PDC type
 */
inline PdcType GetPdcType(uint16_t pdc_id)
{
    return IsIpdc(pdc_id) ? PdcType::IPDC : PdcType::TPDC;
}

} // namespace ns3

#endif /* SOFT_UE_TRANSPORT_LAYER_H */
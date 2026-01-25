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
 * @file             pds-common.h
 * @brief            PDS (Packet Delivery Sub-layer) Common Definitions
 * @author           softuegroup@gmail.com
 * @version          1.0.0
 * @date             2025-12-07
 * @copyright        Apache License Version 2.0
 *
 * @details
 * This file contains common definitions and utilities for the PDS layer
 * of the Ultra Ethernet protocol implementation in ns-3.
 */

#ifndef PDS_COMMON_H
#define PDS_COMMON_H

#include "ns3/object.h"
#include "ns3/timer.h"
#include "ns3/ptr.h"
#include "ns3/vector.h"
#include "ns3/callback.h"
#include "ns3/traced-callback.h"
#include "../common/transport-layer.h"
#include "../ses/operation-metadata.h"
#include "../ses/msn-entry.h"

namespace ns3 {

// Forward declarations
class SesManager;
class PdcBase;
class PDSHeader;

/**
 * @enum DeliveryMode
 * @brief Packet delivery modes for Ultra Ethernet protocol
 */
enum class DeliveryMode : uint8_t
{
    RUD = 0,  ///< Reliable Unordered Delivery
    ROD = 1,  ///< Reliable Ordered Delivery
    UUD = 2,  ///< Unreliable Unordered Delivery
    UOD = 3   ///< Unreliable Ordered Delivery
};

/**
 * @class PDSHeader
 * @brief PDS layer packet header for Ultra Ethernet protocol
 */
class PDSHeader : public Header
{
public:
    /**
     * @brief Get the type ID for this class
     * @return TypeId
     */
    static TypeId GetTypeId (void);

    /**
     * @brief Get the instance type ID
     * @return TypeId
     */
    virtual TypeId GetInstanceTypeId (void) const override;

    /**
     * @brief Default constructor
     */
    PDSHeader ();

    /**
     * @brief Destructor
     */
    virtual ~PDSHeader ();

    /**
     * @brief Print the header to output stream
     * @param os Output stream
     */
    virtual void Print (std::ostream &os) const override;

    /**
     * @brief Get the serialized size of the header
     * @return Serialized size
     */
    virtual uint32_t GetSerializedSize (void) const override;

    /**
     * @brief Serialize the header to a buffer
     * @param start Buffer iterator
     */
    virtual void Serialize (Buffer::Iterator start) const override;

    /**
     * @brief Deserialize the header from a buffer
     * @param start Buffer iterator
     * @return Serialized size
     */
    virtual uint32_t Deserialize (Buffer::Iterator start) override;

    /**
     * @brief Set the PDC identifier
     * @param pdcId PDC identifier
     */
    void SetPdcId (uint16_t pdcId);

    /**
     * @brief Get the PDC identifier
     * @return PDC identifier
     */
    uint16_t GetPdcId (void) const;

    /**
     * @brief Set the sequence number
     * @param seqNum Sequence number
     */
    void SetSequenceNumber (uint32_t seqNum);

    /**
     * @brief Get the sequence number
     * @return Sequence number
     */
    uint32_t GetSequenceNumber (void) const;

    /**
     * @brief Set the start of message flag
     * @param som Start of message flag
     */
    void SetSom (bool som);

    /**
     * @brief Get the start of message flag
     * @return Start of message flag
     */
    bool GetSom (void) const;

    /**
     * @brief Set the end of message flag
     * @param eom End of message flag
     */
    void SetEom (bool eom);

    /**
     * @brief Get the end of message flag
     * @return End of message flag
     */
    bool GetEom (void) const;

private:
    uint16_t m_pdcId;        ///< PDC identifier
    uint32_t m_sequenceNum;  ///< Sequence number
    bool m_som;              ///< Start of message flag
    bool m_eom;              ///< End of message flag
};

/**
 * @enum PdsErrorCode
 * @brief Error codes used by the PDS layer
 */
enum class PdsErrorCode : uint8_t
{
    SUCCESS = 0,          ///< Operation successful
    INVALID_PDC = 1,      ///< Invalid PDC identifier
    PDC_FULL = 2,         ///< PDC table is full
    INVALID_PACKET = 3,   ///< Invalid packet format
    RESOURCE_EXHAUSTED = 4, ///< PDS resources exhausted
    PROTOCOL_ERROR = 5,  ///< Protocol violation
    INTERNAL_ERROR = 6   ///< Internal error
};

/**
 * @class PdsStatistics
 * @brief Statistics collection for PDS layer performance monitoring
 */
class PdsStatistics : public Object
{
public:
    /**
     * @brief Get the type ID for this class
     * @return TypeId
     */
    static TypeId GetTypeId (void);

    /**
     * @brief Get the instance type ID
     * @return TypeId
     */
    virtual TypeId GetInstanceTypeId (void) const override;

    /**
     * @brief Default constructor
     */
    PdsStatistics ();

    /**
     * @brief Destructor
     */
    virtual ~PdsStatistics ();

    /**
     * @brief Reset all statistics counters
     */
    void Reset (void);

    /**
     * @brief Increment packet received count
     */
    void IncrementReceivedPackets (void);

    /**
     * @brief Increment packet sent count
     */
    void IncrementSentPackets (void);

    /**
     * @brief Increment PDC creation count
     */
    void IncrementPdcCreations (void);

    /**
     * @brief Increment PDC destruction count
     */
    void IncrementPdcDestructions (void);

    /**
     * @brief Increment error count
     * @param error The error type that occurred
     */
    void IncrementErrors (PdsErrorCode error);

    /**
     * @brief Get statistics as formatted string
     * @return Formatted statistics string
     */
    std::string GetStatistics (void) const;

    /**
     * @brief Record packet transmission with timing
     * @param bytes Number of bytes transmitted
     * @param latency Packet transmission latency in nanoseconds
     */
    void RecordPacketTransmission (uint64_t bytes, double latency);

    /**
     * @brief Record packet reception with timing
     * @param bytes Number of bytes received
     * @param latency Packet reception latency in nanoseconds
     */
    void RecordPacketReception (uint64_t bytes, double latency);

    /**
     * @brief Record bytes sent (without latency tracking)
     * @param bytes Number of bytes sent
     */
    void RecordBytesSent (uint64_t bytes);

    /**
     * @brief Record bytes received (without latency tracking)
     * @param bytes Number of bytes received
     */
    void RecordBytesReceived (uint64_t bytes);

    /**
     * @brief Get calculated throughput in Mbps
     * @return Throughput in megabits per second
     */
    double GetThroughputMbps (void) const;

    /**
     * @brief Get average latency in nanoseconds
     * @return Average latency
     */
    double GetAverageLatencyNs (void) const;

    /**
     * @brief Get jitter in nanoseconds
     * @return Jitter (standard deviation of delays)
     */
    double GetJitterNs (void) const;

private:
    uint64_t m_receivedPackets;      ///< Total packets received
    uint64_t m_sentPackets;          ///< Total packets sent
    uint64_t m_pdcCreations;         ///< Total PDCs created
    uint64_t m_pdcDestructions;      ///< Total PDCs destroyed
    uint64_t m_successCount;         ///< Successful operations
    uint64_t m_invalidPdcCount;      ///< Invalid PDC operations
    uint64_t m_pdcFullCount;         ///< PDC full errors
    uint64_t m_invalidPacketCount;   ///< Invalid packet errors
    uint64_t m_resourceExhaustedCount; ///< Resource exhaustion errors
    uint64_t m_protocolErrorCount;   ///< Protocol violation errors

    // Data center level performance metrics
    uint64_t m_totalBytesReceived;   ///< Total bytes received
    uint64_t m_totalBytesSent;       ///< Total bytes sent
    Time m_firstPacketTime;          ///< First packet timestamp
    Time m_lastPacketTime;           ///< Last packet timestamp
    std::vector<Time> m_packetDelays; ///< Per-packet delays for jitter analysis
    double m_minLatency;             ///< Minimum packet latency (nanoseconds)
    double m_maxLatency;             ///< Maximum packet latency (nanoseconds)
    double m_totalLatency;           ///< Accumulated latency for average calculation
};

/**
 * @class PdcAllocator
 * @brief PDC allocation and management utility
 */
class PdcAllocator : public Object
{
public:
    /**
     * @brief Get the type ID for this class
     * @return TypeId
     */
    static TypeId GetTypeId (void);

    /**
     * @brief Get the instance type ID
     * @return TypeId
     */
    virtual TypeId GetInstanceTypeId (void) const override;

    /**
     * @brief Default constructor
     */
    PdcAllocator ();

    /**
     * @brief Destructor
     */
    virtual ~PdcAllocator ();

    /**
     * @brief Allocate a PDC identifier
     * @param pdcType Type of PDC to allocate (IPDC or TPDC)
     * @return Allocated PDC identifier, or 0 if allocation failed
     */
    uint16_t AllocatePdc (PdcType pdcType);

    /**
     * @brief Release a PDC identifier
     * @param pdcId PDC identifier to release
     * @return true if release was successful
     */
    bool ReleasePdc (uint16_t pdcId);

    /**
     * @brief Check if a PDC identifier is valid
     * @param pdcId PDC identifier to check
     * @return true if the PDC identifier is valid
     */
    bool IsValidPdcId (uint16_t pdcId) const;

    /**
     * @brief Get PDC type from identifier
     * @param pdcId PDC identifier
     * @return PDC type
     */
    PdcType GetPdcType (uint16_t pdcId) const;

    /**
     * @brief Get number of active PDCs by type
     * @param pdcType PDC type to count
     * @return Number of active PDCs
     */
    uint16_t GetActivePdcCount (PdcType pdcType) const;

    /**
     * @brief Get total number of active PDCs
     * @return Total number of active PDCs
     */
    uint16_t GetTotalActivePdcCount (void) const;

    /**
     * @brief Set maximum PDC limit per type
     * @param maxPdc Maximum number of PDCs per type
     */
    void SetMaxPdcLimit (uint16_t maxPdc);

    /**
     * @brief Get maximum PDC limit per type
     * @return Maximum PDC limit per type
     */
    uint16_t GetMaxPdcLimit (void) const;

private:
    static const uint16_t IPDC_BASE = 1;      ///< Base IPDC identifier
    static const uint16_t TPDC_BASE = 513;     ///< Base TPDC identifier
    static const uint16_t PDC_COUNT = 512;     ///< Number of PDCs per type

    uint16_t m_maxPdcLimit;                         ///< Maximum PDCs per type
    std::vector<bool> m_ipdcBitmap;               ///< IPDC allocation bitmap
    std::vector<bool> m_tpdcBitmap;               ///< TPDC allocation bitmap
};

/**
 * @class PdsPacketRouter
 * @brief Packet routing and dispatch logic for PDS layer
 */
class PdsPacketRouter : public Object
{
public:
    /**
     * @brief Get the type ID for this class
     * @return TypeId
     */
    static TypeId GetTypeId (void);

    /**
     * @brief Get the instance type ID
     * @return TypeId
     */
    virtual TypeId GetInstanceTypeId (void) const override;

    /**
     * @brief Default constructor
     */
    PdsPacketRouter ();

    /**
     * @brief Destructor
     */
    virtual ~PdsPacketRouter ();

    /**
     * @brief Route a packet to the appropriate PDC
     * @param packet Packet to route
     * @param pdcId Target PDC identifier
     * @return true if routing was successful
     */
    bool RoutePacket (Ptr<Packet> packet, uint16_t pdcId);

    /**
     * @brief Broadcast a packet to all active PDCs
     * @param packet Packet to broadcast
     * @param excludePdcId PDC to exclude from broadcast (optional)
     * @return Number of PDCs the packet was sent to
     */
    uint16_t BroadcastPacket (Ptr<Packet> packet, uint16_t excludePdcId = 0);

    /**
     * @brief Get packet destination from metadata
     * @param metadata Packet metadata
     * @return Destination PDC identifier, or 0 if not found
     */
    uint16_t GetDestinationPdc (const SesPdsRequest& request);

    /**
     * @brief Validate packet before routing
     * @param packet Packet to validate
     * @param pdcId Target PDC identifier
     * @return true if packet is valid for routing
     */
    bool ValidatePacket (Ptr<Packet> packet, uint16_t pdcId);

    /**
     * @brief Set SES manager callback
     * @param sesManager Pointer to SES manager
     */
    void SetSesManager (Ptr<SesManager> sesManager);

private:
    Ptr<SesManager> m_sesManager;             ///< Associated SES manager

    // Routing tables and caches
    std::unordered_map<uint32_t, uint16_t> m_jobIdToPdc;  ///< Job ID to PDC mapping
    std::unordered_map<uint16_t, uint32_t> m_pdcToJobId;  ///< PDC to Job ID mapping
};

} // namespace ns3

#endif /* PDS_COMMON_H */
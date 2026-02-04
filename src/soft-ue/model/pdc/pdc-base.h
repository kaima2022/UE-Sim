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
 * @file             pdc-base.h
 * @brief            PDC (Packet Delivery Context) Base Class
 * @author           softuegroup@gmail.com
 * @version          1.0.0
 * @date             2025-12-07
 * @copyright        Apache License Version 2.0
 *
 * @details
 * This file contains the base PDC class that implements common functionality
 * for both IPDC (Unreliable PDC) and TPDC (Reliable PDC) in the Ultra Ethernet
 * protocol stack.
 */

#ifndef PDC_BASE_H
#define PDC_BASE_H

#include "ns3/object.h"
#include "ns3/callback.h"
#include "ns3/timer.h"
#include "ns3/event-id.h"
#include "ns3/traced-callback.h"
#include "ns3/packet.h"
#include "ns3/drop-tail-queue.h"
#include <unordered_map>
#include "../common/transport-layer.h"
#include "../ses/operation-metadata.h"
#include "../pds/pds-common.h"

namespace ns3 {

// Forward declarations
class SesManager;
class SoftUeNetDevice;

/**
 * @struct UETPDSHeader
 * @brief Complete UET PDS header structure for internal processing
 */
struct UETPDSHeader
{
    uint8_t version;                            ///< Protocol version
    uint8_t reserved;                           ///< Reserved field
    uint8_t tc;                                ///< Traffic control category
    uint32_t dest_fep;                         ///< Destination fabric endpoint
    uint32_t src_fep;                          ///< Source fabric endpoint
    bool som;                                  ///< Start of message flag
    bool eom;                                  ///< End of message flag
    PDSNextHeader next_hdr;                    ///< Next header type
    uint32_t msg_len;                          ///< Message length
    uint16_t rsv_pdc_context;                  ///< Reserved PDC context
    uint16_t rsv_ccc_context;                  ///< Reserved CCC context

    UETPDSHeader()
        : version(1), reserved(0), tc(0), dest_fep(0), src_fep(0),
          som(false), eom(false), next_hdr(PDSNextHeader::PAYLOAD),
          msg_len(0), rsv_pdc_context(0), rsv_ccc_context(0)
    {}
};

/**
 * @struct PdcConfig
 * @brief PDC configuration parameters
 */
struct PdcConfig
{
    uint16_t pdcId;                           ///< PDC identifier
    uint32_t localFep;                        ///< Local fabric endpoint
    uint32_t remoteFep;                       ///< Remote fabric endpoint
    uint8_t tc;                              ///< Traffic control category
    PdcType type;                            ///< PDC type (IPDC or TPDC)
    DeliveryMode deliveryMode;               ///< Delivery mode
    uint16_t maxPacketSize;                   ///< Maximum packet size
    uint16_t rtoPdcContext;                   ///< RTO PDC context
    uint16_t rtoCccContext;                   ///< RTO CCC context
    Time rtoInitial;                          ///< Initial RTO timeout
    Time rtoMax;                              ///< Maximum RTO timeout
    bool detailedLogging;                     ///< Enable detailed logging
    uint32_t sequenceNumber;                  ///< Current sequence number

    PdcConfig ()
        : pdcId (0), localFep (0), remoteFep (0), tc (0),
          type (PdcType::IPDC), deliveryMode (DeliveryMode::RUD),
          maxPacketSize (1500), rtoPdcContext (0), rtoCccContext (0),
          rtoInitial (MilliSeconds (100)), rtoMax (Seconds (1)),
          detailedLogging (false), sequenceNumber (1)
    {}
};

/**
 * @struct PdcStatistics
 * @brief PDC performance statistics
 */
struct PdcStatistics
{
    uint64_t packetsSent;                     ///< Total packets sent
    uint64_t packetsReceived;                 ///< Total packets received
    uint64_t bytesTransmitted;                ///< Total bytes transmitted
    uint64_t bytesReceived;                   ///< Total bytes received
    uint64_t retransmissions;                 ///< Number of retransmissions (TPDC only)
    uint64_t timeouts;                        ///< Number of timeouts (TPDC only)
    uint64_t errors;                          ///< Total number of errors
    uint64_t validationErrors;                ///< Number of packet validation errors
    uint64_t protocolErrors;                 ///< Number of protocol errors
    uint64_t bufferErrors;                   ///< Number of buffer overflow errors
    uint64_t networkErrors;                  ///< Number of network transmission errors
    uint64_t packetsDropped;                  ///< Number of packets dropped due to queue overflow
    std::string lastErrorDetails;           ///< Details of the last error
    Time lastActivity;                        ///< Last activity timestamp
    double averageLatency;                    ///< Average packet latency in microseconds
    double minLatency;                        ///< Minimum packet latency in microseconds
    double maxLatency;                        ///< Maximum packet latency in microseconds
    Time startTime;                            ///< Statistics collection start time
    double throughputGbps;                    ///< Current throughput in Gbps

    PdcStatistics ()
        : packetsSent (0), packetsReceived (0), bytesTransmitted (0), bytesReceived (0),
          retransmissions (0), timeouts (0), errors (0), validationErrors (0),
          protocolErrors (0), bufferErrors (0), networkErrors (0), packetsDropped (0),
          lastErrorDetails (""), lastActivity (Seconds (0)), averageLatency (0.0),
          minLatency (0.0), maxLatency (0.0), startTime (Simulator::Now ()), throughputGbps (0.0)
    {}

    /**
     * @brief Reset all statistics
     */
    void Reset ()
    {
        packetsSent = 0;
        packetsReceived = 0;
        bytesTransmitted = 0;
        bytesReceived = 0;
        retransmissions = 0;
        timeouts = 0;
        errors = 0;
        validationErrors = 0;
        protocolErrors = 0;
        bufferErrors = 0;
        networkErrors = 0;
        packetsDropped = 0;
        lastErrorDetails.clear ();
        lastActivity = Seconds (0);
        averageLatency = 0.0;
        minLatency = 0.0;
        maxLatency = 0.0;
        startTime = Simulator::Now ();
        throughputGbps = 0.0;
    }

    /**
     * @brief Update throughput calculation
     */
    void UpdateThroughput ()
    {
        Time elapsed = Simulator::Now () - startTime;
        if (elapsed.GetSeconds () > 0)
          {
            double bytesPerSec = static_cast<double> (bytesTransmitted + bytesReceived) / elapsed.GetSeconds ();
            throughputGbps = (bytesPerSec * 8) / (1024.0 * 1024.0 * 1024.0); // Convert to Gbps
          }
    }
};

/**
 * @class PdcBase
 * @brief Base PDC class implementing common functionality
 *
 * The PDC base class provides common functionality for both IPDC (unreliable)
 * and TPDC (reliable) packet delivery contexts. It handles packet formatting,
 * header management, and provides the foundation for reliability mechanisms
 * in TPDC subclasses.
 */
class PdcBase : public Object
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
    virtual TypeId GetInstanceTypeId (void) const override = 0;

    /**
     * @brief Constructor
     */
    PdcBase ();

    /**
     * @brief Destructor
     */
    virtual ~PdcBase ();

    /**
     * @brief Initialize PDC with configuration
     * @param config PDC configuration
     * @return true if initialization was successful
     */
    virtual bool Initialize (const PdcConfig& config);

    /**
     * @brief Get PDC identifier
     * @return PDC identifier
     */
    uint16_t GetPdcId (void) const;

    /**
     * @brief Set PDC identifier
     * @param pdcId PDC identifier
     */
    void SetPdcId (uint16_t pdcId);

    /**
     * @brief Get PDC type
     * @return PDC type
     */
    PdcType GetPdcType (void) const;

    /**
     * @brief Get local fabric endpoint
     * @return Local fabric endpoint identifier
     */
    uint32_t GetLocalFep (void) const;

    /**
     * @brief Get remote fabric endpoint
     * @return Remote fabric endpoint identifier
     */
    uint32_t GetRemoteFep (void) const;

    /**
     * @brief Set local fabric endpoint
     * @param localFep Local fabric endpoint identifier
     */
    void SetLocalFep (uint32_t localFep);

    /**
     * @brief Set remote fabric endpoint
     * @param remoteFep Remote fabric endpoint identifier
     */
    void SetRemoteFep (uint32_t remoteFep);

    /**
     * @brief Set associated network device
     * @param device Pointer to network device
     */
    void SetNetDevice (Ptr<SoftUeNetDevice> device);

    /**
     * @brief Get associated network device
     * @return Pointer to network device
     */
    Ptr<SoftUeNetDevice> GetNetDevice (void) const;

    /**
     * @brief Set SES manager callback
     * @param sesManager Pointer to SES manager
     */
    void SetSesManager (Ptr<SesManager> sesManager);

    /**
     * @brief Get SES manager
     * @return Pointer to SES manager
     */
    Ptr<SesManager> GetSesManager (void) const;

    /**
     * @brief Send packet through PDC
     * @param packet Packet to send
     * @param som Start of message flag
     * @param eom End of message flag
     * @return true if packet was sent successfully
     */
    virtual bool SendPacket (Ptr<Packet> packet, bool som = false, bool eom = false) = 0;

    /**
     * @brief Handle received packet
     * @param packet Received packet
     * @param sourceFep Source fabric endpoint
     * @return true if packet was handled successfully
     */
    virtual bool HandleReceivedPacket (Ptr<Packet> packet, uint32_t sourceFep);

    /**
     * @brief Get PDC statistics
     * @return PDC statistics
     */
    PdcStatistics GetStatistics (void) const;

    /**
     * @brief Reset PDC statistics
     */
    void ResetStatistics (void);

    /**
     * @brief Enable/disable detailed logging
     * @param enable True to enable detailed logging
     */
    void SetDetailedLogging (bool enable);

    /**
     * @brief Check if PDC is active
     * @return true if PDC is active
     */
    bool IsActive (void) const;

    /**
     * @brief Activate PDC
     */
    void Activate (void);

    /**
     * @brief Deactivate PDC
     */
    void Deactivate (void);

    /**
     * @brief Get current configuration
     * @return PDC configuration
     */
    PdcConfig GetConfiguration (void) const;

    /**
     * @brief Update configuration
     * @param config New configuration
     * @return true if update was successful
     */
    bool UpdateConfiguration (const PdcConfig& config);

    // Traced callbacks for monitoring
    TracedCallback<Ptr<Packet>, uint16_t> m_packetTxTrace;    ///< Packet transmit trace
    TracedCallback<Ptr<Packet>, uint16_t> m_packetRxTrace;    ///< Packet receive trace
    TracedCallback<uint16_t, std::string> m_errorTrace;       ///< Error trace
    TracedCallback<Time, uint16_t> m_latencyTrace;            ///< Latency trace

protected:
    /**
     * @brief DoDispose method for cleanup
     */
    virtual void DoDispose (void) override;

    /**
     * @brief Create PDS header for packet
     * @param packet Packet to create header for
     * @param som Start of message flag
     * @param eom End of message flag
     * @return PDS header
     */
    PDSHeader CreatePdsHeader (Ptr<Packet> packet, bool som, bool eom) const;

    /**
     * @brief Parse PDS header from received packet
     * @param packet Packet with PDS header
     * @return Parsed PDS header
     */
    UETPDSHeader ParsePdsHeader (Ptr<Packet> packet) const;

    /**
     * @brief Update packet statistics
     * @param isSend True for sent packet, false for received packet
     * @param packet Processed packet
     */
    void UpdateStatistics (bool isSend, Ptr<Packet> packet);

    /**
     * @brief Log detailed information
     * @param function Function name
     * @param message Log message
     */
    void LogDetailed (const std::string& function, const std::string& message) const;

    /**
     * @brief Validate packet before processing
     * @param packet Packet to validate
     * @param isSend True for sending, false for receiving
     * @return true if packet is valid
     */
    virtual bool ValidatePacket (Ptr<Packet> packet, bool isSend) const;

    /**
     * @brief Validate and record received packet (no enqueue). Used by Ipdc to avoid double-queue.
     * @param packet Received packet
     * @param sourceFep Source FEP (unused; for API consistency)
     * @return true if valid and recorded
     */
    bool ValidateAndRecordReceivedPacket (Ptr<Packet> packet, uint32_t sourceFep);

    /**
     * @brief Handle PDC-specific error
     * @param error Error code
     * @param details Error details
     * @return true if error was handled
     */
    virtual bool HandleError (PdsErrorCode error, const std::string& details);

private:
    // Core attributes
    PdcConfig m_config;                        ///< PDC configuration
    PdcStatistics m_statistics;                ///< Performance statistics
    bool m_active;                             ///< PDC active status

    // Component associations
    Ptr<SoftUeNetDevice> m_netDevice;         ///< Associated network device
    Ptr<SesManager> m_sesManager;              ///< Associated SES manager

    // Internal state (ns-3 standardized queues)
    Ptr<DropTailQueue<Packet>> m_sendQueue;       ///< Outgoing packet queue (DropTailQueue)
    Ptr<DropTailQueue<Packet>> m_receiveQueue;    ///< Incoming packet queue (DropTailQueue)
    EventId m_processEventId;                  ///< Processing event ID
    Time m_processInterval;                    ///< Processing interval

    // Latency tracking
    std::unordered_map<uint64_t, Time> m_packetTimestamps; ///< Packet entry timestamps for latency measurement
    Time m_maxTimestampAge;                               ///< Maximum age for timestamp entries before cleanup
    uint32_t m_maxTimestampEntries;                       ///< Maximum number of timestamp entries before forced cleanup

    // Internal helper methods
    void ScheduleProcessing (void);
    void DoPeriodicProcessing (void);
    void ProcessSendQueue (void);
    void ProcessReceiveQueue (void);

    // Latency measurement helpers
    uint64_t GetPacketId (Ptr<Packet> packet);
    void RecordPacketEntry (Ptr<Packet> packet);
    void MeasureAndTraceLatency (Ptr<Packet> packet);

    // Memory management helpers
    void CleanupExpiredTimestamps (void);

    // Error handling helpers
    std::string GetErrorTypeString (PdsErrorCode error);
};

} // namespace ns3

#endif /* PDC_BASE_H */
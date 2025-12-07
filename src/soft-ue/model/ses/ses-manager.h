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
 * @file             ses-manager.h
 * @brief            Ultra Ethernet SES (Semantic Sub-layer) Manager
 * @author           softuegroup@gmail.com
 * @version          1.0.0
 * @date             2025-12-07
 * @copyright        Apache License Version 2.0
 *
 * @details
 * This file contains the SES Manager class which implements the Semantic
 * Sub-layer of the Ultra Ethernet protocol for ns-3 simulation.
 */

#ifndef SES_MANAGER_H
#define SES_MANAGER_H

#include <ns3/object.h>
#include <ns3/callback.h>
#include <ns3/event-id.h>
#include <ns3/traced-callback.h>
#include <queue>
#include "operation-metadata.h"
#include "msn-entry.h"
#include "../common/transport-layer.h"

namespace ns3 {

// Forward declarations
class PdsManager;
class SoftUeNetDevice;

/**
 * @class SesManager
 * @brief Ultra Ethernet SES (Semantic Sub-layer) Manager
 *
 * The SES Manager implements the semantic sub-layer of Ultra Ethernet,
 * responsible for endpoint addressing, authorization, message types,
 * and semantic header formats.
 */
class SesManager : public Object
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
    SesManager ();

    /**
     * @brief Destructor
     */
    virtual ~SesManager ();

    /**
     * @brief Set the associated PDS manager
     * @param pdsManager Pointer to PDS manager
     */
    void SetPdsManager (Ptr<PdsManager> pdsManager);

    /**
     * @brief Get the associated PDS manager
     * @return Pointer to PDS manager
     */
    Ptr<PdsManager> GetPdsManager (void) const;

    /**
     * @brief Set the associated network device
     * @param device Pointer to Soft-UE network device
     */
    void SetNetDevice (Ptr<SoftUeNetDevice> device);

    /**
     * @brief Get the associated network device
     * @return Pointer to Soft-UE network device
     */
    Ptr<SoftUeNetDevice> GetNetDevice (void) const;

    /**
     * @brief Initialize SES manager
     */
    void Initialize (void);

    /**
     * @brief Process send request from application layer
     * @param metadata Operation metadata
     * @return true if request was processed successfully
     */
    bool ProcessSendRequest (Ptr<ExtendedOperationMetadata> metadata);

    /**
     * @brief Process received request packet
     * @param request Received PDC to SES request
     * @return true if request was processed successfully
     */
    bool ProcessReceiveRequest (const PdcSesRequest& request);

    /**
     * @brief Process received response packet
     * @param response Received PDC to SES response
     * @return true if response was processed successfully
     */
    bool ProcessReceiveResponse (const PdcSesResponse& response);

    /**
     * @brief Send response to PDS layer
     * @param response SES to PDS response
     * @return true if response was sent successfully
     */
    bool SendResponseToPds (const SesPdsResponse& response);

    /**
     * @brief Get request queue size
     * @return Number of pending requests
     */
    size_t GetRequestQueueSize (void) const;

    /**
     * @brief Check if there are pending operations
     * @return true if there are pending operations
     */
    bool HasPendingOperations (void) const;

    /**
     * @brief Get MSN table reference
     * @return Reference to MSN table
     */
    Ptr<MsnTable> GetMsnTable (void) const;

    /**
     * @brief Set job ID validator callback
     * @param callback Callback function for job ID validation
     */
    void SetJobIdValidator (Callback<bool, uint64_t> callback);

    /**
     * @brief Set permission checker callback
     * @param callback Callback function for permission checking
     */
    void SetPermissionChecker (Callback<bool, uint32_t, uint64_t> callback);

    /**
     * @brief Set memory region validator callback
     * @param callback Callback function for memory region validation
     */
    void SetMemoryRegionValidator (Callback<bool, uint64_t> callback);

    /**
     * @brief Set packet received callback
     * @param callback Callback function for packet reception events
     */
    void SetPacketReceivedCallback (Callback<void, Ptr<ExtendedOperationMetadata>> callback);

    /**
     * @brief Set packet sent callback
     * @param callback Callback function for packet transmission events
     */
    void SetPacketSentCallback (Callback<void, Ptr<ExtendedOperationMetadata>> callback);

    /**
     * @brief Enable/disable detailed logging
     * @param enable True to enable detailed logging
     */
    void SetDetailedLogging (bool enable);

    /**
     * @brief Get statistics
     * @return Statistics string
     */
    std::string GetStatistics (void) const;

    /**
     * @brief Reset statistics
     */
    void ResetStatistics (void);

    // Traced callbacks for monitoring
    TracedCallback<Ptr<ExtendedOperationMetadata>> m_txTrace;        ///< Packet transmit trace
    TracedCallback<Ptr<ExtendedOperationMetadata>> m_rxTrace;        ///< Packet receive trace
    TracedCallback<std::string> m_errorTrace;                         ///< Error trace

protected:
    /**
     * @brief DoDispose method for cleanup
     */
    virtual void DoDispose (void) override;

private:
    Ptr<PdsManager> m_pdsManager;               ///< Associated PDS manager
    Ptr<SoftUeNetDevice> m_netDevice;           ///< Associated network device
    Ptr<MsnTable> m_msnTable;                   ///< MSN table for message tracking

    // Request queues
    std::queue<Ptr<ExtendedOperationMetadata>> m_requestQueue;   ///< Pending send requests
    std::queue<PdcSesRequest> m_recvRequestQueue;              ///< Received requests
    std::queue<PdcSesResponse> m_recvResponseQueue;             ///< Received responses

    // Configuration
    uint16_t m_maxMessageId;                     ///< Maximum message ID
    uint16_t m_currentMessageId;                 ///< Current message ID counter
    uint32_t m_maxMtu;                          ///< Maximum transmission unit
    bool m_detailedLogging;                      ///< Detailed logging flag

    // Callbacks
    Callback<bool, uint64_t> m_jobIdValidator;           ///< Job ID validator
    Callback<bool, uint32_t, uint64_t> m_permissionChecker; ///< Permission checker
    Callback<bool, uint64_t> m_memoryRegionValidator;     ///< Memory region validator
    Callback<void, Ptr<ExtendedOperationMetadata>> m_packetReceivedCallback; ///< Packet received callback
    Callback<void, Ptr<ExtendedOperationMetadata>> m_packetSentCallback;     ///< Packet sent callback

    // Timers and scheduling
    EventId m_processEventId;                   ///< Event ID for periodic processing
    Time m_processInterval;                     ///< Processing interval

    // Statistics
    mutable uint64_t m_totalSendRequests;       ///< Total send requests processed
    mutable uint64_t m_totalReceiveRequests;    ///< Total receive requests processed
    mutable uint64_t m_totalResponses;          ///< Total responses processed
    mutable uint64_t m_totalErrors;             ///< Total errors encountered
    mutable uint64_t m_totalPacketsGenerated;   ///< Total packets generated
    mutable uint64_t m_totalPacketsConsumed;     ///< Total packets consumed

    /**
     * @brief Initialize SES standard header
     * @param metadata Operation metadata
     * @return SES standard header
     */
    SesPdsRequest InitializeSesHeader (Ptr<ExtendedOperationMetadata> metadata);

    /**
     * @brief Generate next message ID
     * @param metadata Operation metadata
     * @return Next message ID
     */
    uint16_t GenerateMessageId (Ptr<ExtendedOperationMetadata> metadata);

    /**
     * @brief Calculate buffer offset
     * @param rkey Memory key
     * @param startAddr Start address
     * @return Buffer offset
     */
    uint64_t CalculateBufferOffset (uint64_t rkey, uint64_t startAddr);

    /**
     * @brief Parse received request into metadata
     * @param request Received request
     * @return Parsed operation metadata
     */
    Ptr<ExtendedOperationMetadata> ParseReceivedRequest (const PdcSesRequest& request);

    /**
     * @brief Decode RKEY to memory region
     * @param rkey Memory key
     * @return Memory region
     */
    MemoryRegion DecodeRkeyToMr (uint64_t rkey);

    /**
     * @brief Lookup memory region by key
     * @param key Memory key
     * @return Memory region
     */
    MemoryRegion LookupMrByKey (uint64_t key);

    /**
     * @brief Generate NACK response
     * @param errorCode Error code
     * @param originalRequest Original request
     * @return NACK response
     */
    SesPdsResponse GenerateNackResponse (ResponseReturnCode errorCode,
                                         const PdcSesRequest& originalRequest);

    // Validation methods
    bool ValidateVersion (uint8_t version);
    bool ValidateHeaderType (SESHeaderType type);
    bool ValidatePidOnFep (uint32_t pidOnFep, uint32_t jobId, bool relative);
    bool ValidateOperation (OpType opcode);
    bool ValidateDataLength (size_t expectedLength, size_t actualLength);
    bool ValidatePdcStatus (uint32_t pdcId, uint64_t psn);
    bool ValidateMemoryKey (uint64_t rkey, uint32_t messageId);
    bool ValidateMsn (uint64_t jobId, uint64_t psn, uint64_t expectedLength,
                     uint32_t pdcId, bool isFirstPacket, bool isLastPacket);
    bool ShouldSendAck (uint32_t messageId, bool deliveryComplete);

    /**
     * @brief Periodic processing
     */
    void DoPeriodicProcessing (void);

    /**
     * @brief Schedule periodic processing
     */
    void ScheduleProcessing (void);

    /**
     * @brief Log detailed information
     * @param function Function name
     * @param message Log message
     */
    void LogDetailed (const std::string& function, const std::string& message) const;

    /**
     * @brief Generate error trace
     * @param function Function name
     * @param error Error message
     */
    void LogError (const std::string& function, const std::string& error) const;
};

} // namespace ns3

#endif /* SES_MANAGER_H */
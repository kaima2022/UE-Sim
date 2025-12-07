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
 * @file             msn-entry.h
 * @brief            Ultra Ethernet MSN (Message Sequence Number) Management
 * @author           softuegroup@gmail.com
 * @version          1.0.0
 * @date             2025-12-07
 * @copyright        Apache License Version 2.0
 *
 * @details
 * This file contains the MSN entry and management classes for tracking
 * message sequence numbers in the Ultra Ethernet SES layer.
 */

#ifndef MSN_ENTRY_H
#define MSN_ENTRY_H

#include <ns3/object.h>
#include <ns3/timer.h>
#include <unordered_map>
#include <set>

namespace ns3 {

/**
 * @class MsnEntry
 * @brief Message Sequence Number entry for tracking message state
 *
 * This class represents an entry in the MSN table used by the SES layer
 * to track the state of received messages and ensure proper ordering.
 */
class MsnEntry : public Object
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
    MsnEntry ();

    /**
     * @brief Constructor with parameters
     * @param jobId Job identifier
     * @param pdcId Associated PDC identifier
     * @param expectedLength Expected total message length
     * @param creationTime Creation timestamp
     */
    MsnEntry (uint64_t jobId, uint32_t pdcId, uint64_t expectedLength, Time creationTime);

    /**
     * @brief Destructor
     */
    virtual ~MsnEntry ();

    /**
     * @brief Get job identifier
     * @return Job identifier
     */
    uint64_t GetJobId (void) const;

    /**
     * @brief Get PDC identifier
     * @return PDC identifier
     */
    uint32_t GetPdcId (void) const;

    /**
     * @brief Get last received packet sequence number
     * @return Last PSN
     */
    uint64_t GetLastPsn (void) const;

    /**
     * @brief Get expected message length
     * @return Expected length in bytes
     */
    uint64_t GetExpectedLength (void) const;

    /**
     * @brief Get creation time
     * @return Creation timestamp
     */
    Time GetCreationTime (void) const;

    /**
     * @brief Check if message is complete
     * @return true if all expected packets have been received
     */
    bool IsComplete (void) const;

    /**
     * @brief Get received packet count
     * @return Number of received packets
     */
    uint32_t GetReceivedPacketCount (void) const;

    /**
     * @brief Check if a PSN is expected next
     * @param psn Packet sequence number to check
     * @return true if PSN is the expected next sequence number
     */
    bool IsExpectedPsn (uint64_t psn) const;

    /**
     * @brief Update entry with received packet
     * @param psn Received packet sequence number
     * @param packetLength Length of received packet
     * @param isEom True if this is the end of message
     * @return true if update was successful
     */
    bool UpdateWithPacket (uint64_t psn, uint32_t packetLength, bool isEom);

    /**
     * @brief Check if entry has timed out
     * @param timeout Maximum allowed age
     * @return true if entry has timed out
     */
    bool HasTimedOut (Time timeout) const;

    /**
     * @brief Get missing PSNs in sequence
     * @return Set of missing PSNs
     */
    std::set<uint64_t> GetMissingPsns (void) const;

    /**
     * @brief Convert to string representation
     * @return String representation
     */
    std::string ToString (void) const;

private:
    uint64_t m_jobId;                 ///< Job identifier
    uint32_t m_pdcId;                  ///< Associated PDC identifier
    uint64_t m_lastPsn;                ///< Last received packet sequence number
    uint64_t m_expectedLength;         ///< Expected total message length
    Time m_creationTime;               ///< Creation timestamp
    uint32_t m_receivedPacketCount;    ///< Number of received packets
    uint64_t m_receivedLength;         ///< Total bytes received
    bool m_complete;                   ///< Message completion flag
    std::set<uint64_t> m_receivedPsns; ///< Set of received PSNs
};

/**
 * @class MsnTable
 * @brief MSN (Message Sequence Number) table manager
 *
 * This class manages the MSN table used by the SES layer to track
 * message ordering and ensure reliable delivery.
 */
class MsnTable : public Object
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
    MsnTable ();

    /**
     * @brief Destructor
     */
    virtual ~MsnTable ();

    /**
     * @brief Add a new MSN entry
     * @param jobId Job identifier
     * @param pdcId Associated PDC identifier
     * @param expectedLength Expected message length
     * @return true if entry was added successfully
     */
    bool AddEntry (uint64_t jobId, uint32_t pdcId, uint64_t expectedLength);

    /**
     * @brief Find MSN entry by job ID
     * @param jobId Job identifier to search for
     * @return Pointer to MSN entry, or nullptr if not found
     */
    Ptr<MsnEntry> FindEntry (uint64_t jobId) const;

    /**
     * @brief Remove MSN entry
     * @param jobId Job identifier to remove
     * @return true if entry was removed
     */
    bool RemoveEntry (uint64_t jobId);

    /**
     * @brief Update entry with received packet
     * @param jobId Job identifier
     * @param psn Packet sequence number
     * @param packetLength Packet length
     * @param isEom True if end of message
     * @param isFirstPacket True if first packet of message
     * @return true if update was successful
     */
    bool UpdateEntry (uint64_t jobId, uint64_t psn, uint32_t packetLength, bool isEom, bool isFirstPacket);

    /**
     * @brief Validate PSN for a job
     * @param jobId Job identifier
     * @param psn Packet sequence number to validate
     * @param expectedLength Expected message length
     * @param pdcId Associated PDC identifier
     * @param isFirstPacket True if first packet
     * @param isLastPacket True if last packet
     * @return true if PSN is valid for this job
     */
    bool ValidatePsn (uint64_t jobId, uint64_t psn, uint64_t expectedLength,
                     uint32_t pdcId, bool isFirstPacket, bool isLastPacket);

    /**
     * @brief Cleanup timed out entries
     * @param timeout Maximum age for entries
     * @return Number of entries cleaned up
     */
    uint32_t CleanupTimedOutEntries (Time timeout);

    /**
     * @brief Get table size
     * @return Number of entries in the table
     */
    size_t GetSize (void) const;

    /**
     * @brief Get all job IDs in the table
     * @return Set of job IDs
     */
    std::set<uint64_t> GetAllJobIds (void) const;

    /**
     * @brief Clear all entries
     */
    void Clear (void);

    /**
     * @brief Get statistics about the MSN table
     * @return Statistics string
     */
    std::string GetStatistics (void) const;

    /**
     * @brief Set maximum table size
     * @param maxSize Maximum number of entries
     */
    void SetMaxSize (size_t maxSize);

    /**
     * @brief Get maximum table size
     * @return Maximum size
     */
    size_t GetMaxSize (void) const;

    /**
     * @brief Enable automatic cleanup
     * @param interval Cleanup interval
     */
    void EnableAutoCleanup (Time interval);

    /**
     * @brief Disable automatic cleanup
     */
    void DisableAutoCleanup (void);

private:
    /// Type alias for MSN table map
    using MsnMap = std::unordered_map<uint64_t, Ptr<MsnEntry>>;

    MsnMap m_msnTable;                ///< MSN table storage
    size_t m_maxSize;                  ///< Maximum table size
    bool m_autoCleanupEnabled;         ///< Auto cleanup flag
    Timer m_cleanupTimer;              ///< Cleanup timer
    Time m_defaultTimeout;             ///< Default entry timeout

    // Statistics
    mutable uint64_t m_totalLookups;   ///< Total lookup operations
    mutable uint64_t m_totalUpdates;   ///< Total update operations
    mutable uint64_t m_totalRemovals;  ///< Total removal operations

    /**
     * @brief Perform automatic cleanup
     */
    void DoAutoCleanup (void);

    /**
     * @brief Ensure table size doesn't exceed maximum
     * @return Number of entries removed
     */
    uint32_t EnforceMaxSize (void);
};

} // namespace ns3

#endif /* MSN_ENTRY_H */
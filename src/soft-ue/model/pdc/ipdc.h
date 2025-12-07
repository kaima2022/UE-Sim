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
 * @file             ipdc.h
 * @brief            IPDC (Unreliable PDC) Implementation
 * @author           softuegroup@gmail.com
 * @version          1.0.0
 * @date             2025-12-07
 * @copyright        Apache License Version 2.0
 *
 * @details
 * This file contains the IPDC (Unreliable Packet Delivery Context) implementation
 * for the Ultra Ethernet protocol stack. IPDC provides best-effort packet delivery
 * without reliability guarantees.
 */

#ifndef IPDC_H
#define IPDC_H

#include "pdc-base.h"
#include "ns3/object.h"
#include "ns3/callback.h"
#include "ns3/timer.h"
#include "ns3/event-id.h"
#include "ns3/traced-callback.h"
#include "ns3/packet.h"
#include "ns3/mac48-address.h"
#include "../common/transport-layer.h"

namespace ns3 {

// Forward declarations
class SoftUeNetDevice;

/**
 * @struct IpdcConfig
 * @brief IPDC-specific configuration parameters
 */
struct IpdcConfig : public PdcConfig
{
    uint32_t maxSendQueueSize;                 ///< Maximum send queue size
    uint32_t maxReceiveQueueSize;              ///< Maximum receive queue size
    uint16_t sendWindow;                       ///< Send window size
    uint16_t receiveWindow;                    ///< Receive window size
    bool enableFlowControl;                    ///< Enable flow control
    Time packetLifetime;                       ///< Packet lifetime timeout

    IpdcConfig ()
        : PdcConfig (), maxSendQueueSize (1024), maxReceiveQueueSize (1024),
          sendWindow (64), receiveWindow (64), enableFlowControl (true),
          packetLifetime (Seconds (10))
    {
        type = PdcType::IPDC;
    }
};

/**
 * @struct IpdcStatistics
 * @brief IPDC-specific performance statistics
 */
struct IpdcStatistics : public PdcStatistics
{
    uint64_t packetsDropped;                   ///< Packets dropped due to queue overflow
    uint64_t packetsExpired;                   ///< Packets that expired
    uint64_t flowControlEvents;                ///< Flow control events
    uint32_t currentSendQueueSize;             ///< Current send queue size
    uint32_t currentReceiveQueueSize;          ///< Current receive queue size
    Time averageQueueTime;                     ///< Average time in send queue

    IpdcStatistics ()
        : PdcStatistics (), packetsDropped (0), packetsExpired (0),
          flowControlEvents (0), currentSendQueueSize (0),
          currentReceiveQueueSize (0), averageQueueTime (Seconds (0))
    {}
};

/**
 * @struct QueuedPacket
 * @brief Information about a queued packet
 */
struct QueuedPacket
{
    Ptr<Packet> packet;                        ///< Packet data
    Time timestamp;                            ///< Time when packet was queued
    bool som;                                  ///< Start of message flag
    bool eom;                                  ///< End of message flag
    uint32_t sequenceNumber;                  ///< Sequence number

    QueuedPacket ()
        : packet (nullptr), timestamp (Seconds (0)),
          som (false), eom (false), sequenceNumber (0)
    {}

    QueuedPacket (Ptr<Packet> pkt, bool s, bool e, uint32_t seq)
        : packet (pkt), timestamp (Simulator::Now ()),
          som (s), eom (e), sequenceNumber (seq)
    {}
};

/**
 * @class Ipdc
 * @brief IPDC (Unreliable Packet Delivery Context) implementation
 *
 * The IPDC class implements best-effort packet delivery without reliability
 * guarantees. It provides simple send/receive functionality with optional
 * flow control and queue management.
 */
class Ipdc : public PdcBase
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
     * @brief Constructor
     */
    Ipdc ();

    /**
     * @brief Destructor
     */
    virtual ~Ipdc ();

    /**
     * @brief Initialize IPDC with configuration
     * @param config IPDC configuration
     * @return true if initialization was successful
     */
    virtual bool Initialize (const PdcConfig& config) override;

    /**
     * @brief Send packet through IPDC
     * @param packet Packet to send
     * @param som Start of message flag
     * @param eom End of message flag
     * @return true if packet was queued for sending
     */
    virtual bool SendPacket (Ptr<Packet> packet, bool som = false, bool eom = false) override;

    /**
     * @brief Handle received packet
     * @param packet Received packet
     * @param sourceFep Source fabric endpoint
     * @return true if packet was handled successfully
     */
    virtual bool HandleReceivedPacket (Ptr<Packet> packet, uint32_t sourceFep) override;

    /**
     * @brief Get IPDC-specific statistics
     * @return IPDC statistics
     */
    IpdcStatistics GetIpdcStatistics (void) const;

    /**
     * @brief Reset IPDC statistics
     */
    void ResetIpdcStatistics (void);

    /**
     * @brief Get current send queue size
     * @return Number of packets in send queue
     */
    uint32_t GetSendQueueSize (void) const;

    /**
     * @brief Get current receive queue size
     * @return Number of packets in receive queue
     */
    uint32_t GetReceiveQueueSize (void) const;

    /**
     * @brief Get IPDC configuration
     * @return IPDC configuration
     */
    IpdcConfig GetIpdcConfiguration (void) const;

    /**
     * @brief Update IPDC configuration
     * @param config New IPDC configuration
     * @return true if update was successful
     */
    bool UpdateIpdcConfiguration (const IpdcConfig& config);

    /**
     * @brief Check if flow control is enabled
     * @return true if flow control is enabled
     */
    bool IsFlowControlEnabled (void) const;

    /**
     * @brief Enable/disable flow control
     * @param enabled true to enable flow control
     */
    void SetFlowControlEnabled (bool enabled);

    /**
     * @brief Get packet lifetime
     * @return Packet lifetime
     */
    Time GetPacketLifetime (void) const;

    /**
     * @brief Set packet lifetime
     * @param lifetime Packet lifetime
     */
    void SetPacketLifetime (Time lifetime);

    /**
     * @brief Send packet directly (bypass queue)
     * @param packet Packet to send
     * @param som Start of message flag
     * @param eom End of message flag
     * @return true if packet was sent successfully
     */
    bool SendPacketDirect (Ptr<Packet> packet, bool som = false, bool eom = false);

    /**
     * @brief Clear send queue
     * @return Number of packets cleared
     */
    uint32_t ClearSendQueue (void);

    /**
     * @brief Clear receive queue
     * @return Number of packets cleared
     */
    uint32_t ClearReceiveQueue (void);

    // IPDC-specific traced callbacks
    TracedCallback<uint32_t, uint32_t> m_queueTrace;           ///< Queue size trace
    TracedCallback<Ptr<Packet>, std::string> m_packetDropTrace; ///< Packet drop trace

protected:
    /**
     * @brief DoDispose method for cleanup
     */
    virtual void DoDispose (void) override;

    /**
     * @brief Validate packet for IPDC
     * @param packet Packet to validate
     * @param isSend True for sending, false for receiving
     * @return true if packet is valid for IPDC
     */
    virtual bool ValidatePacket (Ptr<Packet> packet, bool isSend) const override;

    /**
     * @brief Handle IPDC-specific error
     * @param error Error code
     * @param details Error details
     * @return true if error was handled
     */
    virtual bool HandleError (PdsErrorCode error, const std::string& details) override;

private:
    // IPDC-specific configuration and state
    IpdcConfig m_ipdcConfig;                   ///< IPDC-specific configuration
    IpdcStatistics m_ipdcStatistics;           ///< IPDC-specific statistics

    // Packet queues and management
    std::queue<QueuedPacket> m_sendQueue;       ///< Outgoing packet queue
    std::queue<QueuedPacket> m_receiveQueue;    ///< Incoming packet queue
    uint32_t m_nextSequenceNumber;              ///< Next sequence number

    // Flow control
    uint16_t m_sendCredits;                     ///< Available send credits
    uint16_t m_receiveCredits;                  ///< Available receive credits
    bool m_flowControlActive;                   ///< Flow control status

    // Queue management
    EventId m_queueCleanupEventId;              ///< Event ID for queue cleanup
    Time m_queueCleanupInterval;                ///< Queue cleanup interval

    // Internal helper methods
    bool EnqueuePacket (std::queue<QueuedPacket>& queue, Ptr<Packet> packet,
                       bool som, bool eom, uint32_t maxSize);
    bool DequeuePacket (std::queue<QueuedPacket>& queue, QueuedPacket& qp);
    void ProcessSendQueue (void);
    void ProcessReceiveQueue (void);
    void UpdateFlowControl (void);
    void CleanupExpiredPackets (void);
    void ScheduleQueueCleanup (void);
    bool CheckFlowControl (void) const;
    void UpdateQueueStatistics (void);
    bool TransmitPacket (const QueuedPacket& qp);
    void HandlePacketDelivery (Ptr<Packet> packet, bool success);
    uint32_t GenerateSequenceNumber (void);

    // Network device helper
    Address ConvertFepToAddress (uint32_t fep);
};

} // namespace ns3

#endif /* IPDC_H */
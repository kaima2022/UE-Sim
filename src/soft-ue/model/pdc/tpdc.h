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
 * @file             tpdc.h
 * @brief            TPDC (Reliable PDC) Implementation
 * @author           softuegroup@gmail.com
 * @version          1.0.0
 * @date             2025-12-07
 * @copyright        Apache License Version 2.0
 *
 * @details
 * This file contains the TPDC (Reliable Packet Delivery Context) implementation
 * for the Ultra Ethernet protocol stack. TPDC provides reliable packet delivery
 * with acknowledgment, retransmission, and flow control mechanisms.
 */

#ifndef TPDC_H
#define TPDC_H

#include "pdc-base.h"
#include "rto-timer/rto-timer.h"
#include "ns3/object.h"
#include "ns3/callback.h"
#include "ns3/timer.h"
#include "ns3/event-id.h"
#include "ns3/traced-callback.h"
#include "ns3/packet.h"
#include "../common/transport-layer.h"
#include <unordered_map>
#include <queue>

namespace ns3 {

// Forward declarations
class SoftUeNetDevice;

/**
 * @struct TpdcConfig
 * @brief TPDC-specific configuration parameters
 */
struct TpdcConfig : public PdcConfig
{
    uint32_t maxSendBufferSize;                ///< Maximum send buffer size
    uint32_t maxReceiveBufferSize;             ///< Maximum receive buffer size
    uint16_t sendWindowSize;                   ///< Send window size
    uint16_t receiveWindowSize;                ///< Receive window size
    uint8_t maxRetransmissions;                ///< Maximum retransmission attempts
    Time initialRto;                          ///< Initial retransmission timeout
    Time maxRto;                              ///< Maximum retransmission timeout
    double rtoBackoffMultiplier;               ///< RTO backoff multiplier
    bool enableCumulativeAcks;                ///< Enable cumulative acknowledgments
    Time ackTimeout;                          ///< Acknowledgment timeout
    uint16_t maxPacketsPerAck;                ///< Maximum packets per ACK

    TpdcConfig ()
        : PdcConfig (), maxSendBufferSize (2048), maxReceiveBufferSize (2048),
          sendWindowSize (128), receiveWindowSize (128), maxRetransmissions (5),
          initialRto (MilliSeconds (200)), maxRto (Seconds (2)), rtoBackoffMultiplier (2.0),
          enableCumulativeAcks (true), ackTimeout (MilliSeconds (100)), maxPacketsPerAck (8)
    {
        type = PdcType::TPDC;
    }
};

/**
 * @struct TpdcStatistics
 * @brief TPDC-specific performance statistics
 */
struct TpdcStatistics : public PdcStatistics
{
    uint64_t acknowledgmentsSent;               ///< Number of ACKs sent
    uint64_t acknowledgmentsReceived;           ///< Number of ACKs received
    uint64_t duplicateAcksReceived;             ///< Number of duplicate ACKs
    uint64_t outOfOrderPackets;                 ///< Number of out-of-order packets
    uint64_t cumulativeAcksSent;                ///< Number of cumulative ACKs
    uint64_t retransmissionTimeouts;            ///< Number of RTO timeouts
    uint64_t fastRetransmissions;               ///< Number of fast retransmissions
    uint64_t sendBufferOverflows;               ///< Send buffer overflow events
    uint64_t receiveBufferOverflows;            ///< Receive buffer overflow events
    uint32_t currentSendBufferSize;             ///< Current send buffer size
    uint32_t currentReceiveBufferSize;          ///< Current receive buffer size
    double averageRto;                         ///< Average RTO value
    Time averageRoundTripTime;                 ///< Average round-trip time

    TpdcStatistics ()
        : PdcStatistics (), acknowledgmentsSent (0), acknowledgmentsReceived (0),
          duplicateAcksReceived (0), outOfOrderPackets (0), cumulativeAcksSent (0),
          retransmissionTimeouts (0), fastRetransmissions (0),
          sendBufferOverflows (0), receiveBufferOverflows (0),
          currentSendBufferSize (0), currentReceiveBufferSize (0),
          averageRto (0.0), averageRoundTripTime (Seconds (0))
    {}
};

/**
 * @struct BufferedPacket
 * @brief Information about a buffered packet
 */
struct BufferedPacket
{
    Ptr<Packet> packet;                        ///< Packet data
    Time timestamp;                            ///< Time when packet was sent/buffered
    Time lastRetransmission;                   ///< Time of last retransmission
    uint8_t retransmissionCount;               ///< Number of retransmissions
    bool acknowledged;                         ///< Packet acknowledged status
    bool som;                                  ///< Start of message flag
    bool eom;                                  ///< End of message flag
    uint32_t sequenceNumber;                  ///< Sequence number
    Ptr<RtoTimer> rtoTimer;                    ///< RTO timer for this packet

    BufferedPacket ()
        : packet (nullptr), timestamp (Seconds (0)), lastRetransmission (Seconds (0)),
          retransmissionCount (0), acknowledged (false), som (false), eom (false),
          sequenceNumber (0), rtoTimer (nullptr)
    {}

    BufferedPacket (Ptr<Packet> pkt, bool s, bool e, uint32_t seq)
        : packet (pkt), timestamp (Simulator::Now ()), lastRetransmission (Seconds (0)),
          retransmissionCount (0), acknowledged (false), som (s), eom (e),
          sequenceNumber (seq), rtoTimer (nullptr)
    {}
};

/**
 * @struct Acknowledgment
 * @brief Acknowledgment information
 */
struct Acknowledgment
{
    uint32_t ackSequence;                      ///< Acknowledgment sequence number
    uint32_t receiveWindow;                    ///< Current receive window
    std::vector<uint32_t> nackList;            ///< List of NACKed sequence numbers
    bool cumulative;                           ///< Cumulative ACK flag
    Time timestamp;                            ///< Acknowledgment timestamp

    Acknowledgment ()
        : ackSequence (0), receiveWindow (0), cumulative (false), timestamp (Seconds (0))
    {}

    Acknowledgment (uint32_t ack, uint16_t window, bool cum = false)
        : ackSequence (ack), receiveWindow (window), cumulative (cum), timestamp (Simulator::Now ())
    {}
};

/**
 * @class Tpdc
 * @brief TPDC (Reliable Packet Delivery Context) implementation
 *
 * The TPDC class implements reliable packet delivery with acknowledgment,
 * retransmission, and flow control mechanisms. It provides ordered, guaranteed
 * delivery of packets between endpoints.
 */
class Tpdc : public PdcBase
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
    Tpdc ();

    /**
     * @brief Destructor
     */
    virtual ~Tpdc ();

    /**
     * @brief Initialize TPDC with configuration
     * @param config TPDC configuration
     * @return true if initialization was successful
     */
    virtual bool Initialize (const PdcConfig& config) override;

    /**
     * @brief Send packet through TPDC
     * @param packet Packet to send
     * @param som Start of message flag
     * @param eom End of message flag
     * @return true if packet was buffered for sending
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
     * @brief Get TPDC-specific statistics
     * @return TPDC statistics
     */
    TpdcStatistics GetTpdcStatistics (void) const;

    /**
     * @brief Reset TPDC statistics
     */
    void ResetTpdcStatistics (void);

    /**
     * @brief Get current send buffer size
     * @return Number of packets in send buffer
     */
    uint32_t GetSendBufferSize (void) const;

    /**
     * @brief Get current receive buffer size
     * @return Number of packets in receive buffer
     */
    uint32_t GetReceiveBufferSize (void) const;

    /**
     * @brief Get TPDC configuration
     * @return TPDC configuration
     */
    TpdcConfig GetTpdcConfiguration (void) const;

    /**
     * @brief Update TPDC configuration
     * @param config New TPDC configuration
     * @return true if update was successful
     */
    bool UpdateTpdcConfiguration (const TpdcConfig& config);

    /**
     * @brief Send acknowledgment
     * @param ack Acknowledgment to send
     * @return true if acknowledgment was sent successfully
     */
    bool SendAcknowledgment (const Acknowledgment& ack);

    /**
     * @brief Process received acknowledgment
     * @param ack Received acknowledgment
     * @return true if acknowledgment was processed successfully
     */
    bool ProcessReceivedAcknowledgment (const Acknowledgment& ack);

    /**
     * @brief Force retransmission of unacknowledged packets
     * @return Number of packets retransmitted
     */
    uint32_t ForceRetransmission (void);

    /**
     * @brief Clear send buffer
     * @return Number of packets cleared
     */
    uint32_t ClearSendBuffer (void);

    /**
     * @brief Clear receive buffer
     * @return Number of packets cleared
     */
    uint32_t ClearReceiveBuffer (void);

    /**
     * @brief Get current RTT estimate
     * @return Current RTT estimate
     */
    Time GetCurrentRtt (void) const;

    /**
     * @brief Get current RTO value
     * @return Current RTO value
     */
    Time GetCurrentRto (void) const;

    // TPDC-specific traced callbacks
    TracedCallback<uint32_t, uint32_t> m_bufferTrace;           ///< Buffer size trace
    TracedCallback<const Acknowledgment&> m_ackTrace;           ///< Acknowledgment trace
    TracedCallback<uint32_t, uint8_t> m_retransmissionTrace;    ///< Retransmission trace
    TracedCallback<Time, Time> m_rttTrace;                     ///< RTT measurement trace

protected:
    /**
     * @brief DoDispose method for cleanup
     */
    virtual void DoDispose (void) override;

    /**
     * @brief Validate packet for TPDC
     * @param packet Packet to validate
     * @param isSend True for sending, false for receiving
     * @return true if packet is valid for TPDC
     */
    virtual bool ValidatePacket (Ptr<Packet> packet, bool isSend) const override;

    /**
     * @brief Handle TPDC-specific error
     * @param error Error code
     * @param details Error details
     * @return true if error was handled
     */
    virtual bool HandleError (PdsErrorCode error, const std::string& details) override;

private:
    // TPDC-specific configuration and state
    TpdcConfig m_tpdcConfig;                   ///< TPDC-specific configuration
    TpdcStatistics m_tpdcStatistics;           ///< TPDC-specific statistics

    // Sequence number management
    uint32_t m_nextSendSequence;               ///< Next sequence number to send
    uint32_t m_nextReceiveSequence;            ///< Next expected receive sequence
    uint32_t m_sendWindowBase;                 ///< Base of send window
    uint32_t m_receiveWindowBase;              ///< Base of receive window

    // Packet buffers
    std::unordered_map<uint32_t, BufferedPacket> m_sendBuffer;  ///< Send buffer (seq -> packet)
    std::unordered_map<uint32_t, BufferedPacket> m_receiveBuffer; ///< Receive buffer (seq -> packet)
    std::queue<Ptr<Packet>> m_sendQueue;       ///< Outgoing packet queue

    // ACK management
    std::queue<Acknowledgment> m_ackQueue;     ///< Acknowledgment queue
    EventId m_ackEventId;                      ///< ACK transmission event ID
    Time m_ackInterval;                        ///< ACK transmission interval

    // RTO and RTT management
    Time m_currentRto;                         ///< Current retransmission timeout
    Time m_currentRtt;                         ///< Current round-trip time estimate
    Time m_rttVariance;                         ///< RTT variance for calculation

    // Internal helper methods
    bool BufferPacketForSending (Ptr<Packet> packet, bool som, bool eom);
    bool BufferPacketForReceiving (Ptr<Packet> packet, uint32_t seq);
    void ProcessSendQueue (void);
    void ProcessReceiveQueue (void);
    void RetransmitPacket (uint32_t sequenceNumber);
    bool TransmitPacket (const BufferedPacket& bp);
    void ScheduleAcknowledgment (void);
    void SendPendingAcknowledgments (void);
    void UpdateSendWindow (void);
    void UpdateReceiveWindow (void);
    void UpdateRtoEstimate (Time measuredRtt);
    void HandleRtoTimeout (uint32_t sequenceNumber);
    void HandleAckTimeout (void);
    bool IsInSendWindow (uint32_t sequence) const;
    bool IsInReceiveWindow (uint32_t sequence) const;
    void CleanupAcknowledgedPackets (void);
    Acknowledgment CreateAcknowledgment (void);
    uint32_t GenerateSequenceNumber (void);
    void UpdateStatistics (void);
};

} // namespace ns3

#endif /* TPDC_H */
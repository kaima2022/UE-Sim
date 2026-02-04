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
 * @file             soft-ue-net-device.h
 * @brief            Soft-UE Network Device Implementation
 * @author           softuegroup@gmail.com
 * @version          1.0.0
 * @date             2025-12-07
 * @copyright        Apache License Version 2.0
 *
 * @details
 * This file contains the Soft-UE network device that integrates the Ultra Ethernet
 * protocol stack (SES/PDS/PDC layers) with ns-3's network device framework.
 */

#ifndef SOFT_UE_NET_DEVICE_H
#define SOFT_UE_NET_DEVICE_H

#include "ns3/net-device.h"
#include "ns3/mac48-address.h"
#include "ns3/callback.h"
#include "ns3/packet.h"
#include "ns3/node.h"
#include "ns3/traced-callback.h"
#include "ns3/nstime.h"
#include "ns3/event-id.h"
#include "ns3/random-variable-stream.h"
#include "ns3/ipv4-address.h"
#include "ns3/ipv6-address.h"
#include "ns3/drop-tail-queue.h"
#include <map>
#include <vector>
#include "../common/transport-layer.h"

namespace ns3 {

// Forward declarations to avoid circular dependencies
class SesManager;
class PdsManager;
class PdcBase;

// Forward declarations
class SoftUeChannel;
class UniformRandomVariable;

/**
 * @struct SoftUeStats
 * @brief Soft-Ue network device statistics
 */
struct SoftUeStats
{
    uint64_t totalBytesReceived;                 ///< Total bytes received
    uint64_t totalBytesTransmitted;              ///< Total bytes transmitted
    uint64_t totalPacketsReceived;               ///< Total packets received
    uint64_t totalPacketsTransmitted;            ///< Total packets transmitted
    uint64_t droppedPackets;                     ///< Number of dropped packets
    uint64_t activePdcCount;                     ///< Current active PDC count
    Time lastActivity;                           ///< Last activity timestamp
    double averageLatency;                       ///< Average packet latency (ms)
    double throughput;                           ///< Current throughput (Mbps)

    SoftUeStats ()
        : totalBytesReceived (0), totalBytesTransmitted (0),
          totalPacketsReceived (0), totalPacketsTransmitted (0),
          droppedPackets (0), activePdcCount (0),
          lastActivity (Seconds (0)), averageLatency (0.0), throughput (0.0)
    {}
};

/**
 * @struct SoftUeConfig
 * @brief Soft-Ue network device configuration
 */
struct SoftUeConfig
{
    Mac48Address address;                        ///< MAC address
    uint32_t localFep;                           ///< Local fabric endpoint ID
    uint32_t maxPdcCount;                        ///< Maximum PDC count per type
    Time processingInterval;                     ///< Packet processing interval
    uint16_t maxPacketSize;                      ///< Maximum packet size
    bool enableDetailedLogging;                  ///< Enable detailed logging
    bool enableStatistics;                       ///< Enable statistics collection
    Time statsUpdateInterval;                    ///< Statistics update interval

    SoftUeConfig ()
        : address (Mac48Address::Allocate ()), localFep (1), maxPdcCount (512),
          processingInterval (MilliSeconds (1)), maxPacketSize (1500),
          enableDetailedLogging (false), enableStatistics (true),
          statsUpdateInterval (MilliSeconds (100))
    {}
};

/**
 * @class SoftUeNetDevice
 * @brief Soft-Ue Ultra Ethernet Network Device
 *
 * The SoftUeNetDevice implements a complete Ultra Ethernet protocol stack
 * with SES (Semantic Sub-layer), PDS (Packet Delivery Sub-layer), and PDC
 * (Packet Delivery Context) layers integrated into ns-3's network device framework.
 */
class SoftUeNetDevice : public NetDevice
{
public:
    /**
     * @brief Get the type ID for this class
     * @return TypeId
     */
    static TypeId GetTypeId (void);

    /**
     * @brief Constructor
     */
    SoftUeNetDevice ();

    /**
     * @brief Destructor
     */
    virtual ~SoftUeNetDevice ();

    /**
     * @brief Initialize the Soft-Ue network device
     * @param config Device configuration
     * @return true if initialization was successful
     */
    bool Initialize (const SoftUeConfig& config = SoftUeConfig ());

    // NetDevice interface implementation
    virtual void SetIfIndex (const uint32_t index) override;
    virtual uint32_t GetIfIndex (void) const override;
    virtual Ptr<Channel> GetChannel (void) const override;
    virtual void SetAddress (Address address) override;
    virtual Address GetAddress (void) const override;
    virtual bool SetMtu (const uint16_t mtu) override;
    virtual uint16_t GetMtu (void) const override;
    virtual bool IsLinkUp (void) const override;
    virtual void AddLinkChangeCallback (Callback<void> callback) override;
    virtual bool IsBroadcast (void) const override;
    virtual Address GetBroadcast (void) const override;
    virtual bool IsMulticast (void) const override;
    virtual Address GetMulticast (Ipv4Address multicastGroup) const override;
    virtual Address GetMulticast (Ipv6Address addr) const override;
    virtual bool IsPointToPoint (void) const override;
    virtual bool IsBridge (void) const override;
    virtual bool Send (Ptr<Packet> packet, const Address& dest, uint16_t protocolNumber) override;
    virtual bool SendFrom (Ptr<Packet> packet, const Address& source, const Address& dest,
                         uint16_t protocolNumber) override;
    virtual Ptr<Node> GetNode (void) const override;
    virtual void SetNode (Ptr<Node> node) override;
    virtual bool NeedsArp (void) const override;
    virtual void SetReceiveCallback (ReceiveCallback cb) override;
    virtual void SetPromiscReceiveCallback (PromiscReceiveCallback cb) override;
    virtual bool SupportsSendFrom () const override;
    virtual bool IsPromisc (void) const;
    virtual void SetPromisc (bool promisc);

    // Soft-Ue specific methods

    /**
     * @brief Set associated Soft-Ue channel
     * @param channel Pointer to Soft-Ue channel
     */
    void SetChannel (Ptr<SoftUeChannel> channel);

    /**
     * @brief Get SES manager
     * @return Pointer to SES manager
     */
    Ptr<SesManager> GetSesManager (void) const;

    /**
     * @brief Get PDS manager
     * @return Pointer to PDS manager
     */
    Ptr<PdsManager> GetPdsManager (void) const;

    /**
     * @brief Allocate a new PDC
     * @param destFep Destination fabric endpoint
     * @param tc Traffic control category
     * @param dm Delivery mode
     * @param nextHdr Next header type
     * @return PDC identifier, or 0 if allocation failed
     */
    uint16_t AllocatePdc (uint32_t destFep, uint8_t tc, uint8_t dm, PDSNextHeader nextHdr);

    /**
     * @brief Release a PDC
     * @param pdcId PDC identifier to release
     * @return true if release was successful
     */
    bool ReleasePdc (uint16_t pdcId);

    /**
     * @brief Send packet through specific PDC
     * @param pdcId PDC identifier
     * @param packet Packet to send
     * @param som Start of message flag
     * @param eom End of message flag
     * @return true if packet was sent successfully
     */
    bool SendPacketThroughPdc (uint16_t pdcId, Ptr<Packet> packet, bool som = false, bool eom = false);

    /**
     * @brief Handle received packet from channel
     * @param packet Received packet
     * @param sourceFep Source fabric endpoint
     * @param destFep Destination fabric endpoint
     */
    void ReceivePacket (Ptr<Packet> packet, uint32_t sourceFep, uint32_t destFep);

    /**
     * @brief Get device statistics
     * @return Device statistics
     */
    SoftUeStats GetStatistics (void) const;

    /**
     * @brief Reset device statistics
     */
    void ResetStatistics (void);

    /**
     * @brief Get current configuration
     * @return Device configuration
     */
    SoftUeConfig GetConfiguration (void) const;

    /**
     * @brief Update device configuration
     * @param config New configuration
     * @return true if update was successful
     */
    bool UpdateConfiguration (const SoftUeConfig& config);

    /**
     * @brief Get number of active PDCs
     * @return Active PDC count
     */
    uint32_t GetActivePdcCount (void) const;

    /**
     * @brief Enable/disable link
     * @param up True to enable link, false to disable
     */
    void SetLinkUp (bool up);

    // Attribute getter/setter methods
    bool GetEnableStatistics (void) const;
    void SetEnableStatistics (bool enable);
    uint32_t GetMaxPdcCount (void) const;
    void SetMaxPdcCount (uint32_t count);
    Time GetProcessingInterval (void) const;
    void SetProcessingInterval (Time interval);
    uint16_t GetMaxPacketSize (void) const;
    void SetMaxPacketSize (uint16_t size);

    // Public utility methods
    Address CreateAddressFromFep (uint32_t fep) const;
    uint32_t GetLocalFep (void) const;
    /**
     * @brief Transmit packet directly to channel (used by PDC layer; bypasses PDS Manager)
     * @param packet Packet to transmit (must already include PDS header if needed)
     * @param srcFep Source FEP
     * @param destFep Destination FEP
     * @return true if transmitted successfully
     */
    bool TransmitToChannel (Ptr<Packet> packet, uint32_t srcFep, uint32_t destFep);

    /**
     * @brief Deliver received packet to upper layer (called by PDS Manager after ProcessReceivedPacket)
     * @param packet Payload packet (PDS header already removed)
     */
    void DeliverReceivedPacket (Ptr<Packet> packet);

    // Traced callbacks
    TracedCallback<Ptr<Packet>, const Address&> m_macTxTrace;        ///< MAC transmit trace
    TracedCallback<Ptr<Packet>, const Address&> m_macRxTrace;        ///< MAC receive trace
    TracedCallback<Ptr<Packet>, uint16_t> m_pdcTxTrace;            ///< PDC transmit trace
    TracedCallback<Ptr<Packet>, uint16_t> m_pdcRxTrace;            ///< PDC receive trace
    TracedCallback<const SoftUeStats&> m_statsTrace;                ///< Statistics trace
    TracedCallback<std::string, std::string> m_errorTrace;          ///< Error trace

protected:
    /**
     * @brief DoDispose method for cleanup
     */
    virtual void DoDispose (void) override;

    /**
     * @brief DoInitialize method
     */
    virtual void DoInitialize (void) override;

private:
    // Device state
    uint32_t m_ifIndex;                          ///< Interface index
    Mac48Address m_address;                      ///< MAC address
    uint16_t m_mtu;                             ///< Maximum transmission unit
    bool m_linkUp;                              ///< Link status
    bool m_promisc;                             ///< Promiscuous mode

    // Components
    Ptr<Node> m_node;                           ///< Associated node
    Ptr<SoftUeChannel> m_channel;               ///< Associated channel
    Ptr<SesManager> m_sesManager;                ///< SES manager
    Ptr<PdsManager> m_pdsManager;                ///< PDS manager

    // Configuration
    SoftUeConfig m_config;                       ///< Device configuration
    uint32_t m_localFep;                         ///< Local fabric endpoint

    // Callbacks
    ReceiveCallback m_receiveCallback;           ///< Packet receive callback
    PromiscReceiveCallback m_promiscReceiveCallback; ///< Promiscuous receive callback
    Callback<void> m_linkChangeCallbacks;        ///< Link change callbacks
    Callback<void, uint16_t, PdcType> m_pdcCreatedCallback;   ///< PDC creation callback
    Callback<void, uint16_t> m_pdcDestroyedCallback; ///< PDC destruction callback

    // Statistics and monitoring
    SoftUeStats m_statistics;                    ///< Device statistics
    EventId m_statsEventId;                      ///< Statistics update event ID

    // Packet processing
    Ptr<DropTailQueue<Packet>> m_receiveQueue;   ///< Received packet queue (ns-3 Queue)
    std::map<uint16_t, Ptr<PdcBase>> m_pdcs;     ///< Active PDCs

    // Random variables
    Ptr<UniformRandomVariable> m_rng;            ///< Random number generator

    // Internal helper methods
    void InitializeProtocolStack (void);
    void SetupCallbacks (void);
    void ProcessReceiveQueue (void);
    void UpdateStatistics (void);
    void ScheduleStatisticsUpdate (void);
    bool ValidatePacket (Ptr<Packet> packet) const;
    void LogDetailed (const std::string& function, const std::string& message) const;
    bool HandleProtocolError (const std::string& error, const std::string& details);
    void NotifyLinkChange (void);
    uint16_t ExtractFepFromAddress (const Address& addr) const;
    void OnPdcCreated (uint16_t pdcId, PdcType type);
    void OnPdcDestroyed (uint16_t pdcId);
};

} // namespace ns3

#endif /* SOFT_UE_NET_DEVICE_H */
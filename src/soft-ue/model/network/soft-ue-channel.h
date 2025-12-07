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
 * @file             soft-ue-channel.h
 * @brief            Soft-Ue Channel Implementation
 * @author           softuegroup@gmail.com
 * @version          1.0.0
 * @date             2025-12-07
 * @copyright        Apache License Version 2.0
 *
 * @details
 * This file contains the Soft-Ue channel that provides connectivity between
 * Soft-Ue network devices for Ultra Ethernet protocol transmission.
 */

#ifndef SOFT_UE_CHANNEL_H
#define SOFT_UE_CHANNEL_H

#include "ns3/channel.h"
#include "ns3/net-device.h"
#include "ns3/net-device-container.h"
#include "ns3/nstime.h"
#include "ns3/data-rate.h"
#include "ns3/traced-callback.h"
#include <vector>

namespace ns3 {

/**
 * @class SoftUeChannel
 * @brief Soft-Ue Channel for Ultra Ethernet protocol transmission
 *
 * The SoftUeChannel provides a broadcast-like medium for Soft-Ue network
 * devices, simulating the physical layer characteristics of Ultra Ethernet
 * networks including propagation delay and bandwidth constraints.
 */
class SoftUeChannel : public Channel
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
    SoftUeChannel ();

    /**
     * @brief Destructor
     */
    virtual ~SoftUeChannel ();

    /**
     * @brief Set the data rate of the channel
     * @param dataRate Data rate
     */
    void SetDataRate (DataRate dataRate);

    /**
     * @brief Get the data rate of the channel
     * @return Data rate
     */
    DataRate GetDataRate (void) const;

    /**
     * @brief Set the propagation delay
     * @param delay Propagation delay
     */
    void SetDelay (Time delay);

    /**
     * @brief Get the propagation delay
     * @return Propagation delay
     */
    Time GetDelay (void) const;

    /**
     * @brief Connect devices to this channel
     * @param devices NetDevice container with devices to connect
     */
    void Connect (NetDeviceContainer devices);

    // Channel interface implementation
    virtual void SetPropagationDelay (Time delay);
    virtual Time GetPropagationDelay (void) const;
    virtual void Attach (Ptr<NetDevice> device);
    virtual bool IsAttached (Ptr<NetDevice> device) const;
    virtual std::size_t GetNDevices (void) const override;
    virtual Ptr<NetDevice> GetDevice (std::size_t i) const override;
    virtual std::size_t GetDevice (Ptr<NetDevice> device) const;

    // Soft-Ue specific methods

    /**
     * @brief Transmit packet from source device to all other devices
     * @param packet Packet to transmit
     * @param src Source device
     * @param sourceFep Source fabric endpoint
     * @param destFep Destination fabric endpoint (0 for broadcast)
     */
    void Transmit (Ptr<Packet> packet, Ptr<NetDevice> src, uint32_t sourceFep, uint32_t destFep = 0);

    // Traced callbacks
    TracedCallback<Ptr<const Packet>, uint32_t, uint32_t> m_txTrace;        ///< Transmit trace (packet, srcFep, destFep)
    TracedCallback<Ptr<const Packet>, uint32_t, uint32_t> m_rxTrace;        ///< Receive trace (packet, srcFep, destFep)

protected:
    /**
     * @brief DoDispose method for cleanup
     */
    virtual void DoDispose (void) override;

private:
    DataRate m_dataRate;                           ///< Channel data rate
    Time m_delay;                                   ///< Propagation delay
    std::vector<Ptr<NetDevice>> m_devices;         ///< Connected devices

    void ReceivePacket (Ptr<Packet> packet, Ptr<NetDevice> dest, uint32_t sourceFep, uint32_t destFep);
    void ScheduleReceive (Ptr<Packet> packet, Ptr<NetDevice> dest, uint32_t sourceFep, uint32_t destFep, Time delay);
    Time CalculateTransmissionTime (Ptr<Packet> packet) const;
    uint32_t GetDestinationFepForDevice (Ptr<NetDevice> device) const;
};

} // namespace ns3

#endif /* SOFT_UE_CHANNEL_H */
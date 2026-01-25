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
 * @file             pds-statistics.cc
 * @brief            PDS (Packet Delivery Sub-layer) Statistics Implementation
 * @author           softuegroup@gmail.com
 * @version          1.0.0
 * @date             2025-12-07
 * @copyright        Apache License Version 2.0
 *
 * @details
 * This file contains the implementation of the PDS layer statistics
 * collection for the Ultra Ethernet protocol implementation in ns-3.
 */

#include "pds-common.h"
#include "ns3/log.h"
#include "ns3/assert.h"
#include <sstream>
#include <iomanip>
#include <limits>
#include <cmath>
#include <vector>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("PdsStatistics");

NS_OBJECT_ENSURE_REGISTERED (PdsStatistics);

TypeId
PdsStatistics::GetTypeId (void)
{
    NS_LOG_FUNCTION_NOARGS ();

    static TypeId tid = TypeId ("ns3::PdsStatistics")
        .SetParent<Object> ()
        .SetGroupName ("SoftUe")
        .AddConstructor<PdsStatistics> ();

    return tid;
}

TypeId
PdsStatistics::GetInstanceTypeId (void) const
{
    NS_LOG_FUNCTION (this);
    return GetTypeId ();
}

PdsStatistics::PdsStatistics ()
    : m_receivedPackets (0),
      m_sentPackets (0),
      m_pdcCreations (0),
      m_pdcDestructions (0),
      m_successCount (0),
      m_invalidPdcCount (0),
      m_pdcFullCount (0),
      m_invalidPacketCount (0),
      m_resourceExhaustedCount (0),
      m_protocolErrorCount (0),
      m_totalBytesReceived (0),
      m_totalBytesSent (0),
      m_firstPacketTime (Seconds (0)),
      m_lastPacketTime (Seconds (0)),
      m_minLatency (std::numeric_limits<double>::max()),
      m_maxLatency (0.0),
      m_totalLatency (0.0)
{
    NS_LOG_FUNCTION (this);
    NS_LOG_DEBUG ("PdsStatistics created with data center level metrics");
}

PdsStatistics::~PdsStatistics ()
{
    NS_LOG_FUNCTION (this);
    NS_LOG_DEBUG ("PdsStatistics destroyed");
}

void
PdsStatistics::Reset (void)
{
    NS_LOG_FUNCTION (this);

    m_receivedPackets = 0;
    m_sentPackets = 0;
    m_pdcCreations = 0;
    m_pdcDestructions = 0;
    m_successCount = 0;
    m_invalidPdcCount = 0;
    m_pdcFullCount = 0;
    m_invalidPacketCount = 0;
    m_resourceExhaustedCount = 0;
    m_protocolErrorCount = 0;

    // Reset performance metrics
    m_totalBytesReceived = 0;
    m_totalBytesSent = 0;
    m_firstPacketTime = Seconds (0);
    m_lastPacketTime = Seconds (0);
    m_packetDelays.clear ();
    m_minLatency = std::numeric_limits<double>::max();
    m_maxLatency = 0.0;
    m_totalLatency = 0.0;

    NS_LOG_DEBUG ("All statistics counters reset including performance metrics");
}

void
PdsStatistics::IncrementReceivedPackets (void)
{
    NS_LOG_FUNCTION (this);
    m_receivedPackets++;
    NS_LOG_DEBUG ("Received packets count: " << m_receivedPackets);
}

void
PdsStatistics::IncrementSentPackets (void)
{
    NS_LOG_FUNCTION (this);
    m_sentPackets++;
    NS_LOG_DEBUG ("Sent packets count: " << m_sentPackets);
}

void
PdsStatistics::IncrementPdcCreations (void)
{
    NS_LOG_FUNCTION (this);
    m_pdcCreations++;
    NS_LOG_DEBUG ("PDC creations count: " << m_pdcCreations);
}

void
PdsStatistics::IncrementPdcDestructions (void)
{
    NS_LOG_FUNCTION (this);
    m_pdcDestructions++;
    NS_LOG_DEBUG ("PDC destructions count: " << m_pdcDestructions);
}

void
PdsStatistics::IncrementErrors (PdsErrorCode error)
{
    NS_LOG_FUNCTION (this << static_cast<uint8_t> (error));

    switch (error)
    {
        case PdsErrorCode::SUCCESS:
            m_successCount++;
            break;
        case PdsErrorCode::INVALID_PDC:
            m_invalidPdcCount++;
            break;
        case PdsErrorCode::PDC_FULL:
            m_pdcFullCount++;
            break;
        case PdsErrorCode::INVALID_PACKET:
            m_invalidPacketCount++;
            break;
        case PdsErrorCode::RESOURCE_EXHAUSTED:
            m_resourceExhaustedCount++;
            break;
        case PdsErrorCode::PROTOCOL_ERROR:
            m_protocolErrorCount++;
            break;
        default:
            NS_LOG_WARN ("Unknown error code: " << static_cast<uint8_t> (error));
            break;
    }

    NS_LOG_DEBUG ("Error incremented for code: " << static_cast<uint8_t> (error));
}

std::string
PdsStatistics::GetStatistics (void) const
{
    NS_LOG_FUNCTION (this);

    std::ostringstream oss;
    oss << std::endl;
    oss << "=== PDS Layer Statistics ===" << std::endl;

    // Basic packet statistics
    oss << std::setw(25) << std::left << "Received Packets: "
        << std::setw(12) << std::right << m_receivedPackets << std::endl;
    oss << std::setw(25) << std::left << "Sent Packets: "
        << std::setw(12) << std::right << m_sentPackets << std::endl;
    oss << std::setw(25) << std::left << "PDC Creations: "
        << std::setw(12) << std::right << m_pdcCreations << std::endl;
    oss << std::setw(25) << std::left << "PDC Destructions: "
        << std::setw(12) << std::right << m_pdcDestructions << std::endl;

    // Data center level performance metrics
    oss << std::endl;
    oss << "Data Center Performance Metrics:" << std::endl;
    oss << std::setw(25) << std::left << "  Total Bytes Sent: "
        << std::setw(12) << std::right << m_totalBytesSent << " B" << std::endl;
    oss << std::setw(25) << std::left << "  Total Bytes Received: "
        << std::setw(12) << std::right << m_totalBytesReceived << " B" << std::endl;
    oss << std::setw(25) << std::left << "  Throughput: "
        << std::setw(12) << std::right << std::fixed << std::setprecision(3)
        << GetThroughputMbps () << " Mbps" << std::endl;
    oss << std::setw(25) << std::left << "  Average Latency: "
        << std::setw(12) << std::right << std::fixed << std::setprecision(3)
        << GetAverageLatencyNs () << " ns" << std::endl;
    oss << std::setw(25) << std::left << "  Min Latency: "
        << std::setw(12) << std::right << std::fixed << std::setprecision(3)
        << (m_minLatency == std::numeric_limits<double>::max() ? 0.0 : m_minLatency) << " ns" << std::endl;
    oss << std::setw(25) << std::left << "  Max Latency: "
        << std::setw(12) << std::right << std::fixed << std::setprecision(3)
        << m_maxLatency << " ns" << std::endl;
    oss << std::setw(25) << std::left << "  Jitter: "
        << std::setw(12) << std::right << std::fixed << std::setprecision(3)
        << GetJitterNs () << " ns" << std::endl;

    // Error statistics
    oss << std::endl;
    oss << "Error Statistics:" << std::endl;
    oss << std::setw(25) << std::left << "  Successful Operations: "
        << std::setw(12) << std::right << m_successCount << std::endl;
    oss << std::setw(25) << std::left << "  Invalid PDC Errors: "
        << std::setw(12) << std::right << m_invalidPdcCount << std::endl;
    oss << std::setw(25) << std::left << "  PDC Full Errors: "
        << std::setw(12) << std::right << m_pdcFullCount << std::endl;
    oss << std::setw(25) << std::left << "  Invalid Packet Errors: "
        << std::setw(12) << std::right << m_invalidPacketCount << std::endl;
    oss << std::setw(25) << std::left << "  Resource Exhausted: "
        << std::setw(12) << std::right << m_resourceExhaustedCount << std::endl;
    oss << std::setw(25) << std::left << "  Protocol Violations: "
        << std::setw(12) << std::right << m_protocolErrorCount << std::endl;
    oss << "==============================" << std::endl;

    std::string stats = oss.str();
    NS_LOG_DEBUG ("Statistics generated: " << stats);

    return stats;
}

void
PdsStatistics::RecordPacketTransmission (uint64_t bytes, double latency)
{
    NS_LOG_FUNCTION (this << bytes << latency);

    m_sentPackets++;
    m_totalBytesSent += bytes;
    m_totalLatency += latency;

    // Update min/max latency
    if (latency < m_minLatency)
    {
        m_minLatency = latency;
    }
    if (latency > m_maxLatency)
    {
        m_maxLatency = latency;
    }

    // Record timestamp for first packet
    if (m_firstPacketTime == Seconds (0))
    {
        m_firstPacketTime = Simulator::Now ();
    }
    m_lastPacketTime = Simulator::Now ();

    NS_LOG_DEBUG ("Packet transmission recorded: " << bytes << " bytes, " << latency << "ns");
}

void
PdsStatistics::RecordPacketReception (uint64_t bytes, double latency)
{
    NS_LOG_FUNCTION (this << bytes << latency);

    m_receivedPackets++;
    m_totalBytesReceived += bytes;
    m_totalLatency += latency;

    // Record delay for jitter calculation
    m_packetDelays.push_back (NanoSeconds (static_cast<uint64_t> (latency)));

    // Update min/max latency
    if (latency < m_minLatency)
    {
        m_minLatency = latency;
    }
    if (latency > m_maxLatency)
    {
        m_maxLatency = latency;
    }

    // Record timestamp for first packet
    if (m_firstPacketTime == Seconds (0))
    {
        m_firstPacketTime = Simulator::Now ();
    }
    m_lastPacketTime = Simulator::Now ();

    NS_LOG_DEBUG ("Packet reception recorded: " << bytes << " bytes, " << latency << "ns");
}

void
PdsStatistics::RecordBytesSent (uint64_t bytes)
{
    NS_LOG_FUNCTION (this << bytes);
    m_totalBytesSent += bytes;

    if (m_firstPacketTime == Seconds (0))
    {
        m_firstPacketTime = Simulator::Now ();
    }
    m_lastPacketTime = Simulator::Now ();
}

void
PdsStatistics::RecordBytesReceived (uint64_t bytes)
{
    NS_LOG_FUNCTION (this << bytes);
    m_totalBytesReceived += bytes;

    if (m_firstPacketTime == Seconds (0))
    {
        m_firstPacketTime = Simulator::Now ();
    }
    m_lastPacketTime = Simulator::Now ();
}

double
PdsStatistics::GetThroughputMbps (void) const
{
    if (m_firstPacketTime == Seconds (0) || m_lastPacketTime == Seconds (0))
    {
        return 0.0;
    }

    Time totalTime = m_lastPacketTime - m_firstPacketTime;
    if (totalTime.GetNanoSeconds () == 0)
    {
        return 0.0;
    }

    double totalBytes = static_cast<double> (m_totalBytesSent + m_totalBytesReceived);
    double totalBits = totalBytes * 8.0;
    double totalSeconds = totalTime.GetNanoSeconds () / 1000000000.0;

    return totalBits / totalSeconds / 1000000.0; // Convert to Mbps
}

double
PdsStatistics::GetAverageLatencyNs (void) const
{
    uint64_t totalPackets = m_sentPackets + m_receivedPackets;
    if (totalPackets == 0)
    {
        return 0.0;
    }

    return m_totalLatency / static_cast<double> (totalPackets);
}

double
PdsStatistics::GetJitterNs (void) const
{
    if (m_packetDelays.size () < 2)
    {
        return 0.0;
    }

    // Calculate standard deviation of packet delays
    double meanLatency = GetAverageLatencyNs ();
    double variance = 0.0;

    for (const auto& delay : m_packetDelays)
    {
        double diff = delay.GetNanoSeconds () - meanLatency;
        variance += diff * diff;
    }

    variance /= static_cast<double> (m_packetDelays.size ());
    return std::sqrt (variance);
}

} // namespace ns3
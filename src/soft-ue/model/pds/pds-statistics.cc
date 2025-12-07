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
      m_protocolErrorCount (0)
{
    NS_LOG_FUNCTION (this);
    NS_LOG_DEBUG ("PdsStatistics created");
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

    NS_LOG_DEBUG ("All statistics counters reset");
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
    oss << std::setw(25) << std::left << "Received Packets: "
        << std::setw(12) << std::right << m_receivedPackets << std::endl;
    oss << std::setw(25) << std::left << "Sent Packets: "
        << std::setw(12) << std::right << m_sentPackets << std::endl;
    oss << std::setw(25) << std::left << "PDC Creations: "
        << std::setw(12) << std::right << m_pdcCreations << std::endl;
    oss << std::setw(25) << std::left << "PDC Destructions: "
        << std::setw(12) << std::right << m_pdcDestructions << std::endl;
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

} // namespace ns3
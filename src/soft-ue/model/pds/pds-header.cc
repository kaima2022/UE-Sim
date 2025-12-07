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
 * @file             pds-header.cc
 * @brief            PDS Header implementation
 * @author           softuegroup@gmail.com
 * @version          1.0.0
 * @date             2025-12-07
 * @copyright        Apache License Version 2.0
 *
 * @details
 * This file contains the implementation of the PDS layer header
 * for the Ultra Ethernet protocol in ns-3.
 */

#include "pds-common.h"
#include "ns3/log.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("PDSHeader");

NS_OBJECT_ENSURE_REGISTERED(PDSHeader);

TypeId
PDSHeader::GetTypeId (void)
{
    static TypeId tid = TypeId("ns3::PDSHeader")
        .SetParent<Header>()
        .SetGroupName("SoftUe")
        .AddConstructor<PDSHeader>();
    return tid;
}

TypeId
PDSHeader::GetInstanceTypeId (void) const
{
    return GetTypeId();
}

PDSHeader::PDSHeader ()
    : m_pdcId (0),
      m_sequenceNum (0),
      m_som (false),
      m_eom (false)
{
    NS_LOG_FUNCTION(this);
}

PDSHeader::~PDSHeader ()
{
    NS_LOG_FUNCTION(this);
}

void
PDSHeader::Print (std::ostream &os) const
{
    NS_LOG_FUNCTION(this << &os);
    os << "PDSHeader[PDC=" << m_pdcId
       << ", Seq=" << m_sequenceNum
       << ", SOM=" << m_som
       << ", EOM=" << m_eom << "]";
}

uint32_t
PDSHeader::GetSerializedSize (void) const
{
    NS_LOG_FUNCTION(this);
    // 2 bytes PDC ID + 4 bytes sequence + 1 byte flags
    return 7;
}

void
PDSHeader::Serialize (Buffer::Iterator start) const
{
    NS_LOG_FUNCTION(this << &start);
    Buffer::Iterator i = start;

    i.WriteU16(m_pdcId);
    i.WriteU32(m_sequenceNum);

    uint8_t flags = 0;
    if (m_som) flags |= 0x01;
    if (m_eom) flags |= 0x02;
    i.WriteU8(flags);
}

uint32_t
PDSHeader::Deserialize (Buffer::Iterator start)
{
    NS_LOG_FUNCTION(this << &start);
    Buffer::Iterator i = start;

    m_pdcId = i.ReadU16();
    m_sequenceNum = i.ReadU32();

    uint8_t flags = i.ReadU8();
    m_som = (flags & 0x01) != 0;
    m_eom = (flags & 0x02) != 0;

    return GetSerializedSize();
}

void
PDSHeader::SetPdcId (uint16_t pdcId)
{
    NS_LOG_FUNCTION(this << pdcId);
    m_pdcId = pdcId;
}

uint16_t
PDSHeader::GetPdcId (void) const
{
    NS_LOG_FUNCTION(this);
    return m_pdcId;
}

void
PDSHeader::SetSequenceNumber (uint32_t seqNum)
{
    NS_LOG_FUNCTION(this << seqNum);
    m_sequenceNum = seqNum;
}

uint32_t
PDSHeader::GetSequenceNumber (void) const
{
    NS_LOG_FUNCTION(this);
    return m_sequenceNum;
}

void
PDSHeader::SetSom (bool som)
{
    NS_LOG_FUNCTION(this << som);
    m_som = som;
}

bool
PDSHeader::GetSom (void) const
{
    NS_LOG_FUNCTION(this);
    return m_som;
}

void
PDSHeader::SetEom (bool eom)
{
    NS_LOG_FUNCTION(this << eom);
    m_eom = eom;
}

bool
PDSHeader::GetEom (void) const
{
    NS_LOG_FUNCTION(this);
    return m_eom;
}

} // namespace ns3
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
 * @file             soft-ue-packet-tag.cc
 * @brief            ns-3 Packet Tag Implementation for Soft-UE Protocol Information
 * @author           softuegroup@gmail.com
 * @version          1.0.0
 * @date             2025-12-10
 * @copyright        Apache License Version 2.0
 *
 * @details
 * This file implements the packet tag classes used to attach Soft-UE protocol
 * information to ns-3 packets for transport through the ns-3 simulation framework.
 */

#include "soft-ue-packet-tag.h"
#include "ns3/log.h"
#include "ns3/type-id.h"
#include "ns3/simulator.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("SoftUePacketTag");

// ============================================================================
// SoftUeHeaderTag Implementation
// ============================================================================

NS_OBJECT_ENSURE_REGISTERED (SoftUeHeaderTag);

TypeId
SoftUeHeaderTag::GetTypeId (void)
{
    NS_LOG_FUNCTION_NOARGS ();
    static TypeId tid = TypeId ("ns3::SoftUeHeaderTag")
        .SetParent<Tag> ()
        .SetGroupName ("SoftUe")
        .AddConstructor<SoftUeHeaderTag> ();
    return tid;
}

TypeId
SoftUeHeaderTag::GetInstanceTypeId (void) const
{
    NS_LOG_FUNCTION (this);
    return GetTypeId ();
}

uint32_t
SoftUeHeaderTag::GetSerializedSize (void) const
{
    NS_LOG_FUNCTION (this);
    return sizeof (PDSType) + sizeof (uint16_t) + sizeof (uint32_t) * 4;
}

void
SoftUeHeaderTag::Serialize (TagBuffer i) const
{
    NS_LOG_FUNCTION (this << &i);
    i.WriteU8 ((uint8_t)m_pdsType);
    i.WriteU16 (m_pdcId);
    i.WriteU32 (m_psn);
    i.WriteU32 (m_sourceEndpoint);
    i.WriteU32 (m_destEndpoint);
    i.WriteU32 (m_jobId);
}

void
SoftUeHeaderTag::Deserialize (TagBuffer i)
{
    NS_LOG_FUNCTION (this << &i);
    m_pdsType = (PDSType)i.ReadU8 ();
    m_pdcId = i.ReadU16 ();
    m_psn = i.ReadU32 ();
    m_sourceEndpoint = i.ReadU32 ();
    m_destEndpoint = i.ReadU32 ();
    m_jobId = i.ReadU32 ();
}

void
SoftUeHeaderTag::Print (std::ostream &os) const
{
    NS_LOG_FUNCTION (this << &os);
    os << "SoftUeHeaderTag [pdsType=" << (uint32_t)m_pdsType
       << ", pdcId=" << m_pdcId
       << ", psn=" << m_psn
       << ", src=" << m_sourceEndpoint
       << ", dst=" << m_destEndpoint
       << ", jobId=" << m_jobId << "]";
}

SoftUeHeaderTag::SoftUeHeaderTag ()
    : m_pdsType (PDSType::RESERVED), m_pdcId (0), m_psn (0),
      m_sourceEndpoint (0), m_destEndpoint (0), m_jobId (0)
{
    NS_LOG_FUNCTION (this);
}

SoftUeHeaderTag::SoftUeHeaderTag (PDSType pdsType, uint16_t pdcId, uint32_t psn)
    : m_pdsType (pdsType), m_pdcId (pdcId), m_psn (psn),
      m_sourceEndpoint (0), m_destEndpoint (0), m_jobId (0)
{
    NS_LOG_FUNCTION (this << (uint32_t)pdsType << pdcId << psn);
}

PDSType
SoftUeHeaderTag::GetPdsType (void) const
{
    NS_LOG_FUNCTION (this);
    return m_pdsType;
}

void
SoftUeHeaderTag::SetPdsType (PDSType pdsType)
{
    NS_LOG_FUNCTION (this << (uint32_t)pdsType);
    m_pdsType = pdsType;
}

uint16_t
SoftUeHeaderTag::GetPdcId (void) const
{
    NS_LOG_FUNCTION (this);
    return m_pdcId;
}

void
SoftUeHeaderTag::SetPdcId (uint16_t pdcId)
{
    NS_LOG_FUNCTION (this << pdcId);
    m_pdcId = pdcId;
}

uint32_t
SoftUeHeaderTag::GetPsn (void) const
{
    NS_LOG_FUNCTION (this);
    return m_psn;
}

void
SoftUeHeaderTag::SetPsn (uint32_t psn)
{
    NS_LOG_FUNCTION (this << psn);
    m_psn = psn;
}

uint32_t
SoftUeHeaderTag::GetSourceEndpoint (void) const
{
    NS_LOG_FUNCTION (this);
    return m_sourceEndpoint;
}

void
SoftUeHeaderTag::SetSourceEndpoint (uint32_t endpoint)
{
    NS_LOG_FUNCTION (this << endpoint);
    m_sourceEndpoint = endpoint;
}

uint32_t
SoftUeHeaderTag::GetDestinationEndpoint (void) const
{
    NS_LOG_FUNCTION (this);
    return m_destEndpoint;
}

void
SoftUeHeaderTag::SetDestinationEndpoint (uint32_t endpoint)
{
    NS_LOG_FUNCTION (this << endpoint);
    m_destEndpoint = endpoint;
}

uint32_t
SoftUeHeaderTag::GetJobId (void) const
{
    NS_LOG_FUNCTION (this);
    return m_jobId;
}

void
SoftUeHeaderTag::SetJobId (uint32_t jobId)
{
    NS_LOG_FUNCTION (this << jobId);
    m_jobId = jobId;
}

// ============================================================================
// SoftUeMetadataTag Implementation
// ============================================================================

NS_OBJECT_ENSURE_REGISTERED (SoftUeMetadataTag);

TypeId
SoftUeMetadataTag::GetTypeId (void)
{
    NS_LOG_FUNCTION_NOARGS ();
    static TypeId tid = TypeId ("ns3::SoftUeMetadataTag")
        .SetParent<Tag> ()
        .SetGroupName ("SoftUe")
        .AddConstructor<SoftUeMetadataTag> ();
    return tid;
}

TypeId
SoftUeMetadataTag::GetInstanceTypeId (void) const
{
    NS_LOG_FUNCTION (this);
    return GetTypeId ();
}

uint32_t
SoftUeMetadataTag::GetSerializedSize (void) const
{
    NS_LOG_FUNCTION (this);
    return sizeof (uint32_t) + sizeof (uint32_t) + sizeof (uint16_t) + sizeof (uint8_t);
}

void
SoftUeMetadataTag::Serialize (TagBuffer i) const
{
    NS_LOG_FUNCTION (this << &i);
    i.WriteU32 ((uint32_t)m_opType);
    i.WriteU32 (m_messageId);
    i.WriteU16 (m_resourceIndex);
    i.WriteU8 (m_reliable ? 1 : 0);
}

void
SoftUeMetadataTag::Deserialize (TagBuffer i)
{
    NS_LOG_FUNCTION (this << &i);
    m_opType = (OpType)i.ReadU32 ();
    m_messageId = i.ReadU32 ();
    m_resourceIndex = i.ReadU16 ();
    m_reliable = (i.ReadU8 () != 0);
}

void
SoftUeMetadataTag::Print (std::ostream &os) const
{
    NS_LOG_FUNCTION (this << &os);
    os << "SoftUeMetadataTag [opType=" << (uint32_t)m_opType
       << ", messageId=" << m_messageId
       << ", resourceIndex=" << m_resourceIndex
       << ", reliable=" << (m_reliable ? "true" : "false") << "]";
}

SoftUeMetadataTag::SoftUeMetadataTag ()
    : m_opType (OpType::SEND), m_messageId (0), m_resourceIndex (0), m_reliable (false)
{
    NS_LOG_FUNCTION (this);
}

SoftUeMetadataTag::SoftUeMetadataTag (const OperationMetadata& metadata)
    : m_opType (metadata.op_type), m_messageId (metadata.messages_id),
      m_resourceIndex (metadata.res_index), m_reliable (false)  // Default to false since not in struct
{
    NS_LOG_FUNCTION (this << (uint32_t)m_opType << m_messageId << m_resourceIndex << m_reliable);
}

OpType
SoftUeMetadataTag::GetOperationType (void) const
{
    NS_LOG_FUNCTION (this);
    return m_opType;
}

void
SoftUeMetadataTag::SetOperationType (OpType opType)
{
    NS_LOG_FUNCTION (this << (uint32_t)opType);
    m_opType = opType;
}

uint32_t
SoftUeMetadataTag::GetMessageId (void) const
{
    NS_LOG_FUNCTION (this);
    return m_messageId;
}

void
SoftUeMetadataTag::SetMessageId (uint32_t messageId)
{
    NS_LOG_FUNCTION (this << messageId);
    m_messageId = messageId;
}

uint16_t
SoftUeMetadataTag::GetResourceIndex (void) const
{
    NS_LOG_FUNCTION (this);
    return m_resourceIndex;
}

void
SoftUeMetadataTag::SetResourceIndex (uint16_t resourceIndex)
{
    NS_LOG_FUNCTION (this << resourceIndex);
    m_resourceIndex = resourceIndex;
}

bool
SoftUeMetadataTag::IsReliable (void) const
{
    NS_LOG_FUNCTION (this);
    return m_reliable;
}

void
SoftUeMetadataTag::SetReliable (bool reliable)
{
    NS_LOG_FUNCTION (this << reliable);
    m_reliable = reliable;
}

// ============================================================================
// SoftUeTimingTag Implementation
// ============================================================================

NS_OBJECT_ENSURE_REGISTERED (SoftUeTimingTag);

TypeId
SoftUeTimingTag::GetTypeId (void)
{
    NS_LOG_FUNCTION_NOARGS ();
    static TypeId tid = TypeId ("ns3::SoftUeTimingTag")
        .SetParent<Tag> ()
        .SetGroupName ("SoftUe")
        .AddConstructor<SoftUeTimingTag> ();
    return tid;
}

TypeId
SoftUeTimingTag::GetInstanceTypeId (void) const
{
    NS_LOG_FUNCTION (this);
    return GetTypeId ();
}

uint32_t
SoftUeTimingTag::GetSerializedSize (void) const
{
    NS_LOG_FUNCTION (this);
    return 2 * sizeof (int64_t); // Two timestamps
}

void
SoftUeTimingTag::Serialize (TagBuffer i) const
{
    NS_LOG_FUNCTION (this << &i);
    i.WriteU64 ((int64_t)m_timestamp.GetNanoSeconds ());
    i.WriteU64 ((int64_t)m_expectedDeliveryTime.GetNanoSeconds ());
}

void
SoftUeTimingTag::Deserialize (TagBuffer i)
{
    NS_LOG_FUNCTION (this << &i);
    m_timestamp = NanoSeconds (i.ReadU64 ());
    m_expectedDeliveryTime = NanoSeconds (i.ReadU64 ());
}

void
SoftUeTimingTag::Print (std::ostream &os) const
{
    NS_LOG_FUNCTION (this << &os);
    os << "SoftUeTimingTag [timestamp=" << m_timestamp.GetNanoSeconds ()
       << "ns, expected=" << m_expectedDeliveryTime.GetNanoSeconds () << "ns]";
}

SoftUeTimingTag::SoftUeTimingTag ()
    : m_timestamp (Simulator::Now ()), m_expectedDeliveryTime (Time::Max ())
{
    NS_LOG_FUNCTION (this);
}

SoftUeTimingTag::SoftUeTimingTag (Time timestamp)
    : m_timestamp (timestamp), m_expectedDeliveryTime (Time::Max ())
{
    NS_LOG_FUNCTION (this << timestamp.GetNanoSeconds ());
}

Time
SoftUeTimingTag::GetTimestamp (void) const
{
    NS_LOG_FUNCTION (this);
    return m_timestamp;
}

void
SoftUeTimingTag::SetTimestamp (Time timestamp)
{
    NS_LOG_FUNCTION (this << timestamp.GetNanoSeconds ());
    m_timestamp = timestamp;
}

Time
SoftUeTimingTag::GetExpectedDeliveryTime (void) const
{
    NS_LOG_FUNCTION (this);
    return m_expectedDeliveryTime;
}

void
SoftUeTimingTag::SetExpectedDeliveryTime (Time time)
{
    NS_LOG_FUNCTION (this << time.GetNanoSeconds ());
    m_expectedDeliveryTime = time;
}

// ============================================================================
// SoftUeFragmentTag Implementation
// ============================================================================

NS_OBJECT_ENSURE_REGISTERED (SoftUeFragmentTag);

TypeId
SoftUeFragmentTag::GetTypeId (void)
{
    static TypeId tid = TypeId ("ns3::SoftUeFragmentTag")
        .SetParent<Tag> ()
        .SetGroupName ("SoftUe")
        .AddConstructor<SoftUeFragmentTag> ();
    return tid;
}

TypeId
SoftUeFragmentTag::GetInstanceTypeId (void) const
{
    return GetTypeId ();
}

uint32_t
SoftUeFragmentTag::GetSerializedSize (void) const
{
    return sizeof (uint32_t) * 2;
}

void
SoftUeFragmentTag::Serialize (TagBuffer i) const
{
    i.WriteU32 (m_fragmentIndex);
    i.WriteU32 (m_totalFragments);
}

void
SoftUeFragmentTag::Deserialize (TagBuffer i)
{
    m_fragmentIndex = i.ReadU32 ();
    m_totalFragments = i.ReadU32 ();
}

void
SoftUeFragmentTag::Print (std::ostream &os) const
{
    os << "SoftUeFragmentTag [fragment=" << m_fragmentIndex << "/" << m_totalFragments << "]";
}

SoftUeFragmentTag::SoftUeFragmentTag ()
    : m_fragmentIndex (0), m_totalFragments (0)
{
}

SoftUeFragmentTag::SoftUeFragmentTag (uint32_t fragmentIndex, uint32_t totalFragments)
    : m_fragmentIndex (fragmentIndex), m_totalFragments (totalFragments)
{
}

uint32_t
SoftUeFragmentTag::GetFragmentIndex (void) const
{
    return m_fragmentIndex;
}

void
SoftUeFragmentTag::SetFragmentIndex (uint32_t index)
{
    m_fragmentIndex = index;
}

uint32_t
SoftUeFragmentTag::GetTotalFragments (void) const
{
    return m_totalFragments;
}

void
SoftUeFragmentTag::SetTotalFragments (uint32_t total)
{
    m_totalFragments = total;
}

} // namespace ns3
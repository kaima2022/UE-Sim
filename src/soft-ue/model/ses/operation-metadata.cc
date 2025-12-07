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
 * @file             operation-metadata.cc
 * @brief            Ultra Ethernet SES Layer Operation Metadata Implementation
 * @author           softuegroup@gmail.com
 * @version          1.0.0
 * @date             2025-12-08
 * @copyright        Apache License Version 2.0
 *
 * @details
 * This file contains the implementation of ExtendedOperationMetadata class
 * used by the SES layer for Ultra Ethernet protocol operations.
 */

#include "operation-metadata.h"
#include "ns3/log.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("ExtendedOperationMetadata");
NS_OBJECT_ENSURE_REGISTERED (ExtendedOperationMetadata);

TypeId
ExtendedOperationMetadata::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::ExtendedOperationMetadata")
    .SetParent<Object> ()
    .SetGroupName ("Soft-Ue")
    .AddConstructor<ExtendedOperationMetadata> ()
    ;
  return tid;
}

TypeId
ExtendedOperationMetadata::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

ExtendedOperationMetadata::ExtendedOperationMetadata ()
{
  NS_LOG_FUNCTION (this);
}

ExtendedOperationMetadata::ExtendedOperationMetadata (const OperationMetadata& baseMetadata)
  : OperationMetadata (baseMetadata)
{
  NS_LOG_FUNCTION (this);
}

ExtendedOperationMetadata::~ExtendedOperationMetadata ()
{
  NS_LOG_FUNCTION (this);
}

void
ExtendedOperationMetadata::SetSourceEndpoint (uint32_t nodeId, uint16_t endpointId)
{
  NS_LOG_FUNCTION (this << nodeId << endpointId);
  m_sourceNodeId = nodeId;
  m_sourceEndpointId = endpointId;
}

void
ExtendedOperationMetadata::SetDestinationEndpoint (uint32_t nodeId, uint16_t endpointId)
{
  NS_LOG_FUNCTION (this << nodeId << endpointId);
  m_destNodeId = nodeId;
  m_destEndpointId = endpointId;
}

uint32_t
ExtendedOperationMetadata::GetSourceNodeId (void) const
{
  return m_sourceNodeId;
}

uint16_t
ExtendedOperationMetadata::GetSourceEndpointId (void) const
{
  return m_sourceEndpointId;
}

uint32_t
ExtendedOperationMetadata::GetDestinationNodeId (void) const
{
  return m_destNodeId;
}

uint16_t
ExtendedOperationMetadata::GetDestinationEndpointId (void) const
{
  return m_destEndpointId;
}

bool
ExtendedOperationMetadata::IsValid (void) const
{
  NS_LOG_FUNCTION (this);

  // Basic validation checks
  return (m_sourceNodeId > 0 || m_destNodeId > 0) &&
         (m_sourceEndpointId > 0 || m_destEndpointId > 0);
}

std::string
ExtendedOperationMetadata::ToString (void) const
{
  NS_LOG_FUNCTION (this);

  std::ostringstream oss;
  oss << "ExtendedOperationMetadata["
      << "srcNode=" << m_sourceNodeId
      << ",srcEp=" << m_sourceEndpointId
      << ",destNode=" << m_destNodeId
      << ",destEp=" << m_destEndpointId
      << "]";
  return oss.str ();
}

} // namespace ns3
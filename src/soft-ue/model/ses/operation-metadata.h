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
 * @file             operation-metadata.h
 * @brief            Ultra Ethernet SES Layer Operation Metadata
 * @author           softuegroup@gmail.com
 * @version          1.0.0
 * @date             2025-12-07
 * @copyright        Apache License Version 2.0
 *
 * @details
 * This file contains the operation metadata structure used by the SES layer
 * to handle Ultra Ethernet protocol operations within the ns-3 simulation framework.
 */

#ifndef OPERATION_METADATA_H
#define OPERATION_METADATA_H

#include "ns3/object.h"
#include "ns3/nstime.h"
#include "../common/transport-layer.h"

namespace ns3 {

/**
 * @class ExtendedOperationMetadata
 * @brief Extended operation metadata for ns-3 SES layer
 *
 * This class extends the basic OperationMetadata with ns-3 specific functionality
 * for Ultra Ethernet protocol operations.
 */
class ExtendedOperationMetadata : public Object, public OperationMetadata
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
    ExtendedOperationMetadata ();

    /**
     * @brief Constructor with basic operation metadata
     * @param baseMetadata Base operation metadata to extend
     */
    ExtendedOperationMetadata (const OperationMetadata& baseMetadata);

    /**
     * @brief Destructor
     */
    virtual ~ExtendedOperationMetadata ();

    /**
     * @brief Set source endpoint information
     * @param nodeId Source node ID
     * @param endpointId Source endpoint ID
     */
    void SetSourceEndpoint (uint32_t nodeId, uint16_t endpointId);

    /**
     * @brief Set destination endpoint information
     * @param nodeId Destination node ID
     * @param endpointId Destination endpoint ID
     */
    void SetDestinationEndpoint (uint32_t nodeId, uint16_t endpointId);

    /**
     * @brief Get source node ID
     * @return Source node ID
     */
    uint32_t GetSourceNodeId (void) const;

    /**
     * @brief Get source endpoint ID
     * @return Source endpoint ID
     */
    uint16_t GetSourceEndpointId (void) const;

    /**
     * @brief Get destination node ID
     * @return Destination node ID
     */
    uint32_t GetDestinationNodeId (void) const;

    /**
     * @brief Get destination endpoint ID
     * @return Destination endpoint ID
     */
    uint16_t GetDestinationEndpointId (void) const;

    /**
     * @brief Validate operation metadata
     * @return true if metadata is valid
     */
    bool IsValid (void) const;

    /**
     * @brief Convert to string representation
     * @return String representation
     */
    std::string ToString (void) const;

    /**
     * @brief Calculate required packet count for this operation
     * @param mtu Maximum transmission unit
     * @return Number of packets required
     */
    uint32_t CalculatePacketCount (uint32_t mtu) const;

    /**
     * @brief Check if this operation requires fragmentation
     * @param mtu Maximum transmission unit
     * @return true if fragmentation is required
     */
    bool RequiresFragmentation (uint32_t mtu) const;

private:
    uint32_t m_sourceNodeId;        ///< Source node ID in ns-3 simulation
    uint16_t m_sourceEndpointId;     ///< Source endpoint ID
    uint32_t m_destNodeId;          ///< Destination node ID in ns-3 simulation
    uint16_t m_destEndpointId;      ///< Destination endpoint ID
    Time m_creationTime;            ///< Creation timestamp for tracking
    uint64_t m_operationId;         ///< Unique operation identifier

    static uint64_t s_nextOperationId; ///< Static counter for unique IDs
};

/**
 * @class OperationMetadataFactory
 * @brief Factory class for creating operation metadata
 */
class OperationMetadataFactory
{
public:
    /**
     * @brief Create a SEND operation metadata
     * @param srcNode Source node ID
     * @param srcEp Source endpoint ID
     * @param dstNode Destination node ID
     * @param dstEp Destination endpoint ID
     * @param dataPtr Pointer to data
     * @param dataSize Size of data
     * @return Extended operation metadata
     */
    static Ptr<ExtendedOperationMetadata> CreateSendOperation (
        uint32_t srcNode, uint16_t srcEp,
        uint32_t dstNode, uint16_t dstEp,
        const uint8_t* dataPtr, size_t dataSize);

    /**
     * @brief Create a READ operation metadata
     * @param srcNode Source node ID
     * @param srcEp Source endpoint ID
     * @param dstNode Destination node ID
     * @param dstEp Destination endpoint ID
     * @param remoteAddr Remote memory address
     * @param size Size of data to read
     * @param rkey Remote memory key
     * @return Extended operation metadata
     */
    static Ptr<ExtendedOperationMetadata> CreateReadOperation (
        uint32_t srcNode, uint16_t srcEp,
        uint32_t dstNode, uint16_t dstEp,
        uint64_t remoteAddr, size_t size, uint64_t rkey);

    /**
     * @brief Create a WRITE operation metadata
     * @param srcNode Source node ID
     * @param srcEp Source endpoint ID
     * @param dstNode Destination node ID
     * @param dstEp Destination endpoint ID
     * @param dataPtr Pointer to data
     * @param dataSize Size of data
     * @param remoteAddr Remote memory address
     * @param rkey Remote memory key
     * @return Extended operation metadata
     */
    static Ptr<ExtendedOperationMetadata> CreateWriteOperation (
        uint32_t srcNode, uint16_t srcEp,
        uint32_t dstNode, uint16_t dstEp,
        const uint8_t* dataPtr, size_t dataSize,
        uint64_t remoteAddr, uint64_t rkey);
};

} // namespace ns3

#endif /* OPERATION_METADATA_H */
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
 * @file             soft-ue-packet-tag.h
 * @brief            ns-3 Packet Tag for Soft-UE Protocol Information
 * @author           softuegroup@gmail.com
 * @version          1.0.0
 * @date             2025-12-07
 * @copyright        Apache License Version 2.0
 *
 * @details
 * This file defines the packet tag classes used to attach Soft-UE protocol
 * information to ns-3 packets for transport through the ns-3 simulation framework.
 */

#ifndef SOFT_UE_PACKET_TAG_H
#define SOFT_UE_PACKET_TAG_H

#include <ns3/tag.h>
#include <ns3/tag-buffer.h>
#include <ns3/type-id.h>
#include <ns3/nstime.h>
#include "transport-layer.h"

namespace ns3 {

/**
 * @class SoftUeHeaderTag
 * @brief ns-3 packet tag for Soft-UE protocol headers
 *
 * This tag stores the Soft-UE protocol header information that needs to
 * be carried with ns-3 packets. It allows the Soft-UE protocol to operate
 * within the ns-3 packet framework while maintaining the original protocol
 * semantics.
 */
class SoftUeHeaderTag : public Tag
{
public:
    /**
     * @brief Get the type ID for this tag
     * @return TypeId
     */
    static TypeId GetTypeId (void);

    /**
     * @brief Get the instance type ID
     * @return TypeId
     */
    virtual TypeId GetInstanceTypeId (void) const override;

    /**
     * @brief Get the tag size in bytes
     * @return Tag size
     */
    virtual uint32_t GetSerializedSize (void) const override;

    /**
     * @brief Serialize the tag to a buffer
     * @param i The buffer to serialize to
     */
    virtual void Serialize (TagBuffer i) const override;

    /**
     * @brief Deserialize the tag from a buffer
     * @param i The buffer to deserialize from
     */
    virtual void Deserialize (TagBuffer i) override;

    /**
     * @brief Print the tag information
     * @param os Output stream to print to
     */
    virtual void Print (std::ostream &os) const override;

    /**
     * @brief Default constructor
     */
    SoftUeHeaderTag ();

    /**
     * @brief Constructor with parameters
     * @param pdsType PDS packet type
     * @param pdcId PDC identifier
     * @param psn Packet sequence number
     */
    SoftUeHeaderTag (PDSType pdsType, uint16_t pdcId, uint32_t psn);

    /**
     * @brief Get PDS packet type
     * @return PDS packet type
     */
    PDSType GetPdsType (void) const;

    /**
     * @brief Set PDS packet type
     * @param pdsType PDS packet type
     */
    void SetPdsType (PDSType pdsType);

    /**
     * @brief Get PDC identifier
     * @return PDC identifier
     */
    uint16_t GetPdcId (void) const;

    /**
     * @brief Set PDC identifier
     * @param pdcId PDC identifier
     */
    void SetPdcId (uint16_t pdcId);

    /**
     * @brief Get packet sequence number
     * @return Packet sequence number
     */
    uint32_t GetPsn (void) const;

    /**
     * @brief Set packet sequence number
     * @param psn Packet sequence number
     */
    void SetPsn (uint32_t psn);

    /**
     * @brief Get source endpoint
     * @return Source endpoint identifier
     */
    uint32_t GetSourceEndpoint (void) const;

    /**
     * @brief Set source endpoint
     * @param endpoint Source endpoint identifier
     */
    void SetSourceEndpoint (uint32_t endpoint);

    /**
     * @brief Get destination endpoint
     * @return Destination endpoint identifier
     */
    uint32_t GetDestinationEndpoint (void) const;

    /**
     * @brief Set destination endpoint
     * @param endpoint Destination endpoint identifier
     */
    void SetDestinationEndpoint (uint32_t endpoint);

    /**
     * @brief Get job identifier
     * @return Job identifier
     */
    uint32_t GetJobId (void) const;

    /**
     * @brief Set job identifier
     * @param jobId Job identifier
     */
    void SetJobId (uint32_t jobId);

private:
    PDSType m_pdsType;           ///< PDS packet type
    uint16_t m_pdcId;            ///< PDC identifier
    uint32_t m_psn;              ///< Packet sequence number
    uint32_t m_sourceEndpoint;   ///< Source endpoint identifier
    uint32_t m_destEndpoint;     ///< Destination endpoint identifier
    uint32_t m_jobId;            ///< Job identifier
};

/**
 * @class SoftUeMetadataTag
 * @brief ns-3 packet tag for Soft-UE operation metadata
 *
 * This tag stores additional metadata about the operation associated
 * with the packet, such as operation type and timing information.
 */
class SoftUeMetadataTag : public Tag
{
public:
    /**
     * @brief Get the type ID for this tag
     * @return TypeId
     */
    static TypeId GetTypeId (void);

    /**
     * @brief Get the instance type ID
     * @return TypeId
     */
    virtual TypeId GetInstanceTypeId (void) const override;

    /**
     * @brief Get the tag size in bytes
     * @return Tag size
     */
    virtual uint32_t GetSerializedSize (void) const override;

    /**
     * @brief Serialize the tag to a buffer
     * @param i The buffer to serialize to
     */
    virtual void Serialize (TagBuffer i) const override;

    /**
     * @brief Deserialize the tag from a buffer
     * @param i The buffer to deserialize from
     */
    virtual void Deserialize (TagBuffer i) override;

    /**
     * @brief Print the tag information
     * @param os Output stream to print to
     */
    virtual void Print (std::ostream &os) const override;

    /**
     * @brief Default constructor
     */
    SoftUeMetadataTag ();

    /**
     * @brief Constructor with operation metadata
     * @param metadata Operation metadata
     */
    SoftUeMetadataTag (const OperationMetadata& metadata);

    /**
     * @brief Get operation type
     * @return Operation type
     */
    OpType GetOperationType (void) const;

    /**
     * @brief Set operation type
     * @param opType Operation type
     */
    void SetOperationType (OpType opType);

    /**
     * @brief Get message identifier
     * @return Message identifier
     */
    uint32_t GetMessageId (void) const;

    /**
     * @brief Set message identifier
     * @param messageId Message identifier
     */
    void SetMessageId (uint32_t messageId);

    /**
     * @brief Get resource index
     * @return Resource index
     */
    uint16_t GetResourceIndex (void) const;

    /**
     * @brief Set resource index
     * @param resourceIndex Resource index
     */
    void SetResourceIndex (uint16_t resourceIndex);

    /**
     * @brief Check if this is a reliable transmission
     * @return true if reliable
     */
    bool IsReliable (void) const;

    /**
     * @brief Set reliable transmission flag
     * @param reliable Reliable transmission flag
     */
    void SetReliable (bool reliable);

private:
    OpType m_opType;              ///< Operation type
    uint32_t m_messageId;         ///< Message identifier
    uint16_t m_resourceIndex;     ///< Resource index
    bool m_reliable;              ///< Reliable transmission flag
};

/**
 * @class SoftUeTimingTag
 * @brief ns-3 packet tag for timing information
 *
 * This tag stores timing information used for performance analysis
 * and protocol timing requirements.
 */
class SoftUeTimingTag : public Tag
{
public:
    /**
     * @brief Get the type ID for this tag
     * @return TypeId
     */
    static TypeId GetTypeId (void);

    /**
     * @brief Get the instance type ID
     * @return TypeId
     */
    virtual TypeId GetInstanceTypeId (void) const override;

    /**
     * @brief Get the tag size in bytes
     * @return Tag size
     */
    virtual uint32_t GetSerializedSize (void) const override;

    /**
     * @brief Serialize the tag to a buffer
     * @param i The buffer to serialize to
     */
    virtual void Serialize (TagBuffer i) const override;

    /**
     * @brief Deserialize the tag from a buffer
     * @param i The buffer to deserialize from
     */
    virtual void Deserialize (TagBuffer i) override;

    /**
     * @brief Print the tag information
     * @param os Output stream to print to
     */
    virtual void Print (std::ostream &os) const override;

    /**
     * @brief Default constructor
     */
    SoftUeTimingTag ();

    /**
     * @brief Constructor with timestamp
     * @param timestamp Creation timestamp
     */
    SoftUeTimingTag (Time timestamp);

    /**
     * @brief Get creation timestamp
     * @return Creation timestamp
     */
    Time GetTimestamp (void) const;

    /**
     * @brief Set creation timestamp
     * @param timestamp Creation timestamp
     */
    void SetTimestamp (Time timestamp);

    /**
     * @brief Get expected delivery time
     * @return Expected delivery time
     */
    Time GetExpectedDeliveryTime (void) const;

    /**
     * @brief Set expected delivery time
     * @param time Expected delivery time
     */
    void SetExpectedDeliveryTime (Time time);

private:
    Time m_timestamp;              ///< Creation timestamp
    Time m_expectedDeliveryTime;   ///< Expected delivery time
};

} // namespace ns3

#endif /* SOFT_UE_PACKET_TAG_H */
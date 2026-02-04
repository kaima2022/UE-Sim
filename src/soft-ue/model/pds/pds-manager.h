#ifndef PDS_MANAGER_H
#define PDS_MANAGER_H

#include "ns3/object.h"
#include "ns3/ptr.h"
#include "ns3/packet.h"
#include "ns3/event-id.h"
#include "ns3/nstime.h"
#include "ns3/callback.h"
#include "ns3/traced-callback.h"
#include "pds-common.h"
#include "../ses/ses-manager.h"
#include "../pdc/pdc-base.h"
#include "../pdc/ipdc.h"

namespace ns3 {

// Forward declaration to avoid circular dependency
class SoftUeNetDevice;

/**
 * @class PdsManager
 * @brief Packet Delivery Sub-layer Manager
 *
 * This class implements the PDS layer management functionality for
 * Ultra Ethernet protocol, handling packet routing and PDC management.
 */
class PdsManager : public Object
{
public:
    /**
     * @brief PDS Manager state enumeration
     */
    enum PdsState
    {
        PDS_IDLE,        ///< PDS manager is idle and ready to process requests
        PDS_BUSY,        ///< PDS manager is busy processing requests
        PDS_ERROR        ///< PDS manager is in error state
    };
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
    PdsManager ();

    /**
     * @brief Destructor
     */
    virtual ~PdsManager ();

    /**
     * @brief Dispose of this object
     */
    virtual void DoDispose (void) override;

    /**
     * @brief Initialize the manager
     */
    void Initialize (void);

    /**
     * @brief Set the SES manager
     * @param sesManager Pointer to SES manager
     */
    void SetSesManager (Ptr<SesManager> sesManager);

    /**
     * @brief Get the SES manager
     * @return Pointer to SES manager
     */
    Ptr<SesManager> GetSesManager (void) const;

    /**
     * @brief Set the network device
     * @param device Pointer to network device
     */
    void SetNetDevice (Ptr<SoftUeNetDevice> device);

    /**
     * @brief Get the network device
     * @return Pointer to network device
     */
    Ptr<SoftUeNetDevice> GetNetDevice (void) const;

    /**
     * @brief Process SES request
     * @param request SES PDS request
     * @return true if successful
     */
    bool ProcessSesRequest (const SesPdsRequest& request);

    /**
     * @brief Process received packet
     * @param packet Received packet
     * @param sourceEndpoint Source endpoint
     * @param destEndpoint Destination endpoint
     * @return true if successful
     */
    bool ProcessReceivedPacket (Ptr<Packet> packet, uint32_t sourceEndpoint, uint32_t destEndpoint);

    /**
     * @brief Allocate a PDC
     * @param destFep Destination FEP
     * @param tc Traffic class
     * @param dm Delivery mode
     * @param nextHdr Next header
     * @param jobLandingJob Job landing job
     * @param nextJob Next job
     * @return PDC identifier
     */
    uint16_t AllocatePdc (uint32_t destFep, uint8_t tc, uint8_t dm,
                         PDSNextHeader nextHdr, uint16_t jobLandingJob, uint16_t nextJob);

    /**
     * @brief Release a PDC
     * @param pdcId PDC identifier
     * @return true if successful
     */
    bool ReleasePdc (uint16_t pdcId);

    /**
     * @brief Send packet through PDC
     * @param pdcId PDC identifier
     * @param packet Packet to send
     * @param som Start of message flag
     * @param eom End of message flag
     * @return true if successful
     */
    bool SendPacketThroughPdc (uint16_t pdcId, Ptr<Packet> packet, bool som, bool eom);

    /**
     * @brief Dispatch packet to PDC (AllocatePdc + SendPacketThroughPdc)
     * @param request SES PDS request
     * @return true if successful
     */
    bool DispatchPacket (const SesPdsRequest& request);

    /**
     * @brief Get active PDCs count
     * @return Number of active PDCs
     */
    uint32_t GetActivePdcs (void) const;

    /**
     * @brief Get total active PDC count
     * @return Total number of active PDCs
     */
    uint32_t GetTotalActivePdcCount (void) const;

    /**
     * @brief Handle PDC error
     * @param pdcId PDC identifier
     * @param error Error code
     * @param errorDetails Error details
     */
    void HandlePdcError (uint16_t pdcId, PdsErrorCode error, const std::string& errorDetails);

    // Statistics collection methods

    /**
     * @brief Get PDS statistics
     * @return Pointer to PDS statistics object
     */
    Ptr<PdsStatistics> GetStatistics (void) const;

    /**
     * @brief Reset all statistics
     */
    void ResetStatistics (void);

    /**
     * @brief Get statistics as formatted string
     * @return Formatted statistics string
     */
    std::string GetStatisticsString (void) const;

    /**
     * @brief Enable/disable statistics collection
     * @param enabled Enable statistics collection
     */
    void SetStatisticsEnabled (bool enabled);

    /**
     * @brief Check if statistics collection is enabled
     * @return true if statistics collection is enabled
     */
    bool IsStatisticsEnabled (void) const;

    /**
     * @brief Get current PDS state
     * @return Current PDS state
     */
    PdsState GetState (void) const;

    /**
     * @brief Check if PDS manager is busy
     * @return true if PDS manager is busy
     */
    bool IsBusy (void) const;

    /**
     * @brief Check if PDS manager is in error state
     * @return true if PDS manager is in error state
     */
    bool IsError (void) const;

    /**
     * @brief Reset PDS manager to idle state
     */
    void Reset (void);

private:
    /**
     * @brief Get PDC by identifier
     * @param pdcId PDC identifier
     * @return Pointer to PDC
     */
    Ptr<PdcBase> GetPdc (uint16_t pdcId) const;

    /**
     * @brief Validate SES PDS request
     * @param request SES PDS request to validate
     * @return true if request is valid
     */
    bool ValidateSesPdsRequest (const SesPdsRequest& request) const;

    /**
     * @brief Ensure a PDC exists for receive (passive creation when packet arrives with unknown pdc_id)
     * @param pdcId PDC identifier from packet header
     * @param sourceFep Source FEP (remote endpoint)
     * @return true if PDC exists or was created
     */
    bool EnsureReceivePdc (uint16_t pdcId, uint32_t sourceFep);

    // Member variables
    Ptr<SesManager> m_sesManager;                        ///< Associated SES manager
    Ptr<SoftUeNetDevice> m_netDevice;                    ///< Network device
    Ptr<PdsStatistics> m_statistics;                     ///< Statistics collection
    bool m_statisticsEnabled;                            ///< Statistics collection enabled flag

    // PDC Management
    std::map<uint16_t, Ptr<PdcBase>> m_pdcs;             ///< Active PDCs indexed by PDC ID
    uint16_t m_nextPdcId;                                ///< Next available PDC ID
    uint32_t m_maxPdcCount;                              ///< Maximum PDC count

    // Performance optimization: track free PDC IDs using a bitmap and queue
    static const uint16_t MAX_PDC_ID = 65535;             ///< Maximum PDC ID value
    std::vector<bool> m_pdcIdBitmap;                      ///< Bitmap for quick free ID lookup
    std::queue<uint16_t> m_freePdcIds;                    ///< Queue of recently freed PDC IDs

    // State management
    PdsState m_state;                                    ///< Current PDS manager state
};

} // namespace ns3

#endif /* PDS_MANAGER_H */
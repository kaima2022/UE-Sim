#ifndef RTO_TIMER_H
#define RTO_TIMER_H

#include "ns3/object.h"
#include "ns3/timer.h"
#include "ns3/event-id.h"
#include "ns3/nstime.h"

namespace ns3 {

/**
 * @class RtoTimer
 * @brief Retransmission Timeout (RTO) Timer for reliable packet delivery
 *
 * This class implements the RTO timer mechanism specified in the Ultra Ethernet
 * protocol for tracking packet retransmission timeouts.
 */
class RtoTimer : public Object
{
public:
    /**
     * @brief Timer state enumeration
     */
    enum TimerState
    {
        STOPPED,     ///< Timer is stopped
        RUNNING,     ///< Timer is running
        EXPIRED      ///< Timer has expired
    };

    /**
     * @brief Get the type ID for this class
     * @return TypeId
     */
    static TypeId GetTypeId (void);

    /**
     * @brief Constructor
     */
    RtoTimer ();

    /**
     * @brief Destructor
     */
    virtual ~RtoTimer ();

    /**
     * @brief Set the timeout duration
     * @param timeout Timeout duration
     */
    void SetTimeout (Time timeout);

    /**
     * @brief Get the current timeout duration
     * @return Timeout duration
     */
    Time GetTimeout (void) const;

    /**
     * @brief Start the timer
     */
    void Start (void);

    /**
     * @brief Restart the timer (stop and start again)
     */
    void Restart (void);

    /**
     * @brief Stop the timer
     */
    void Stop (void);

    /**
     * @brief Cancel the timer (alias for Stop)
     */
    void Cancel (void);

    /**
     * @brief Check if the timer is running
     * @return true if timer is running
     */
    bool IsRunning (void) const;

    /**
     * @brief Check if the timer has expired
     * @return true if timer has expired
     */
    bool IsExpired (void) const;

    /**
     * @brief Get the current timer state
     * @return Timer state
     */
    TimerState GetState (void) const;

    /**
     * @brief Set the timeout callback
     * @tparam MEM_PTR \deduced The type of the class member function
     * @tparam OBJ_PTR \deduced The type of the class instance pointer
     * @param memPtr The member function pointer
     * @param objPtr The class instance pointer
     */
    template <typename MEM_PTR, typename OBJ_PTR>
    void SetTimeoutCallback (MEM_PTR memPtr, OBJ_PTR objPtr);

    /**
     * @brief Set the PDC identifier for this timer
     * @param pdcId PDC identifier
     */
    void SetPdcId (uint16_t pdcId);

    /**
     * @brief Get the PDC identifier for this timer
     * @return PDC identifier
     */
    uint16_t GetPdcId (void) const;

    /**
     * @brief Set the sequence number for this timer
     * @param sequenceNumber Sequence number
     */
    void SetSequenceNumber (uint32_t sequenceNumber);

    /**
     * @brief Get the sequence number for this timer
     * @return Sequence number
     */
    uint32_t GetSequenceNumber (void) const;

private:
    Timer m_timer;                 ///< ns3 Timer implementation
    Time m_timeout;                ///< Timeout duration
    TimerState m_state;            ///< Current timer state
    bool m_expired;                ///< Timer expiration flag
    uint16_t m_pdcId;              ///< PDC identifier
    uint32_t m_sequenceNumber;     ///< Sequence number

    /**
     * @brief Internal timeout handler
     */
    void HandleTimeout (void);
};

} // namespace ns3

#endif /* RTO_TIMER_H */

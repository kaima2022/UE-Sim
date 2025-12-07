#include "rto-timer.h"
#include "ns3/log.h"
#include "ns3/simulator.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("RtoTimer");
NS_OBJECT_ENSURE_REGISTERED (RtoTimer);

TypeId
RtoTimer::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::RtoTimer")
    .SetParent<Object> ()
    .SetGroupName ("Soft-Ue")
    .AddConstructor<RtoTimer> ()
    ;
  return tid;
}

RtoTimer::RtoTimer ()
  : m_timer (Timer::CANCEL_ON_DESTROY),
    m_timeout (Seconds (1.0)),
    m_state (STOPPED),
    m_expired (false),
    m_pdcId (0),
    m_sequenceNumber (0)
{
  NS_LOG_FUNCTION (this);

  // Set up the timer
  m_timer.SetFunction (&RtoTimer::HandleTimeout, this);
}

RtoTimer::~RtoTimer ()
{
  NS_LOG_FUNCTION (this);

  if (m_state == RUNNING)
    {
      m_timer.Cancel ();
    }
}

void
RtoTimer::SetTimeout (Time timeout)
{
  NS_LOG_FUNCTION (this << timeout);
  m_timeout = timeout;
}

Time
RtoTimer::GetTimeout (void) const
{
  return m_timeout;
}

void
RtoTimer::Start (void)
{
  NS_LOG_FUNCTION (this);

  if (m_state == RUNNING)
    {
      m_timer.Cancel ();
    }

  m_expired = false;
  m_state = RUNNING;
  m_timer.Schedule (m_timeout);

  NS_LOG_DEBUG ("RTO Timer started with timeout " << m_timeout.GetMilliSeconds () << " ms");
}

void
RtoTimer::Restart (void)
{
  NS_LOG_FUNCTION (this);

  // Stop and restart
  if (m_state == RUNNING)
    {
      m_timer.Cancel ();
    }

  m_expired = false;
  m_state = RUNNING;
  m_timer.Schedule (m_timeout);

  NS_LOG_DEBUG ("RTO Timer restarted with timeout " << m_timeout.GetMilliSeconds () << " ms");
}

void
RtoTimer::Stop (void)
{
  NS_LOG_FUNCTION (this);

  if (m_state == RUNNING)
    {
      m_timer.Cancel ();
      m_state = STOPPED;
      NS_LOG_DEBUG ("RTO Timer stopped");
    }
}

void
RtoTimer::Cancel (void)
{
  NS_LOG_FUNCTION (this);
  Stop (); // Alias for Stop
}

bool
RtoTimer::IsRunning (void) const
{
  return m_state == RUNNING && !m_expired;
}

bool
RtoTimer::IsExpired (void) const
{
  return m_expired;
}

RtoTimer::TimerState
RtoTimer::GetState (void) const
{
  return m_state;
}

template <typename MEM_PTR, typename OBJ_PTR>
void
RtoTimer::SetTimeoutCallback (MEM_PTR memPtr, OBJ_PTR objPtr)
{
  NS_LOG_FUNCTION (this);
  m_timer.SetFunction (memPtr, objPtr);
}

void
RtoTimer::SetPdcId (uint16_t pdcId)
{
  NS_LOG_FUNCTION (this << pdcId);
  m_pdcId = pdcId;
}

uint16_t
RtoTimer::GetPdcId (void) const
{
  return m_pdcId;
}

void
RtoTimer::SetSequenceNumber (uint32_t sequenceNumber)
{
  NS_LOG_FUNCTION (this << sequenceNumber);
  m_sequenceNumber = sequenceNumber;
}

uint32_t
RtoTimer::GetSequenceNumber (void) const
{
  return m_sequenceNumber;
}

void
RtoTimer::HandleTimeout (void)
{
  NS_LOG_FUNCTION (this);

  m_state = EXPIRED;
  m_expired = true;

  NS_LOG_DEBUG ("RTO Timer expired for PDC " << m_pdcId << " Sequence " << m_sequenceNumber);
}

} // namespace ns3

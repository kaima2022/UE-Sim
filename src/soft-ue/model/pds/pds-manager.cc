#include "pds-manager.h"
#include "../pdc/pdc-base.h"
#include "../network/soft-ue-net-device.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/assert.h"
#include <algorithm>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("PdsManager");
NS_OBJECT_ENSURE_REGISTERED (PdsManager);

TypeId
PdsManager::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::PdsManager")
    .SetParent<Object> ()
    .SetGroupName ("Soft-Ue")
    .AddConstructor<PdsManager> ()
    ;
  return tid;
}

TypeId
PdsManager::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

PdsManager::PdsManager ()
  : m_sesManager (nullptr),
    m_netDevice (nullptr),
    m_statistics (nullptr),
    m_statisticsEnabled (true)
{
  NS_LOG_FUNCTION (this);
}

PdsManager::~PdsManager ()
{
  NS_LOG_FUNCTION (this);
}

void
PdsManager::DoDispose (void)
{
  NS_LOG_FUNCTION (this);

  // Clear components
  m_sesManager = nullptr;
  m_netDevice = nullptr;
  m_statistics = nullptr;

  Object::DoDispose ();
}

void
PdsManager::Initialize (void)
{
  NS_LOG_FUNCTION (this);

  // Create statistics object
  m_statistics = CreateObject<PdsStatistics> ();

  NS_LOG_INFO ("PDS Manager initialized");
}

void
PdsManager::SetSesManager (Ptr<SesManager> sesManager)
{
  NS_LOG_FUNCTION (this << sesManager);
  m_sesManager = sesManager;
}

Ptr<SesManager>
PdsManager::GetSesManager (void) const
{
  return m_sesManager;
}

void
PdsManager::SetNetDevice (Ptr<SoftUeNetDevice> device)
{
  NS_LOG_FUNCTION (this << device);
  m_netDevice = device;
}

Ptr<SoftUeNetDevice>
PdsManager::GetNetDevice (void) const
{
  return m_netDevice;
}

bool
PdsManager::ProcessSesRequest (const SesPdsRequest& request)
{
  NS_LOG_FUNCTION (this << "Processing SES request");

  // Validate request
  if (request.packet == nullptr)
    {
      if (m_statistics && m_statisticsEnabled)
        {
          m_statistics->IncrementErrors (PdsErrorCode::PROTOCOL_ERROR);
        }
      return false;
    }

  // Increment received packets count
  if (m_statistics && m_statisticsEnabled)
    {
      m_statistics->IncrementReceivedPackets ();
    }

  NS_LOG_DEBUG ("Processed SES request successfully");
  return true;
}

bool
PdsManager::ProcessReceivedPacket (Ptr<Packet> packet, uint32_t sourceEndpoint, uint32_t destEndpoint)
{
  NS_LOG_FUNCTION (this << "Processing received packet");

  if (!packet)
    {
      if (m_statistics && m_statisticsEnabled)
        {
          m_statistics->IncrementErrors (PdsErrorCode::INVALID_PACKET);
        }
      return false;
    }

  // Increment received packets count
  if (m_statistics && m_statisticsEnabled)
    {
      m_statistics->IncrementReceivedPackets ();
    }

  NS_LOG_DEBUG ("Processed received packet - size: " << packet->GetSize ());
  return true;
}

uint16_t
PdsManager::AllocatePdc (uint32_t destFep, uint8_t tc, uint8_t dm,
                         PDSNextHeader nextHdr, uint16_t jobLandingJob, uint16_t nextJob)
{
  NS_LOG_FUNCTION (this << "Allocating PDC for destFep=" << destFep <<
                  " tc=" << static_cast<int> (tc) << " dm=" << static_cast<int> (dm));

  // For now, return a simple sequential PDC ID
  static uint16_t nextPdcId = 1;
  uint16_t pdcId = nextPdcId++;

  if (m_statistics && m_statisticsEnabled)
    {
      m_statistics->IncrementPdcCreations ();
    }

  NS_LOG_INFO ("Successfully allocated PDC " << pdcId);
  return pdcId;
}

bool
PdsManager::ReleasePdc (uint16_t pdcId)
{
  NS_LOG_FUNCTION (this << "Releasing PDC " << pdcId);

  if (pdcId == 0)
    {
      if (m_statistics && m_statisticsEnabled)
        {
          m_statistics->IncrementErrors (PdsErrorCode::INVALID_PDC);
        }
      return false;
    }

  if (m_statistics && m_statisticsEnabled)
    {
      m_statistics->IncrementPdcDestructions ();
    }

  NS_LOG_INFO ("Released PDC " << pdcId);
  return true;
}

bool
PdsManager::SendPacketThroughPdc (uint16_t pdcId, Ptr<Packet> packet, bool som, bool eom)
{
  NS_LOG_FUNCTION (this << "Sending packet through PDC " << pdcId);

  if (pdcId == 0 || !packet)
    {
      if (m_statistics && m_statisticsEnabled)
        {
          m_statistics->IncrementErrors (PdsErrorCode::INVALID_PDC);
        }
      return false;
    }

  // Increment sent packets count
  if (m_statistics && m_statisticsEnabled)
    {
      m_statistics->IncrementSentPackets ();
    }

  NS_LOG_INFO ("Packet sent successfully through PDC " << pdcId);
  return true;
}

bool
PdsManager::DispatchPacket (const SesPdsRequest& request)
{
  NS_LOG_FUNCTION (this << "Dispatching packet to PDC");

  // Allocate a new PDC for this request
  uint16_t pdcId = AllocatePdc (request.dst_fep, request.tc, request.mode,
                               request.next_hdr, 0, 0);
  if (pdcId == 0)
    {
      if (m_statistics && m_statisticsEnabled)
        {
          m_statistics->IncrementErrors (PdsErrorCode::PDC_FULL);
        }
      return false;
    }

  // Send packet through PDC
  bool success = SendPacketThroughPdc (pdcId, request.packet, request.som, request.eom);
  if (!success)
    {
      NS_LOG_ERROR ("Failed to send packet through PDC " << pdcId);
    }

  return success;
}

void
PdsManager::HandlePdcError (uint16_t pdcId, PdsErrorCode error, const std::string& errorDetails)
{
  NS_LOG_FUNCTION (this << "Handling PDC error for PDC " << pdcId <<
                  " error=" << static_cast<int> (error) << " details=" << errorDetails);

  if (m_statistics && m_statisticsEnabled)
    {
      m_statistics->IncrementErrors (error);
    }

  NS_LOG_WARN ("PDC error handled for PDC " << pdcId << ": " << errorDetails);
}

Ptr<PdcBase>
PdsManager::GetPdc (uint16_t pdcId) const
{
  // For now, return nullptr as PDC management is simplified
  return nullptr;
}

uint32_t
PdsManager::GetActivePdcs (void) const
{
  // For now, return 0 as PDC management is simplified
  return 0;
}

uint32_t
PdsManager::GetTotalActivePdcCount (void) const
{
  // For now, return the same as GetActivePdcs
  // In a full implementation, this would count both IPDC and TPDC instances
  return GetActivePdcs ();
}

Ptr<PdsStatistics>
PdsManager::GetStatistics (void) const
{
  return m_statistics;
}

void
PdsManager::ResetStatistics (void)
{
  if (m_statistics)
    {
      m_statistics->Reset ();
    }
}

std::string
PdsManager::GetStatisticsString (void) const
{
  if (m_statistics)
    {
      return m_statistics->GetStatistics ();
    }
  return "Statistics not available";
}

void
PdsManager::SetStatisticsEnabled (bool enabled)
{
  m_statisticsEnabled = enabled;
}

bool
PdsManager::IsStatisticsEnabled (void) const
{
  return m_statisticsEnabled;
}


} // namespace ns3
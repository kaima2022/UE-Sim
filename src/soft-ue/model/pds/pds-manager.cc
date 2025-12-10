#include "pds-manager.h"
#include "../pdc/pdc-base.h"
#include "../network/soft-ue-net-device.h"
#include "../network/soft-ue-channel.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/assert.h"
#include "ns3/mac48-address.h"
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
    m_statisticsEnabled (true),
    m_nextPdcId (1),
    m_maxPdcCount (1024),
    m_pdcIdBitmap (MAX_PDC_ID + 1, false), // Initialize bitmap with all IDs free
    m_freePdcIds (),
    m_state (PDS_IDLE)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_DEBUG ("PdsManager created with optimized PDC ID allocation system");
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

  // Check state machine - reject if busy or in error state
  if (m_state == PDS_BUSY)
  {
    NS_LOG_WARN ("ProcessSesRequest rejected: PDS manager is busy");
    if (m_statistics && m_statisticsEnabled)
    {
      m_statistics->IncrementErrors (PdsErrorCode::RESOURCE_EXHAUSTED);
    }
    return false;
  }

  if (m_state == PDS_ERROR)
  {
    NS_LOG_ERROR ("ProcessSesRequest rejected: PDS manager is in error state");
    if (m_statistics && m_statisticsEnabled)
    {
      m_statistics->IncrementErrors (PdsErrorCode::INTERNAL_ERROR);
    }
    return false;
  }

  // Set busy state during processing
  m_state = PDS_BUSY;

  // Enhanced request validation
  if (!ValidateSesPdsRequest (request))
  {
    NS_LOG_ERROR ("Invalid SES PDS request received");
    if (m_statistics && m_statisticsEnabled)
    {
      m_statistics->IncrementErrors (PdsErrorCode::PROTOCOL_ERROR);
    }
    m_state = PDS_ERROR;
    return false;
  }

  // Validate network device
  if (!m_netDevice)
  {
    NS_LOG_ERROR ("Network device not available for transmission");
    if (m_statistics && m_statisticsEnabled)
    {
      m_statistics->IncrementErrors (PdsErrorCode::RESOURCE_EXHAUSTED);
    }
    m_state = PDS_ERROR;
    return false;
  }

  // Create destination address from destination FEP
  uint8_t macBytes[6];
  macBytes[0] = 0x02;
  macBytes[1] = 0x06;
  macBytes[2] = 0x00;
  macBytes[3] = 0x00;
  macBytes[4] = (request.dst_fep >> 8) & 0xFF;  // High byte of FEP
  macBytes[5] = request.dst_fep & 0xFF;         // Low byte of FEP

  Mac48Address destMacAddr;
  destMacAddr.CopyFrom (macBytes);
  Address destAddress = destMacAddr;

  NS_LOG_INFO ("Transmitting packet to FEP " << request.dst_fep
                << " at address " << destAddress);

  // Actually send the packet through the channel
  bool success = false;
  Ptr<Channel> baseChannel = m_netDevice->GetChannel ();
  if (baseChannel)
  {
    Ptr<SoftUeChannel> channel = DynamicCast<SoftUeChannel> (baseChannel);
    if (channel)
    {
      // Send through channel directly
      channel->Transmit (request.packet, m_netDevice, request.src_fep, request.dst_fep);
      success = true;
      NS_LOG_DEBUG ("Packet sent successfully through channel");
    }
    else
    {
      NS_LOG_ERROR ("Channel is not a SoftUeChannel");
      if (m_statistics && m_statisticsEnabled)
      {
        m_statistics->IncrementErrors (PdsErrorCode::PROTOCOL_ERROR);
      }
      m_state = PDS_ERROR;
      return false;
    }
  }
  else
  {
    NS_LOG_ERROR ("No channel available for transmission");
    if (m_statistics && m_statisticsEnabled)
    {
      m_statistics->IncrementErrors (PdsErrorCode::RESOURCE_EXHAUSTED);
    }
    m_state = PDS_ERROR;
    return false;
  }

  // Increment received and sent packets count
  if (m_statistics && m_statisticsEnabled)
  {
    m_statistics->IncrementReceivedPackets ();
    m_statistics->IncrementSentPackets ();
  }

  NS_LOG_DEBUG ("Processed SES request and transmitted packet successfully");

  if (success)
  {
    m_state = PDS_IDLE; // Return to idle on success
  }
  else
  {
    m_state = PDS_ERROR; // Set error on failure
  }

  return success;
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

  // Check if we have reached maximum PDC count
  if (m_pdcs.size () >= m_maxPdcCount)
  {
    NS_LOG_ERROR ("Maximum PDC count reached: " << m_maxPdcCount);
    if (m_statistics && m_statisticsEnabled)
    {
      m_statistics->IncrementErrors (PdsErrorCode::PDC_FULL);
    }
    return 0;
  }

  // Optimized PDC ID allocation - O(1) average case
  uint16_t pdcId = 0;

  // First try to use a recently freed ID from the queue
  if (!m_freePdcIds.empty ())
  {
    pdcId = m_freePdcIds.front ();
    m_freePdcIds.pop ();
    NS_LOG_DEBUG ("Reusing freed PDC ID: " << pdcId);
  }
  else
  {
    // Find next available ID using bitmap for O(1) lookup
    while (m_nextPdcId <= MAX_PDC_ID)
    {
      if (!m_pdcIdBitmap[m_nextPdcId])
      {
        pdcId = m_nextPdcId;
        m_nextPdcId++;
        if (m_nextPdcId == 0) m_nextPdcId = 1; // Wrap around, skip 0
        break;
      }
      m_nextPdcId++;
      if (m_nextPdcId == 0) m_nextPdcId = 1; // Wrap around
    }

    // If we wrapped around and still didn't find a free ID, check if any IDs are available
    if (pdcId == 0 && m_pdcs.size () < m_maxPdcCount)
    {
      // Search from beginning for any free ID
      for (uint16_t id = 1; id <= MAX_PDC_ID && id < m_maxPdcCount; ++id)
      {
        if (!m_pdcIdBitmap[id])
        {
          pdcId = id;
          m_nextPdcId = id + 1;
          if (m_nextPdcId == 0) m_nextPdcId = 1;
          break;
        }
      }
    }

    if (pdcId == 0)
    {
      NS_LOG_ERROR ("No free PDC IDs available (max: " << m_maxPdcCount << ")");
      if (m_statistics && m_statisticsEnabled)
      {
        m_statistics->IncrementErrors (PdsErrorCode::PDC_FULL);
      }
      return 0;
    }
  }

  // Mark the ID as used in bitmap
  m_pdcIdBitmap[pdcId] = true;

  // Create IPDC (unreliable) for now - could be made configurable
  Ptr<Ipdc> pdc = Create<Ipdc> ();
  if (!pdc)
  {
    NS_LOG_ERROR ("Failed to create PDC");
    return 0;
  }

  // Configure PDC
  pdc->SetPdcId (pdcId);
  pdc->SetRemoteFep (destFep);
  // TODO: Add TrafficClass and DeliveryMode setters to PDC base class

  // Add to PDC container
  m_pdcs[pdcId] = pdc;
  m_nextPdcId = pdcId + 1;

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

  // Find and remove PDC from container
  auto it = m_pdcs.find (pdcId);
  if (it == m_pdcs.end ())
  {
    NS_LOG_WARN ("PDC " << pdcId << " not found for release");
    return false;
  }

  // Remove PDC from container and mark ID as available for reuse
  uint16_t freedPdcId = pdcId;
  m_pdcs.erase (it);

  // Mark ID as free in bitmap and add to reuse queue for O(1) allocation
  if (freedPdcId > 0 && freedPdcId <= MAX_PDC_ID)
  {
    m_pdcIdBitmap[freedPdcId] = false;
    m_freePdcIds.push (freedPdcId);
    NS_LOG_DEBUG ("PDC ID " << freedPdcId << " marked as available for reuse");
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

  // Get PDC from container
  Ptr<PdcBase> pdc = GetPdc (pdcId);
  if (!pdc)
  {
    NS_LOG_ERROR ("PDC " << pdcId << " not found");
    if (m_statistics && m_statisticsEnabled)
    {
      m_statistics->IncrementErrors (PdsErrorCode::INVALID_PDC);
    }
    return false;
  }

  // Send packet through PDC
  bool success = pdc->SendPacket (packet, som, eom);
  if (!success)
  {
    NS_LOG_ERROR ("Failed to send packet through PDC " << pdcId);
    if (m_statistics && m_statisticsEnabled)
    {
      m_statistics->IncrementErrors (PdsErrorCode::PROTOCOL_ERROR);
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
  auto it = m_pdcs.find (pdcId);
  if (it != m_pdcs.end ())
  {
    return it->second;
  }
  return nullptr;
}

uint32_t
PdsManager::GetActivePdcs (void) const
{
  return m_pdcs.size ();
}

uint32_t
PdsManager::GetTotalActivePdcCount (void) const
{
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

PdsManager::PdsState
PdsManager::GetState (void) const
{
  return m_state;
}

bool
PdsManager::IsBusy (void) const
{
  return m_state == PDS_BUSY;
}

bool
PdsManager::IsError (void) const
{
  return m_state == PDS_ERROR;
}

void
PdsManager::Reset (void)
{
  NS_LOG_FUNCTION (this);

  // Reset state
  m_state = PDS_IDLE;

  NS_LOG_INFO ("PdsManager reset to IDLE state");
}

bool
PdsManager::ValidateSesPdsRequest (const SesPdsRequest& request) const
{
  NS_LOG_FUNCTION (this);

  // Check if packet exists
  if (!request.packet)
  {
    NS_LOG_WARN ("SES PDS request has null packet");
    return false;
  }

  // Validate packet size
  if (request.packet->GetSize () == 0)
  {
    NS_LOG_WARN ("SES PDS request has empty packet");
    return false;
  }

  // Validate packet size against reasonable limit (e.g., 64KB)
  const uint32_t MAX_PACKET_SIZE = 65536;
  if (request.packet->GetSize () > MAX_PACKET_SIZE)
  {
    NS_LOG_WARN ("SES PDS request packet size " << request.packet->GetSize ()
                 << " exceeds maximum allowed size " << MAX_PACKET_SIZE);
    return false;
  }

  // Validate destination FEP address
  if (request.dst_fep == 0)
  {
    NS_LOG_WARN ("SES PDS request has invalid destination FEP address (0)");
    return false;
  }

  // Validate source FEP address
  if (request.src_fep == 0)
  {
    NS_LOG_WARN ("SES PDS request has invalid source FEP address (0)");
    return false;
  }

  // Validate mode field
  if (request.mode > 7)  // Assuming 3-bit mode field
  {
    NS_LOG_WARN ("SES PDS request has invalid mode: " << request.mode);
    return false;
  }

  // Validate traffic class
  if (request.tc > 255)  // 8-bit traffic class
  {
    NS_LOG_WARN ("SES PDS request has invalid traffic class: " << request.tc);
    return false;
  }

  // Validate packet length consistency
  if (request.pkt_len != request.packet->GetSize ())
  {
    NS_LOG_WARN ("SES PDS request packet length mismatch: header says "
                 << request.pkt_len << ", actual packet size is "
                 << request.packet->GetSize ());
    return false;
  }

  return true;
}

} // namespace ns3
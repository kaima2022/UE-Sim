#include "soft-ue-channel.h"
#include "soft-ue-net-device.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/packet.h"
#include "ns3/simulator.h"
#include "ns3/mac48-address.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("SoftUeChannel");
NS_OBJECT_ENSURE_REGISTERED (SoftUeChannel);

TypeId
SoftUeChannel::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SoftUeChannel")
    .SetParent<Channel> ()
    .SetGroupName ("Soft-Ue")
    .AddConstructor<SoftUeChannel> ()
    .AddAttribute ("DataRate",
                   "The data rate of the channel",
                   DataRateValue (DataRate ("1Gbps")),
                   MakeDataRateAccessor (&SoftUeChannel::SetDataRate,
                                            &SoftUeChannel::GetDataRate),
                   MakeDataRateChecker ())
    .AddAttribute ("Delay",
                   "The propagation delay of the channel (Data center level latency)",
                   TimeValue (NanoSeconds (100)), // 100ns - Data center internal transmission delay
                   MakeTimeAccessor (&SoftUeChannel::SetDelay,
                                        &SoftUeChannel::GetDelay),
                   MakeTimeChecker ())
    .AddTraceSource ("Tx",
                     "Trace source for packet transmission",
                     MakeTraceSourceAccessor (&SoftUeChannel::m_txTrace),
                     "ns3::Packet::TracedCallback")
    .AddTraceSource ("Rx",
                     "Trace source for packet reception",
                     MakeTraceSourceAccessor (&SoftUeChannel::m_rxTrace),
                     "ns3::Packet::TracedCallback")
    ;
  return tid;
}

SoftUeChannel::SoftUeChannel ()
  : m_dataRate ("1Gbps"),
    m_delay (NanoSeconds (100)) // Data center internal transmission delay
{
  NS_LOG_FUNCTION (this);
}

SoftUeChannel::~SoftUeChannel ()
{
  NS_LOG_FUNCTION (this);
}

void
SoftUeChannel::DoDispose (void)
{
  NS_LOG_FUNCTION (this);

  m_devices.clear ();
  Channel::DoDispose ();
}

void
SoftUeChannel::SetDataRate (DataRate dataRate)
{
  NS_LOG_FUNCTION (this << dataRate);
  m_dataRate = dataRate;
}

DataRate
SoftUeChannel::GetDataRate (void) const
{
  return m_dataRate;
}

void
SoftUeChannel::SetDelay (Time delay)
{
  NS_LOG_FUNCTION (this << delay);
  m_delay = delay;
}

Time
SoftUeChannel::GetDelay (void) const
{
  return m_delay;
}

void
SoftUeChannel::SetPropagationDelay (Time delay)
{
  NS_LOG_FUNCTION (this << delay);
  SetDelay (delay);
}

Time
SoftUeChannel::GetPropagationDelay (void) const
{
  return GetDelay ();
}

void
SoftUeChannel::Attach (Ptr<NetDevice> device)
{
  NS_LOG_FUNCTION (this << device);

  // Check if device is already attached
  for (uint32_t i = 0; i < m_devices.size (); ++i)
    {
      if (m_devices[i] == device)
        {
          NS_LOG_DEBUG ("Device already attached to channel");
          return;
        }
    }

  // Add device to channel with limit check
  static const std::size_t MAX_DEVICES_PER_CHANNEL = 1000; // Reasonable limit
  if (m_devices.size () >= MAX_DEVICES_PER_CHANNEL)
  {
    NS_LOG_ERROR ("Cannot attach device: maximum device count per channel reached ("
                  << MAX_DEVICES_PER_CHANNEL << ")");
    return;
  }

  m_devices.push_back (device);
  NS_LOG_INFO ("Attached device " << device << " to Soft-Ue channel (total devices: "
               << m_devices.size () << ")");
}

bool
SoftUeChannel::IsAttached (Ptr<NetDevice> device) const
{
  for (uint32_t i = 0; i < m_devices.size (); ++i)
    {
      if (m_devices[i] == device)
        {
          return true;
        }
    }
  return false;
}

std::size_t
SoftUeChannel::GetNDevices (void) const
{
  return m_devices.size ();
}

Ptr<NetDevice>
SoftUeChannel::GetDevice (std::size_t i) const
{
  NS_ASSERT (i < m_devices.size ());
  return m_devices[i];
}

std::size_t
SoftUeChannel::GetDevice (Ptr<NetDevice> device) const
{
  for (std::size_t i = 0; i < m_devices.size (); ++i)
    {
      if (m_devices[i] == device)
        {
          return i;
        }
    }
  return m_devices.size (); // Not found
}

void
SoftUeChannel::Connect (NetDeviceContainer devices)
{
  NS_LOG_FUNCTION (this << devices.GetN ());

  for (uint32_t i = 0; i < devices.GetN (); ++i)
    {
      Attach (devices.Get (i));
    }
}

void
SoftUeChannel::Transmit (Ptr<Packet> packet, Ptr<NetDevice> src, uint32_t sourceFep, uint32_t destFep)
{
  NS_LOG_FUNCTION (this << packet << src << sourceFep << destFep);

  if (!packet || !src)
    {
      NS_LOG_WARN ("Invalid packet or source device");
      return;
    }

  // Trace transmission
  NS_LOG_INFO ("[UEC-E2E] [Channel] ⑤ 信道 Transmit: FEP " << sourceFep << " → FEP " << destFep
               << " size=" << packet->GetSize () << " B (经延迟后送达对端)");
  m_txTrace (packet, sourceFep, destFep);

  // Transmit to all destination devices
  for (uint32_t i = 0; i < m_devices.size (); ++i)
    {
      Ptr<NetDevice> dest = m_devices[i];
      if (dest != src) // Don't transmit back to source
        {
          // For unicast transmission, only send to specific destination
          if (destFep == 0 || destFep == GetDestinationFepForDevice (dest))
            {
              // Schedule packet reception with propagation delay
              Time receiveDelay = m_delay + CalculateTransmissionTime (packet);
              ScheduleReceive (packet->Copy (), dest, sourceFep, destFep, receiveDelay);
            }
        }
    }
}

void
SoftUeChannel::ScheduleReceive (Ptr<Packet> packet, Ptr<NetDevice> dest, uint32_t sourceFep, uint32_t destFep, Time delay)
{
  NS_LOG_FUNCTION (this << packet << dest << sourceFep << destFep << delay);

  // Schedule packet reception
  Simulator::Schedule (delay, &SoftUeChannel::ReceivePacket, this, packet, dest, sourceFep, destFep);
}

void
SoftUeChannel::ReceivePacket (Ptr<Packet> packet, Ptr<NetDevice> dest, uint32_t sourceFep, uint32_t destFep)
{
  NS_LOG_FUNCTION (this << packet << dest << sourceFep << destFep);

  // Check if destination device is a SoftUeNetDevice
  Ptr<SoftUeNetDevice> softUeDevice = DynamicCast<SoftUeNetDevice> (dest);
  if (!softUeDevice)
    {
      NS_LOG_DEBUG ("Destination device is not a SoftUeNetDevice");
      return;
    }

  // Trace reception
  NS_LOG_INFO ("[UEC-E2E] [Channel] ⑥ 信道 ReceivePacket: FEP " << sourceFep << " → FEP " << destFep
               << " 送达设备，设备即将 ReceivePacket");
  m_rxTrace (packet, sourceFep, destFep);

  // Forward packet to device for processing
  softUeDevice->ReceivePacket (packet, sourceFep, destFep);
}

Time
SoftUeChannel::CalculateTransmissionTime (Ptr<Packet> packet) const
{
  if (!packet)
    {
      return Seconds (0);
    }

  // Calculate transmission time based on data rate and packet size
  uint32_t packetSize = packet->GetSize () * 8; // Convert to bits
  double dataRateBps = m_dataRate.GetBitRate ();

  if (dataRateBps > 0)
    {
      return Seconds (packetSize / dataRateBps);
    }
  else
    {
      return Seconds (0); // Instantaneous transmission
    }
}

uint32_t
SoftUeChannel::GetDestinationFepForDevice (Ptr<NetDevice> device) const
{
  // For SoftUeNetDevice, extract the FEP ID from the MAC address
  Ptr<SoftUeNetDevice> softUeDevice = DynamicCast<SoftUeNetDevice> (device);
  if (softUeDevice)
    {
      SoftUeConfig config = softUeDevice->GetConfiguration ();
      return config.localFep;
    }

  // For other devices, use device index + 1 as default
  return GetDevice (device) + 1;
}

} // namespace ns3

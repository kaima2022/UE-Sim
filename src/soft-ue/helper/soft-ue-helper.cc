#include "soft-ue-helper.h"
#include "../model/network/soft-ue-net-device.h"
#include "../model/network/soft-ue-channel.h"
#include "ns3/log.h"
#include "ns3/node.h"
#include "ns3/names.h"
#include "ns3/pointer.h"
#include "ns3/mac48-address.h"
#include "ns3/object-factory.h"
#include "ns3/boolean.h"
#include "ns3/uinteger.h"
#include "ns3/integer.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("SoftUeHelper");

SoftUeHelper::SoftUeHelper ()
{
  NS_LOG_FUNCTION (this);

  // Set default device factory
  m_deviceFactory.SetTypeId ("ns3::SoftUeNetDevice");

  // Set default device attributes
  m_deviceFactory.Set ("EnableStatistics", BooleanValue (true));
  m_deviceFactory.Set ("MaxPdcCount", UintegerValue (256));
  m_deviceFactory.Set ("ProcessingInterval", TimeValue (MilliSeconds (1)));
  m_deviceFactory.Set ("MaxPacketSize", UintegerValue (1500));
}

NetDeviceContainer
SoftUeHelper::Install (NodeContainer nodes)
{
  NS_LOG_FUNCTION (this << nodes.GetN ());

  NetDeviceContainer devices;

  // Create and install Soft-Ue channels
  Ptr<SoftUeChannel> channel = CreateObject<SoftUeChannel> ();

  // Install devices on each node
  for (uint32_t i = 0; i < nodes.GetN (); ++i)
    {
      Ptr<Node> node = nodes.Get (i);
      Ptr<SoftUeNetDevice> device = m_deviceFactory.Create<SoftUeNetDevice> ();

      // Set device attributes
      SoftUeConfig config;
      config.address = Mac48Address::Allocate ();
      config.localFep = i + 1;  // Use node index + 1 as FEP ID

      // Initialize the device
      device->Initialize (config);

      // Connect device to channel
      device->SetChannel (channel);

      // Add device to node
      node->AddDevice (device);

      devices.Add (device);

      NS_LOG_INFO ("Installed Soft-Ue device on node " << i <<
                   " with FEP ID " << config.localFep <<
                   " and MAC address " << config.address);
    }

  // Connect channel to devices
  channel->Connect (devices);

  NS_LOG_INFO ("Installed " << devices.GetN () << " Soft-Ue devices on " << nodes.GetN () << " nodes");
  return devices;
}

void
SoftUeHelper::SetDeviceAttribute (std::string name, const AttributeValue& value)
{
  NS_LOG_FUNCTION (this << name << &value);
  m_deviceFactory.Set (name, value);
}

} // namespace ns3

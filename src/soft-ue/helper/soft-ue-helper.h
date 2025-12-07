#ifndef SOFT_UE_HELPER_H
#define SOFT_UE_HELPER_H

#include <ns3/object-factory.h>
#include <ns3/node-container.h>
#include <ns3/net-device-container.h>

namespace ns3 {

/**
 * @brief Soft-UE Helper class
 */
class SoftUeHelper
{
public:
    /**
     * Default constructor
     */
    SoftUeHelper ();

    /**
     * Install Soft-UE devices on nodes
     * @param nodes Node container
     * @return NetDeviceContainer with installed devices
     */
    NetDeviceContainer Install (NodeContainer nodes);

    /**
     * Install Soft-UE device on a single node
     * @param node Single node
     * @return NetDeviceContainer with installed device
     */
    NetDeviceContainer Install (Ptr<Node> node);

    /**
     * Set an attribute on the devices to be created
     * @param name Attribute name
     * @param value Attribute value
     */
    void SetDeviceAttribute (std::string name, const AttributeValue& value);

private:
    ObjectFactory m_deviceFactory; ///< Device factory
};

} // namespace ns3

#endif /* SOFT_UE_HELPER_H */
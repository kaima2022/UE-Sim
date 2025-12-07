=============================================
soft-ue API Documentation
=============================================

.. module:: soft-ue

.. highlight:: cpp

Ultra Ethernet (Soft-UE) Protocol Implementation for ns-3
==========================================================

This module provides a complete implementation of the Ultra Ethernet (Soft-UE) protocol
for the ns-3 network simulator. The implementation follows the Ultra Ethernet
Specification and includes the following layers:

* **SES (Semantic Sub-layer)**: Endpoint addressing, authorization, message types
* **PDS (Packet Delivery Sub-layer)**: Packet reception and dispatch to PDCs
* **PDC (Packet Delivery Context)**: Reliable and unreliable packet delivery contexts

The module is designed to be used with ns-3's discrete event simulation framework
and provides both simulation capabilities and performance analysis tools.

Main Classes
------------

The following are the main classes provided by the soft-ue module:

.. doxygenclass:: ns3::SoftUeHelper
   :members:

.. doxygenclass:: ns3::SoftUeNetDevice
   :members:

.. doxygenclass:: ns3::SesManager
   :members:

.. doxygenclass:: ns3::PdsManager
   :members:

.. doxygenclass:: ns3::PdcBase
   :members:

.. doxygenclass:: ns3::Ipdc
   :members:

.. doxygenclass:: ns3::Tpdc
   :members:

Usage Examples
---------------

Basic Soft-UE Node Setup
~~~~~~~~~~~~~~~~~~~~~~~~

::

    // Create soft-ue helper
    SoftUeHelper softUeHelper;

    // Create nodes
    NodeContainer nodes;
    nodes.Create(2);

    // Install soft-ue devices on nodes
    NetDeviceContainer devices = softUeHelper.Install(nodes);

    // Install internet stack
    InternetStackHelper internet;
    internet.Install(nodes);

    // Assign IP addresses
    Ipv4AddressHelper ipv4;
    ipv4.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer interfaces = ipv4.Assign(devices);

SES Layer Communication Example
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    // Get SES manager from net device
    Ptr<SoftUeNetDevice> softUeDevice = DynamicCast<SoftUeNetDevice>(devices.Get(0));
    Ptr<SesManager> sesManager = softUeDevice->GetSesManager();

    // Create operation metadata for sending
    OperationMetadata metadata;
    metadata.op_type = OpType::SEND;
    metadata.job_id = 1;
    metadata.t_pid_on_fep = 2;  // target PID
    metadata.payload.start_addr = 0x1000;
    metadata.payload.length = 1024;

    // Send packet through SES layer
    sesManager->ProcessSendRequest(metadata);

Testing and Validation
----------------------

The soft-ue module includes comprehensive test suites:

* Unit tests for individual components (SES, PDS, PDC)
* Integration tests for end-to-end functionality
* Performance benchmarks for scalability

Run tests using::

    ./ns3 run "soft-ue --test"

Performance Considerations
--------------------------

When using the soft-ue module in large-scale simulations:

1. **Memory Usage**: Monitor PDC allocation limits (default: 512 PDCs per type)
2. **Processing Overhead**: SES layer validation adds computational cost
3. **Scalability**: Use appropriate node counts and traffic patterns

For performance tuning, consider adjusting these parameters:

* ``MAX_PDC``: Maximum PDCs per type
* ``Base_RTO``: Retransmission timeout
* ``MAX_MTU``: Maximum transmission unit

References
----------

* Ultra Ethernet Specification: https://ultraethernet.org/
* ns-3 Documentation: https://www.nsnam.org/documentation/
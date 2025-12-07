/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Soft-UE Quick Integration Test
 *
 * This test verifies that the Soft-UE module can be properly instantiated
 * and basic functionality works as expected.
 */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/soft-ue-helper.h"

using namespace ns3;

int
main (int argc, char *argv[])
{
  std::cout << "Soft-UE Integration Test" << std::endl;
  std::cout << "=========================" << std::endl;

  try
    {
      // Create SoftUeHelper
      SoftUeHelper helper;

      // Set some device attributes for testing
      helper.SetDeviceAttribute ("EnableStatistics", BooleanValue (true));
      helper.SetDeviceAttribute ("MaxPdcCount", UintegerValue (256));
      helper.SetDeviceAttribute ("ProcessingInterval", TimeValue (MilliSeconds (2)));
      helper.SetDeviceAttribute ("MaxPacketSize", UintegerValue (1200));

      std::cout << "Soft-UE Helper created and configured successfully" << std::endl;

      // Create nodes
      NodeContainer nodes;
      nodes.Create (2);

      std::cout << "Created " << nodes.GetN () << " nodes" << std::endl;

      // Install Soft-UE devices
      NetDeviceContainer devices = helper.Install (nodes);
      std::cout << "Successfully installed " << devices.GetN () << " Soft-UE devices" << std::endl;

      // Test basic device functionality
      for (uint32_t i = 0; i < devices.GetN (); ++i)
        {
          Ptr<NetDevice> device = devices.Get (i);
          std::cout << "Device " << i << " - MAC: " << device->GetAddress () << std::endl;
          std::cout << "Device " << i << " - MTU: " << device->GetMtu () << " bytes" << std::endl;
          std::cout << "Device " << i << " - Link Up: " << (device->IsLinkUp () ? "YES" : "NO") << std::endl;
        }

      std::cout << "\nSoft-UE Integration Test: PASSED" << std::endl;
      std::cout << "All components initialized successfully!" << std::endl;
    }
  catch (const std::exception& e)
    {
      std::cout << "ERROR: " << e.what () << std::endl;
      return 1;
    }

  return 0;
}
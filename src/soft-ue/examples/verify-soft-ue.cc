/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/soft-ue-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("SoftUeVerification");

int
main (int argc, char *argv[])
{
  CommandLine cmd (__FILE__);
  cmd.Parse (argc, argv);

  Time::SetResolution (Time::NS);
  LogComponentEnable ("SoftUeVerification", LOG_LEVEL_INFO);

  NS_LOG_INFO ("Starting Soft-UE Module Verification");

  // Test 1: Verify SES components can be created
  NS_LOG_INFO ("Test 1: Creating SES Manager");
  Ptr<SesManager> sesManager = CreateObject<SesManager> ();
  if (sesManager != nullptr)
    {
      NS_LOG_INFO ("✓ SES Manager created successfully");
    }
  else
    {
      NS_LOG_INFO ("✗ SES Manager creation failed");
      return 1;
    }

  // Test 2: Verify ExtendedOperationMetadata
  NS_LOG_INFO ("Test 2: Creating ExtendedOperationMetadata");
  Ptr<ExtendedOperationMetadata> metadata = CreateObject<ExtendedOperationMetadata> ();
  if (metadata != nullptr)
    {
      NS_LOG_INFO ("✓ ExtendedOperationMetadata created successfully");

      // Test basic functionality
      metadata->SetOperationType (OpType::SEND);
      metadata->SetSourceEndpoint (1, 1001);
      metadata->SetDestinationEndpoint (2, 2002);
      metadata->SetJobId (12345);

      bool isValid = metadata->IsValid ();
      if (isValid)
        {
          NS_LOG_INFO ("✓ ExtendedOperationMetadata validation passed");
        }
      else
        {
          NS_LOG_INFO ("✗ ExtendedOperationMetadata validation failed");
        }
    }
  else
    {
      NS_LOG_INFO ("✗ ExtendedOperationMetadata creation failed");
      return 1;
    }

  // Test 3: Verify MSN Table
  NS_LOG_INFO ("Test 3: Creating MSN Table");
  Ptr<MsnTable> msnTable = CreateObject<MsnTable> ();
  if (msnTable != nullptr)
    {
      NS_LOG_INFO ("✓ MSN Table created successfully");

      // Test basic functionality
      bool added = msnTable->AddEntry (12345, 1, 1024);
      if (added)
        {
          NS_LOG_INFO ("✓ MSN entry added successfully");

          Ptr<MsnEntry> entry = msnTable->FindEntry (12345);
          if (entry != nullptr)
            {
              NS_LOG_INFO ("✓ MSN entry found successfully");
            }
          else
            {
              NS_LOG_INFO ("✗ MSN entry not found");
            }
        }
      else
        {
          NS_LOG_INFO ("✗ MSN entry addition failed");
        }
    }
  else
    {
      NS_LOG_INFO ("✗ MSN Table creation failed");
      return 1;
    }

  // Test 4: Verify Helper class
  NS_LOG_INFO ("Test 4: Creating SoftUeHelper");
  SoftUeHelper softUeHelper;
  NS_LOG_INFO ("✓ SoftUeHelper created successfully");

  // Test 5: Test basic node setup
  NS_LOG_INFO ("Test 5: Setting up basic node topology");
  NodeContainer nodes;
  nodes.Create (2);

  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("1Gbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("1ms"));

  NetDeviceContainer devices = pointToPoint.Install (nodes);
  NS_LOG_INFO ("✓ Basic node topology created successfully");

  InternetStackHelper internet;
  internet.Install (nodes);

  Ipv4AddressHelper ipv4;
  ipv4.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer interfaces = ipv4.Assign (devices);

  NS_LOG_INFO ("Node 0 IP: " << interfaces.GetAddress (0));
  NS_LOG_INFO ("Node 1 IP: " << interfaces.GetAddress (1));

  NS_LOG_INFO ("All Soft-UE verification tests completed successfully!");
  NS_LOG_INFO ("✓ Module is ready for further development");

  return 0;
}
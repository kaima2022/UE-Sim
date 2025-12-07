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
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/soft-ue-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("SoftUeDemo");

int
main(int argc, char* argv[])
{
    CommandLine cmd(__FILE__);
    cmd.Parse(argc, argv);

    Time::SetResolution(Time::NS);
    LogComponentEnable("SoftUeDemo", LOG_LEVEL_INFO);
    LogComponentEnable("SesManager", LOG_LEVEL_INFO);
    LogComponentEnable("PdsManager", LOG_LEVEL_INFO);

    NS_LOG_INFO("Soft-UE Demo: Ultra Ethernet Protocol Simulation");

    // Create Soft-UE components
    Ptr<SesManager> sesManager = CreateObject<SesManager>();
    Ptr<PdsManager> pdsManager = CreateObject<PdsManager>();

    NS_LOG_INFO("Created SES Manager and PDS Manager");

    // Simulate basic packet flow
    for (int i = 0; i < 5; ++i)
    {
        NS_LOG_INFO("Processing packet " << i + 1);

        // SES layer processing
        if (sesManager)
        {
            NS_LOG_INFO("  SES Manager: Processing packet metadata");
        }

        // PDS layer processing
        if (pdsManager)
        {
            NS_LOG_INFO("  PDS Manager: Managing packet distribution");
        }

        Simulator::Schedule(MilliSeconds(100 * i), []() {
            NS_LOG_INFO("Packet transmission completed");
        });
    }

    NS_LOG_INFO("Soft-UE Demo initialization complete");
    NS_LOG_INFO("Starting simulation...");

    Simulator::Stop(Seconds(2.0));
    Simulator::Run();
    Simulator::Destroy();

    NS_LOG_INFO("Soft-UE Demo completed successfully");

    return 0;
}
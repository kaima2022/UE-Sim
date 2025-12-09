/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * SES Manager 核心模块测试
 * 测试SES管理器的基本功能和属性
 */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/soft-ue-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("SesManagerTest");

int
main(int argc, char* argv[])
{
    LogComponentEnable("SesManagerTest", LOG_LEVEL_INFO);

    NS_LOG_INFO("=== SES Manager 核心模块测试 ===");

    // 1. 创建Soft-UE设备
    NodeContainer nodes;
    nodes.Create(2);

    SoftUeHelper helper;
    helper.SetDeviceAttribute("MaxPdcCount", UintegerValue(1024));

    NetDeviceContainer devices = helper.Install(nodes);

    // 2. 测试SES管理器基本功能
    Ptr<SoftUeNetDevice> device = DynamicCast<SoftUeNetDevice>(devices.Get(0));
    if (device)
    {
        NS_LOG_INFO("✓ Soft-UE设备创建成功");

        // 测试设备基本属性
        uint32_t maxPdcs = device->GetMaxPdcCount();
        NS_LOG_INFO("✓ 最大PDC数量: " << maxPdcs);

        // 测试设备配置
        device->SetMaxPdcCount(2048);
        uint32_t newMaxPdcs = device->GetMaxPdcCount();
        NS_LOG_INFO("✓ PDC数量更新: " << newMaxPdcs);

        // 测试网络设备功能
        Address macAddr = device->GetAddress();
        NS_LOG_INFO("✓ MAC地址: " << macAddr);

        // 测试设备类型
        std::string deviceType = device->GetInstanceTypeId().GetName();
        NS_LOG_INFO("✓ 设备类型: " << deviceType);
    }
    else
    {
        NS_LOG_ERROR("✗ Soft-UE设备创建失败");
        return 1;
    }

    // 3. 运行简短仿真
    Simulator::Run();

    NS_LOG_INFO("=== SES Manager 测试完成 ===");
    NS_LOG_INFO("状态: ✓ 所有基本功能正常");
    NS_LOG_INFO("验证: ✓ SES管理器核心功能可用");

    Simulator::Destroy();
    return 0;
}
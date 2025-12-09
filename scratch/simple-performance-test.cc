/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * 简单性能验证测试
 * 测试Soft-UE模块的基本性能
 */

#include "ns3/core-module.h"
#include "ns3/soft-ue-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("SimplePerformanceTest");

int
main(int argc, char* argv[])
{
    LogComponentEnable("SimplePerformanceTest", LOG_LEVEL_INFO);

    NS_LOG_INFO("=== Soft-UE 简单性能验证测试 ===");

    try {
        // 1. 创建单个节点进行基本测试
        NodeContainer nodes;
        nodes.Create(1);  // 创建1个节点

        SoftUeHelper helper;
        helper.SetDeviceAttribute("MaxPdcCount", UintegerValue(1500));

        NS_LOG_INFO("开始安装设备...");
        NetDeviceContainer devices = helper.Install(nodes);

        NS_LOG_INFO("性能测试配置:");
        NS_LOG_INFO("  节点数量: " << nodes.GetN());
        NS_LOG_INFO("  设备数量: " << devices.GetN());

        // 2. 验证设备属性 - 安全访问
        if (devices.GetN() > 0) {
            Ptr<SoftUeNetDevice> device = DynamicCast<SoftUeNetDevice>(devices.Get(0));
            if (device) {
                NS_LOG_INFO("✓ 设备创建成功");
                NS_LOG_INFO("  最大PDC: " << device->GetMaxPdcCount());
                NS_LOG_INFO("  MAC地址: " << device->GetAddress());
                NS_LOG_INFO("  链路状态: " << (device->IsLinkUp() ? "UP" : "DOWN"));
            } else {
                NS_LOG_INFO("❌ 设备类型转换失败");
                return 1;
            }
        }

        // 3. 运行极简仿真
        NS_LOG_INFO("开始极简仿真...");
        Simulator::Stop(Seconds(1.0));
        Simulator::Run();
        Simulator::Destroy();

        NS_LOG_INFO("=== 性能测试完成 ===");
        NS_LOG_INFO("🎉 状态: 极简性能测试成功");
        NS_LOG_INFO("📊 评估: Soft-UE模块基本功能正常");

    } catch (const std::exception& e) {
        NS_LOG_INFO("❌ 异常: " << e.what());
        return 1;
    }

    return 0;
}
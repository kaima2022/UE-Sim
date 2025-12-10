/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * 简化的多节点演示测试
 */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/soft-ue-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("SimpleDemoTest");

int main(int argc, char* argv[]) {
    // 禁用日志简化输出
    LogComponentEnable("SimpleDemoTest", LOG_LEVEL_INFO);

    CommandLine cmd(__FILE__);
    uint32_t nNodes = 3;
    cmd.AddValue("nNodes", "节点数量", nNodes);
    cmd.Parse(argc, argv);

    NS_LOG_INFO("开始简单演示测试，节点数: " << nNodes);

    try {
        // 创建节点
        NodeContainer nodes;
        nodes.Create(nNodes);

        // 安装Soft-UE设备
        SoftUeHelper helper;
        NetDeviceContainer devices = helper.Install(nodes);

        NS_LOG_INFO("成功安装 " << devices.GetN() << " 个Soft-UE设备");

        // 验证设备
        uint32_t validDevices = 0;
        for (uint32_t i = 0; i < devices.GetN(); ++i) {
            Ptr<SoftUeNetDevice> device = DynamicCast<SoftUeNetDevice>(devices.Get(i));
            if (device) {
                validDevices++;
            }
        }

        NS_LOG_INFO("验证结果: " << validDevices << "/" << devices.GetN() << " 设备有效");

        if (validDevices == devices.GetN()) {
            NS_LOG_INFO("测试通过！");
            return 0;
        } else {
            NS_LOG_ERROR("测试失败！");
            return 1;
        }

    } catch (const std::exception& e) {
        NS_LOG_ERROR("测试异常: " << e.what());
        return 1;
    }
}
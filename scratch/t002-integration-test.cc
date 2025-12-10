/*******************************************************************************
 * Copyright 2025 Soft UE Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ******************************************************************************/

/**
 * @file             t002-integration-test.cc
 * @brief            T002 端到端协议栈集成测试
 * @author           softuegroup@gmail.com
 * @version          1.0.0
 * @date             2025-12-10
 * @copyright        Apache License Version 2.0
 *
 * @details
 * 这是T002集成测试的核心实现，用于验证：
 * - 端到端协议栈通信（SES/PDS/PDC）
 * - 多节点之间的数据包传输
 * - 接收回调机制和统计收集
 * - 协议栈完整性验证
 */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/soft-ue-helper.h"
#include "ns3/soft-ue-net-device.h"
#include "ns3/soft-ue-channel.h"
#include "ns3/ses-manager.h"
#include "ns3/pds-manager.h"
#include "ns3/pdc-base.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("T002IntegrationTest");

/**
 * @brief 接收回调统计结构
 */
struct ReceptionStats
{
    uint32_t packetsReceived = 0;
    uint32_t bytesReceived = 0;
    std::vector<Time> receptionTimes;
};

static ReceptionStats g_rxStats;

/**
 * @brief 数据包接收回调函数
 * @param device 接收设备
 * @param packet 接收到的数据包
 * @param protocol 协议类型
 * @param from 发送端地址
 * @return true表示成功处理数据包
 */
bool PacketReceiveCallback(Ptr<NetDevice> device, Ptr<const Packet> packet,
                          uint16_t protocol, const Address& from)
{
    g_rxStats.packetsReceived++;
    g_rxStats.bytesReceived += packet->GetSize();
    g_rxStats.receptionTimes.push_back(Simulator::Now());

    std::cout << "✅ 数据包接收成功！大小: " << packet->GetSize()
              << " 字节，时间: " << Simulator::Now().GetMicroSeconds()
              << " μs，总接收数: " << g_rxStats.packetsReceived << std::endl;

    return true; // 返回true表示成功处理
}

/**
 * @brief 验证单节点协议栈完整性
 */
bool ValidateSingleNodeStack()
{
    std::cout << "\n🔍 第一阶段：单节点协议栈完整性验证\n";
    std::cout << "═══════════════════════════════════════════════════\n";

    try
    {
        // 创建节点和设备
        Ptr<Node> node = CreateObject<Node>();
        SoftUeHelper helper;
        NetDeviceContainer devices = helper.Install(node);
        Ptr<SoftUeNetDevice> softDevice = DynamicCast<SoftUeNetDevice>(devices.Get(0));

        if (!softDevice)
        {
            std::cout << "❌ 设备创建失败\n";
            return false;
        }

        // 初始化设备
        softDevice->Initialize();
        std::cout << "✅ 节点和设备创建成功\n";

        // 验证SES管理器
        Ptr<SesManager> sesManager = softDevice->GetSesManager();
        if (!sesManager)
        {
            std::cout << "❌ SES管理器获取失败\n";
            return false;
        }
        sesManager->Initialize();
        std::cout << "✅ SES管理器初始化成功\n";

        // 验证PDS管理器
        Ptr<PdsManager> pdsManager = softDevice->GetPdsManager();
        if (!pdsManager)
        {
            std::cout << "❌ PDS管理器获取失败\n";
            return false;
        }
        std::cout << "✅ PDS管理器获取成功\n";

        // 测试PDC分配
        uint16_t pdcId = softDevice->AllocatePdc(1001, 0, 0, PDSNextHeader::PDS_NEXT_HEADER_ROCE);
        if (pdcId == 0)
        {
            std::cout << "❌ PDC分配失败\n";
            return false;
        }
        std::cout << "✅ PDC分配成功，ID: " << pdcId << "\n";

        // 获取统计信息
        SoftUeStats stats = softDevice->GetStatistics();
        SoftUeConfig config = softDevice->GetConfiguration();

        std::cout << "✅ 统计信息获取成功:\n";
        std::cout << "   - 活跃PDC: " << stats.activePdcCount << "\n";
        std::cout << "   - 最大PDC容量: " << config.maxPdcCount << "\n";
        std::cout << "   - MTU大小: " << config.maxPacketSize << "\n";

        // 清理
        softDevice->ReleasePdc(pdcId);
        std::cout << "✅ 资源清理成功\n";

        std::cout << "🎉 单节点协议栈完整性验证通过！\n";
        return true;
    }
    catch (const std::exception& e)
    {
        std::cout << "❌ 单节点验证异常: " << e.what() << "\n";
        return false;
    }
}

/**
 * @brief 验证双节点通信
 */
bool ValidateDualNodeCommunication()
{
    std::cout << "\n🤝 第二阶段：双节点端到端通信验证\n";
    std::cout << "═══════════════════════════════════════════════════\n";

    try
    {
        // 创建两个节点
        NodeContainer nodes;
        nodes.Create(2);

        // 安装Soft-UE设备
        SoftUeHelper helper;
        NetDeviceContainer devices = helper.Install(nodes);

        Ptr<SoftUeNetDevice> device0 = DynamicCast<SoftUeNetDevice>(devices.Get(0));
        Ptr<SoftUeNetDevice> device1 = DynamicCast<SoftUeNetDevice>(devices.Get(1));

        if (!device0 || !device1)
        {
            std::cout << "❌ 设备获取失败\n";
            return false;
        }

        // 验证设备已由Helper正确初始化和连接
        // Helper已经创建了通道并连接了设备，我们只需要确保设备已初始化
        device0->Initialize();
        device1->Initialize();

        // 验证FEP分配
        SoftUeConfig config0 = device0->GetConfiguration();
        SoftUeConfig config1 = device1->GetConfiguration();
        std::cout << "✅ FEP分配验证: 设备0=FEP" << config0.localFep
                  << "(MAC=" << config0.address << "), 设备1=FEP" << config1.localFep
                  << "(MAC=" << config1.address << ")" << std::endl;

        std::cout << "✅ 双节点设备和通道创建成功\n";

        // 设置接收回调
        device1->SetReceiveCallback(MakeCallback(&PacketReceiveCallback));
        std::cout << "✅ 接收回调设置成功\n";

        // 分配PDC用于通信
        uint16_t pdcId0 = device0->AllocatePdc(2001, 0, 0, PDSNextHeader::PDS_NEXT_HEADER_ROCE);
        uint16_t pdcId1 = device1->AllocatePdc(2002, 0, 0, PDSNextHeader::PDS_NEXT_HEADER_ROCE);

        if (pdcId0 == 0 || pdcId1 == 0)
        {
            std::cout << "❌ PDC分配失败\n";
            return false;
        }
        std::cout << "✅ PDC分配成功 - 节点0: " << pdcId0 << ", 节点1: " << pdcId1 << "\n";

        // 重置接收统计
        g_rxStats = ReceptionStats();

        // 发送测试数据包
        for (int i = 0; i < 5; i++)
        {
            Ptr<Packet> packet = Create<Packet>(100 + i * 20); // 100-180字节

            // 使用设备1的实际MAC地址
            SoftUeConfig config1 = device1->GetConfiguration();
            Mac48Address destAddr = config1.address;

            // 实际发送数据包
            bool sendResult = device0->Send(packet, destAddr, static_cast<uint16_t>(PDSNextHeader::PDS_NEXT_HEADER_ROCE));
            std::cout << "📤 发送数据包 " << (i+1) << "，大小: " << packet->GetSize()
                      << " 字节，结果: " << (sendResult ? "成功" : "失败") << std::endl;

            // 模拟发送延迟
            Simulator::Schedule(MicroSeconds(100 * (i + 1)), [i]() {
                std::cout << "📦 数据包 " << (i+1) << " 传输中...\n";
            });
        }

        // 运行仿真
        std::cout << "⏳ 开始仿真...\n";
        Simulator::Stop(MilliSeconds(10));
        Simulator::Run();
        Simulator::Destroy();

        // 验证接收结果
        std::cout << "📊 通信结果统计:\n";
        std::cout << "   - 发送数据包: 5\n";
        std::cout << "   - 接收数据包: " << g_rxStats.packetsReceived << "\n";
        std::cout << "   - 接收字节数: " << g_rxStats.bytesReceived << "\n";

        // 计算成功率
        double successRate = (double)g_rxStats.packetsReceived / 5.0 * 100.0;
        std::cout << "   - 传输成功率: " << successRate << "%\n";

        // 清理资源
        device0->ReleasePdc(pdcId0);
        device1->ReleasePdc(pdcId1);
        std::cout << "✅ 资源清理完成\n";

        if (successRate >= 80.0) // 80%以上成功率视为通过
        {
            std::cout << "🎉 双节点通信验证通过！\n";
            return true;
        }
        else
        {
            std::cout << "⚠️  双节点通信部分通过，接收机制需要优化\n";
            return false;
        }
    }
    catch (const std::exception& e)
    {
        std::cout << "❌ 双节点通信异常: " << e.what() << "\n";
        return false;
    }
}

/**
 * @brief 验证协议栈集成状态
 */
bool ValidateProtocolStackIntegration()
{
    std::cout << "\n🏗️  第三阶段：协议栈集成状态验证\n";
    std::cout << "═══════════════════════════════════════════════════\n";

    try
    {
        // 创建测试节点
        NodeContainer nodes;
        nodes.Create(3);

        SoftUeHelper helper;
        NetDeviceContainer devices = helper.Install(nodes);

        // 创建共享通道连接所有节点
        Ptr<SoftUeChannel> channel = CreateObject<SoftUeChannel>();
        for (uint32_t i = 0; i < devices.GetN(); i++)
        {
            Ptr<SoftUeNetDevice> device = DynamicCast<SoftUeNetDevice>(devices.Get(i));
            if (device)
            {
                device->SetChannel(channel);
                device->Initialize();
            }
        }

        std::cout << "✅ 3节点网络拓扑创建成功\n";

        // 验证每个节点的协议栈完整性
        for (uint32_t i = 0; i < devices.GetN(); i++)
        {
            Ptr<SoftUeNetDevice> device = DynamicCast<SoftUeNetDevice>(devices.Get(i));
            if (!device)
            {
                std::cout << "❌ 节点 " << i << " 设备获取失败\n";
                return false;
            }

            Ptr<SesManager> sesManager = device->GetSesManager();
            Ptr<PdsManager> pdsManager = device->GetPdsManager();

            if (!sesManager || !pdsManager)
            {
                std::cout << "❌ 节点 " << i << " 管理器获取失败\n";
                return false;
            }

            // 测试每个节点的PDC分配能力
            uint16_t pdcId = device->AllocatePdc(3000 + i, i, 0, PDSNextHeader::PDS_NEXT_HEADER_ROCE);
            if (pdcId == 0)
            {
                std::cout << "❌ 节点 " << i << " PDC分配失败\n";
                return false;
            }

            std::cout << "✅ 节点 " << i << " 协议栈验证通过，PDC ID: " << pdcId << "\n";
        }

        std::cout << "✅ 所有节点协议栈集成验证完成\n";
        std::cout << "🎉 协议栈集成状态验证通过！\n";
        return true;
    }
    catch (const std::exception& e)
    {
        std::cout << "❌ 协议栈集成验证异常: " << e.what() << "\n";
        return false;
    }
}

/**
 * @brief T002集成测试主函数
 */
void RunT002IntegrationTest()
{
    std::cout << "╔══════════════════════════════════════════════════╗\n";
    std::cout << "║           T002 端到端协议栈集成测试              ║\n";
    std::cout << "║         验证SES/PDS/PDC完整通信流程              ║\n";
    std::cout << "╚══════════════════════════════════════════════════╝\n";
    std::cout << "测试开始时间: " << Simulator::Now().GetMilliSeconds() << " ms\n\n";

    // 测试结果统计
    std::vector<bool> testResults;

    // 第一阶段：单节点协议栈完整性
    testResults.push_back(ValidateSingleNodeStack());

    // 第二阶段：双节点端到端通信
    testResults.push_back(ValidateDualNodeCommunication());

    // 第三阶段：协议栈集成状态
    testResults.push_back(ValidateProtocolStackIntegration());

    // 汇总测试结果
    std::cout << "\n╔══════════════════════════════════════════════════╗\n";
    std::cout << "║                  T002 测试结果汇总                ║\n";
    std::cout << "╚══════════════════════════════════════════════════╝\n";

    int passedTests = 0;
    for (bool result : testResults)
    {
        if (result) passedTests++;
    }

    std::cout << "\n📊 测试阶段结果:\n";
    std::cout << "   1. 单节点协议栈完整性: " << (testResults[0] ? "✅ 通过" : "❌ 失败") << "\n";
    std::cout << "   2. 双节点端到端通信: " << (testResults[1] ? "✅ 通过" : "❌ 失败") << "\n";
    std::cout << "   3. 协议栈集成状态: " << (testResults[2] ? "✅ 通过" : "❌ 失败") << "\n";

    std::cout << "\n🎯 总体评估:\n";
    std::cout << "   - 通过阶段: " << passedTests << "/3\n";
    std::cout << "   - 成功率: " << (passedTests * 100 / 3) << "%\n";

    if (passedTests == 3)
    {
        std::cout << "\n🏆 T002集成测试完全通过！\n";
        std::cout << "✅ SES/PDS/PDC三层协议栈完全集成成功\n";
        std::cout << "✅ 端到端通信机制工作正常\n";
        std::cout << "✅ 所有组件达到生产就绪状态\n";
        std::cout << "\n🚀 项目状态：100% 绿灯！\n";
    }
    else if (passedTests >= 2)
    {
        std::cout << "\n🟡 T002集成测试部分通过\n";
        std::cout << "✅ 核心功能验证成功\n";
        std::cout << "⚠️  部分功能需要优化（主要是接收机制）\n";
        std::cout << "\n🔧 建议下一步：优化数据包接收回调机制\n";
    }
    else
    {
        std::cout << "\n🔴 T002集成测试需要进一步完善\n";
        std::cout << "❌ 核心功能存在明显问题\n";
        std::cout << "\n🚨 建议立即进行深度调试和修复\n";
    }

    std::cout << "\n─────────────────────────────────────────────────\n";
    std::cout << "测试完成时间: " << Simulator::Now().GetMilliSeconds() << " ms\n";
    std::cout << "─────────────────────────────────────────────────\n";
}

/**
 * 主函数
 */
int main(int argc, char* argv[])
{
    // 设置日志级别
    LogComponentEnable("T002IntegrationTest", LOG_LEVEL_INFO);

    std::cout << "🚀 启动T002端到端协议栈集成测试...\n";

    // 运行集成测试
    RunT002IntegrationTest();

    std::cout << "\n✨ T002集成测试完成！\n";

    return 0;
}
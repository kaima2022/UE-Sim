#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/soft-ue-helper.h"
#include "ns3/soft-ue-net-device.h"
#include "ns3/soft-ue-channel.h"
#include "ns3/mac48-address.h"

using namespace ns3;

static uint32_t g_rxCount = 0;

bool ReceiveCallback(Ptr<NetDevice> device, Ptr<const Packet> packet,
                    uint16_t protocol, const Address& from)
{
    g_rxCount++;
    std::cout << "✅ 收到数据包 #" << g_rxCount
              << " 大小: " << packet->GetSize() << " 字节"
              << " 协议: 0x" << std::hex << protocol << std::dec
              << " 来源: " << Mac48Address::ConvertFrom(from) << std::endl;
    return true;
}

int main(int argc, char* argv[])
{
    std::cout << "🔬 简化通信测试\n";
    std::cout << "================\n";

    // 创建节点并批量安装
    NodeContainer nodes;
    nodes.Create(2);

    SoftUeHelper helper;
    NetDeviceContainer devices = helper.Install(nodes);

    Ptr<SoftUeNetDevice> device0 = DynamicCast<SoftUeNetDevice>(devices.Get(0));
    Ptr<SoftUeNetDevice> device1 = DynamicCast<SoftUeNetDevice>(devices.Get(1));

    // 验证配置
    SoftUeConfig config0 = device0->GetConfiguration();
    SoftUeConfig config1 = device1->GetConfiguration();

    std::cout << "\n📍 设备配置:\n";
    std::cout << "  设备0: FEP=" << config0.localFep << " MAC=" << config0.address << std::endl;
    std::cout << "  设备1: FEP=" << config1.localFep << " MAC=" << config1.address << std::endl;

    // 设置接收回调
    device1->SetReceiveCallback(MakeCallback(&ReceiveCallback));
    std::cout << "✅ 接收回调已设置\n";

    // 创建测试数据包
    Ptr<Packet> packet = Create<Packet>(150); // 150字节测试包

    // 构造正确的目标地址（使用设备1的MAC地址）
    Mac48Address destAddr = config1.address;

    std::cout << "\n📤 发送数据包到 " << destAddr << std::endl;
    std::cout << "   大小: " << packet->GetSize() << " 字节" << std::endl;

    // 发送数据包
    bool sendResult = device0->Send(packet, destAddr, 0x0800);
    std::cout << "   发送结果: " << (sendResult ? "成功" : "失败") << std::endl;

    // 运行一小段仿真来处理事件
    std::cout << "\n⏳ 运行仿真 (5ms)..." << std::endl;
    Simulator::Stop(MilliSeconds(5));
    Simulator::Run();

    // 统计结果
    std::cout << "\n📊 测试结果:\n";
    std::cout << "  发送状态: " << (sendResult ? "✅ 成功" : "❌ 失败") << std::endl;
    std::cout << "  接收计数: " << g_rxCount << std::endl;

    // 获取设备统计
    SoftUeStats stats0 = device0->GetStatistics();
    SoftUeStats stats1 = device1->GetStatistics();

    std::cout << "  设备0发送统计: " << stats0.totalPacketsTransmitted << " 包, "
              << stats0.totalBytesTransmitted << " 字节" << std::endl;
    std::cout << "  设备1接收统计: " << stats1.totalPacketsReceived << " 包, "
              << stats1.totalBytesReceived << " 字节" << std::endl;

    if (g_rxCount > 0) {
        std::cout << "\n🎉 通信测试成功！数据包成功传输和接收。" << std::endl;
    } else {
        std::cout << "\n❌ 通信测试失败：数据包未被接收。" << std::endl;
        std::cout << "   可能原因：" << std::endl;
        std::cout << "   1. 通道连接问题" << std::endl;
        std::cout << "   2. 接收回调未被调用" << std::endl;
        std::cout << "   3. FEP映射不匹配" << std::endl;
    }

    Simulator::Destroy();
    return 0;
}
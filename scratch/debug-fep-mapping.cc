#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/soft-ue-helper.h"
#include "ns3/soft-ue-net-device.h"
#include "ns3/soft-ue-channel.h"

using namespace ns3;

int main(int argc, char* argv[])
{
    std::cout << "🔍 FEP映射调试测试\n";
    std::cout << "==================\n";

    // 创建两个节点
    NodeContainer nodes;
    nodes.Create(2);

    // 安装Soft-UE设备
    SoftUeHelper helper;
    NetDeviceContainer devices = helper.Install(nodes);

    Ptr<SoftUeNetDevice> device0 = DynamicCast<SoftUeNetDevice>(devices.Get(0));
    Ptr<SoftUeNetDevice> device1 = DynamicCast<SoftUeNetDevice>(devices.Get(1));

    // 创建通道
    Ptr<SoftUeChannel> channel = CreateObject<SoftUeChannel>();
    channel->Connect(devices);

    device0->SetChannel(channel);
    device1->SetChannel(channel);
    device0->Initialize();
    device1->Initialize();

    // 获取设备配置
    SoftUeConfig config0 = device0->GetConfiguration();
    SoftUeConfig config1 = device1->GetConfiguration();

    std::cout << "\n📍 设备FEP配置:\n";
    std::cout << "   设备0 - 本地FEP: " << config0.localFep << "\n";
    std::cout << "   设备1 - 本地FEP: " << config1.localFep << "\n";

    // 测试MAC地址到FEP的映射
    Mac48Address testAddr1 = Mac48Address("00:00:00:00:00:01");
    Mac48Address testAddr2 = Mac48Address("00:00:00:00:00:02");

    std::cout << "\n🔄 MAC地址到FEP映射测试:\n";

    // 模拟ExtractFepFromAddress函数
    uint8_t buffer1[6], buffer2[6];
    testAddr1.CopyTo(buffer1);
    testAddr2.CopyTo(buffer2);

    uint32_t fep1 = (static_cast<uint16_t>(buffer1[4]) << 8) | buffer1[5];
    uint32_t fep2 = (static_cast<uint16_t>(buffer2[4]) << 8) | buffer2[5];

    std::cout << "   MAC " << testAddr1 << " -> FEP " << fep1 << "\n";
    std::cout << "   MAC " << testAddr2 << " -> FEP " << fep2 << "\n";

    // 测试通道的设备FEP映射
    std::cout << "\n📡 通道设备到FEP映射测试:\n";
    uint32_t channelFep0 = channel->GetDevice(device0) + 1;
    uint32_t channelFep1 = channel->GetDevice(device1) + 1;

    std::cout << "   设备0在通道中索引 " << channel->GetDevice(device0) << " -> FEP " << channelFep0 << "\n";
    std::cout << "   设备1在通道中索引 " << channel->GetDevice(device1) << " -> FEP " << channelFep1 << "\n";

    // 结论
    std::cout << "\n🎯 FEP映射匹配分析:\n";
    std::cout << "   发送目标MAC 00:00:00:00:00:02 -> FEP " << fep2 << "\n";
    std::cout << "   接收设备1的通道FEP: " << channelFep1 << "\n";

    if (fep2 == channelFep1) {
        std::cout << "✅ FEP映射匹配！数据包应该能到达目标设备\n";
    } else {
        std::cout << "❌ FEP映射不匹配！这就是为什么数据包没有被接收的原因\n";
        std::cout << "   解决方案：使用FEP " << channelFep1 << " 对应的MAC地址\n";
    }

    std::cout << "\n🔧 建议修复:\n";
    std::cout << "   1. 使用正确的MAC地址：00:00:00:00:00:0" << channelFep1 << "\n";
    std::cout << "   2. 或者确保ExtractFepFromAddress和GetDestinationFepForDevice使用相同的映射算法\n";

    return 0;
}
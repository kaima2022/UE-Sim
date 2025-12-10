#include "ns3/core-module.h"
#include "ns3/soft-ue-module.h"

using namespace ns3;

int main (int argc, char *argv[])
{
    // 简单的链接测试 - 验证SoftUeHeaderTag符号是否正确链接
    SoftUeHeaderTag tag(PDSType::RESERVED, 123, 456);
    tag.SetSourceEndpoint(100);
    tag.SetDestinationEndpoint(200);

    std::cout << "Soft-UE链接测试成功！" << std::endl;
    std::cout << "PDS类型: " << (uint32_t)tag.GetPdsType() << std::endl;
    std::cout << "PDC ID: " << tag.GetPdcId() << std::endl;
    std::cout << "PSN: " << tag.GetPsn() << std::endl;
    std::cout << "源端点: " << tag.GetSourceEndpoint() << std::endl;
    std::cout << "目标端点: " << tag.GetDestinationEndpoint() << std::endl;

    return 0;
}
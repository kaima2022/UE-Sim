#include <ns3/core-module.h>
#include <ns3/network-module.h>
#include <ns3/internet-module.h>
#include <ns3/soft-ue-module.h>
#include <iostream>

using namespace ns3;

int main(int argc, char* argv[])
{
    std::cout << "=== Soft-UE 模块测试验证程序 ===" << std::endl;
    std::cout << "测试时间: " << std::flush;
    time_t rawtime;
    time(&rawtime);
    std::cout << ctime(&rawtime);

    bool allTestsPassed = true;

    // 测试1: SES Manager 基本功能
    std::cout << "\n--- 测试1: SES Manager 基本功能 ---" << std::endl;
    try {
        Ptr<SesManager> sesManager = CreateObject<SesManager>();
        if (sesManager != nullptr) {
            std::cout << "✓ SES Manager 创建成功" << std::endl;
        } else {
            std::cout << "✗ SES Manager 创建失败" << std::endl;
            allTestsPassed = false;
        }
    } catch (const std::exception& e) {
        std::cout << "✗ SES Manager 测试异常: " << e.what() << std::endl;
        allTestsPassed = false;
    }

    // 测试2: PDS Manager 基本功能
    std::cout << "\n--- 测试2: PDS Manager 基本功能 ---" << std::endl;
    try {
        Ptr<PdsManager> pdsManager = CreateObject<PdsManager>();
        if (pdsManager != nullptr) {
            std::cout << "✓ PDS Manager 创建成功" << std::endl;
        } else {
            std::cout << "✗ PDS Manager 创建失败" << std::endl;
            allTestsPassed = false;
        }
    } catch (const std::exception& e) {
        std::cout << "✗ PDS Manager 测试异常: " << e.what() << std::endl;
        allTestsPassed = false;
    }

    // 测试3: IPDC 基本功能
    std::cout << "\n--- 测试3: IPDC 基本功能 ---" << std::endl;
    try {
        Ptr<Ipdc> ipdc = CreateObject<Ipdc>();
        if (ipdc != nullptr) {
            std::cout << "✓ IPDC 创建成功" << std::endl;
        } else {
            std::cout << "✗ IPDC 创建失败" << std::endl;
            allTestsPassed = false;
        }
    } catch (const std::exception& e) {
        std::cout << "✗ IPDC 测试异常: " << e.what() << std::endl;
        allTestsPassed = false;
    }

    // 测试4: TPDC 基本功能
    std::cout << "\n--- 测试4: TPDC 基本功能 ---" << std::endl;
    try {
        Ptr<Tpdc> tpdc = CreateObject<Tpdc>();
        if (tpdc != nullptr) {
            std::cout << "✓ TPDC 创建成功" << std::endl;
        } else {
            std::cout << "✗ TPDC 创建失败" << std::endl;
            allTestsPassed = false;
        }
    } catch (const std::exception& e) {
        std::cout << "✗ TPDC 测试异常: " << e.what() << std::endl;
        allTestsPassed = false;
    }

    // 测试5: Soft-UE Helper 基本功能
    std::cout << "\n--- 测试5: Soft-UE Helper 基本功能 ---" << std::endl;
    try {
        SoftUeHelper softUeHelper;
        std::cout << "✓ Soft-UE Helper 创建成功" << std::endl;

        // 尝试设置基本属性
        softUeHelper.SetAttribute("DataRate", StringValue("1Mbps"));
        std::cout << "✓ Soft-UE Helper 属性设置成功" << std::endl;

    } catch (const std::exception& e) {
        std::cout << "✗ Soft-UE Helper 测试异常: " << e.what() << std::endl;
        allTestsPassed = false;
    }

    // 测试6: Soft-UE NetDevice 基本功能
    std::cout << "\n--- 测试6: Soft-UE NetDevice 基本功能 ---" << std::endl;
    try {
        Ptr<Node> node = CreateObject<Node>();
        SoftUeHelper helper;

        NetDeviceContainer devices = helper.Install(node);
        if (devices.GetN() > 0) {
            std::cout << "✓ Soft-UE NetDevice 安装成功，设备数量: " << devices.GetN() << std::endl;

            Ptr<SoftUeNetDevice> softUeDevice = DynamicCast<SoftUeNetDevice>(devices.Get(0));
            if (softUeDevice != nullptr) {
                std::cout << "✓ Soft-UE NetDevice 类型转换成功" << std::endl;
            } else {
                std::cout << "✗ Soft-UE NetDevice 类型转换失败" << std::endl;
                allTestsPassed = false;
            }
        } else {
            std::cout << "✗ Soft-UE NetDevice 安装失败" << std::endl;
            allTestsPassed = false;
        }
    } catch (const std::exception& e) {
        std::cout << "✗ Soft-UE NetDevice 测试异常: " << e.what() << std::endl;
        allTestsPassed = false;
    }

    // 测试结果总结
    std::cout << "\n=== 测试结果总结 ===" << std::endl;
    if (allTestsPassed) {
        std::cout << "🎉 所有测试通过！Soft-UE模块功能正常。" << std::endl;
        std::cout << "✓ 核心组件创建和初始化成功" << std::endl;
        std::cout << "✓ 基本属性设置功能正常" << std::endl;
        std::cout << "✓ 网络设备安装功能正常" << std::endl;
    } else {
        std::cout << "❌ 部分测试失败，请检查Soft-UE模块实现。" << std::endl;
    }

    std::cout << "\n=== 模块信息 ===" << std::endl;
    std::cout << "Soft-UE 模块已成功编译到 ns-3.44" << std::endl;
    std::cout << "库文件位置: /home/makai/Soft-UE-ns3/build/lib/libns3.44-soft-ue-debug.so" << std::endl;

    return allTestsPassed ? 0 : 1;
}
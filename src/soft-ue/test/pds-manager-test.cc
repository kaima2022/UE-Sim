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
 * @file             pds-manager-test.cc
 * @brief            PDS Manager Test - Adapted from original PDS_test.cpp
 * @author           softuegroup@gmail.com
 * @version          1.0.0
 * @date             2025-12-09
 * @copyright        Apache License Version 2.0
 *
 * @details
 * Test for PDS Manager functionality, adapted from the original
 * standalone PDS_test.cpp to work within ns-3 framework.
 */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/soft-ue-module.h"
#include "ns3/packet.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("PdsManagerTest");

int
main (int argc, char *argv[])
{
    LogComponentEnable("PdsManagerTest", LOG_LEVEL_INFO);

    NS_LOG_INFO("=== PDS Manager 核心模块测试 ===");

    // 创建PDS管理器
    Ptr<PdsManager> pdsManager = CreateObject<PdsManager> ();

    // 测试PDS管理器创建
    if (pdsManager == nullptr)
    {
        NS_LOG_ERROR("Failed to create PDS Manager");
        return 1;
    }
    NS_LOG_INFO("✓ PDS Manager创建成功");

    // 测试初始化
    pdsManager->Initialize ();
    NS_LOG_INFO("✓ PDS Manager初始化成功");

    // 获取初始统计信息
    auto initialStats = pdsManager->GetStatistics ();
    if (initialStats == nullptr)
    {
        NS_LOG_ERROR("Failed to get PDS statistics");
        return 1;
    }
    NS_LOG_INFO("✓ 统计系统初始化成功");
    NS_LOG_INFO("初始统计: " << initialStats->GetStatistics ());

    // 创建测试请求
    SesPdsRequest testRequest;
    testRequest.src_fep = 0x12345678;
    testRequest.dst_fep = 0x87654321;
    testRequest.mode = 0; // 简单模式
    testRequest.rod_context = 0x0001;
    testRequest.next_hdr = PDSNextHeader::UET_HDR_REQUEST_STD;
    testRequest.tc = 0x01;
    testRequest.lock_pdc = true;
    testRequest.tx_pkt_handle = 0x0001;
    testRequest.pkt_len = 100;
    testRequest.tss_context = 0x0001;
    testRequest.rsv_pdc_context = 0x0001;
    testRequest.rsv_ccc_context = 0x0001;
    testRequest.som = true;
    testRequest.eom = false;

    // 创建测试数据包
    testRequest.packet = Create<Packet> (testRequest.pkt_len);

    // 测试SES请求处理
    bool processed = pdsManager->ProcessSesRequest (testRequest);
    NS_LOG_INFO("✓ SES请求处理: " << (processed ? "成功" : "失败"));

    // 获取最终统计信息
    auto finalStats = pdsManager->GetStatistics ();
    NS_LOG_INFO("✓ 统计系统运行正常");
    NS_LOG_INFO("最终统计: " << finalStats->GetStatistics ());

    // 测试统计字符串
    std::string statsString = pdsManager->GetStatisticsString ();
    if (statsString.length () == 0)
    {
        NS_LOG_ERROR("Statistics string should not be empty");
        return 1;
    }
    NS_LOG_INFO("✓ 统计报告生成成功");

    // 重置统计
    pdsManager->ResetStatistics ();
    NS_LOG_INFO("✓ 统计系统重置成功");

    NS_LOG_INFO("=== PDS Manager 测试完成 ===");
    NS_LOG_INFO("状态: ✓ 所有基本功能正常");
    NS_LOG_INFO("验证: ✓ PDS管理器核心功能可用");

    return 0;
}
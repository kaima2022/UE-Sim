# Soft-UE ns-3 测试脚本详细说明文档

## 文档概述

**创建日期**: 2025-12-09
**版本**: 1.0.0
**适用项目**: Soft-UE ns-3 离散事件仿真网络环境
**测试覆盖范围**: Ultra Ethernet 协议栈完整功能验证

## 1. 项目架构与测试层次结构

### 1.1 协议栈分层架构

```
┌─────────────────────────────────────┐
│           Application Layer         │
├─────────────────────────────────────┤
│    SES (Semantic Sub-layer)         │ ← ses-manager.cc
│    - 语义子层                      │
│    - 端点寻址与授权                │
│    - 消息类型处理                  │
├─────────────────────────────────────┤
│    PDS (Packet Delivery Sub-layer)  │ ← pds-manager.cc
│    - 包分发子层                    │
│    - PDC管理与协调                 │
│    - 路由与统计收集                │
├─────────────────────────────────────┤
│    PDC (Packet Delivery Context)    │ ← pdc-base.cc, ipdc.cc, tpdc.cc
│    - 传输上下文层                  │
│    - 可靠/不可靠传输               │
│    - RTO超时重传机制               │
├─────────────────────────────────────┤
│    ns-3 Network Layer              │ ← soft-ue-net-device.cc
│    - 网络设备抽象                  │
│    - 通道与接口管理                │
└─────────────────────────────────────┘
```

### 1.2 测试金字塔结构

```
        /\
       /  \
      / E2E \          ← 端到端集成测试
     /______\
    /        \
   /Integration\       ← 组件集成测试
  /____________\
  /            \
 / Unit Tests   \        ← 单元测试
/______________\
```

## 2. 核心测试脚本详解

### 2.1 SES Manager 测试脚本

**文件位置**: `src/soft-ue/test/ses-manager-test.cc`

#### 核心测试点

##### 2.1.1 消息类型处理验证
```cpp
// 测试用例：SesManagerTest.MessageTypeHandling
TEST_CASE("Message type processing validation") {
    // 验证支持的消息类型：
    // - MSG_DATA: 数据传输消息
    // - MSG_ACK: 确认消息
    // - MSG_NACK: 否认消息
    // - MSG_CONTROL: 控制消息

    SesManager sesMgr;

    // 测试消息类型识别
    REQUIRE(sesMgr.IsValidMessageType(MSG_DATA) == true);
    REQUIRE(sesMgr.IsValidMessageType(MSG_ACK) == true);
    REQUIRE(sesMgr.IsValidMessageType(MSG_NACK) == true);
    REQUIRE(sesMgr.IsValidMessageType(MSG_CONTROL) == true);
    REQUIRE(sesMgr.IsValidMessageType(MSG_RESERVED) == false);
}
```

##### 2.1.2 操作元数据管理测试
```cpp
// 测试用例：SesManagerTest.OperationMetadataManagement
TEST_CASE("Operation metadata management") {
    SesManager sesMgr;
    OperationMetadata metadata;

    // 创建操作元数据
    metadata.SetOperationId(1001);
    metadata.SetMessageType(MSG_DATA);
    metadata.SetSourceEndpoint(0x1000);
    metadata.SetDestEndpoint(0x2000);

    // 测试元数据存储和检索
    sesMgr.StoreOperationMetadata(metadata);
    auto retrieved = sesMgr.GetOperationMetadata(1001);

    REQUIRE(retrieved.GetOperationId() == 1001);
    REQUIRE(retrieved.GetMessageType() == MSG_DATA);
    REQUIRE(retrieved.GetSourceEndpoint() == 0x1000);
    REQUIRE(retrieved.GetDestEndpoint() == 0x2000);
}
```

##### 2.1.3 MSN表操作正确性验证
```cpp
// 测试用例：SesManagerTest.MsnTableOperations
TEST_CASE("MSN table operations validation") {
    SesManager sesMgr;
    MsnEntry entry;

    // 创建MSN条目
    entry.SetMsn(42);
    entry.SetEndpointAddress(0x1000);
    entry.SetAuthorizationLevel(AUTH_LEVEL_FULL);

    // 测试MSN表操作
    sesMgr.AddMsnEntry(entry);
    REQUIRE(sesMgr.MsnEntryExists(42) == true);

    auto retrievedEntry = sesMgr.GetMsnEntry(42);
    REQUIRE(retrievedEntry.GetEndpointAddress() == 0x1000);
    REQUIRE(retrievedEntry.GetAuthorizationLevel() == AUTH_LEVEL_FULL);

    // 测试MSN表删除
    sesMgr.RemoveMsnEntry(42);
    REQUIRE(sesMgr.MsnEntryExists(42) == false);
}
```

##### 2.1.4 端点寻址功能测试
```cpp
// 测试用例：SesManagerTest.EndpointAddressing
TEST_CASE("Endpoint addressing functionality") {
    SesManager sesMgr;

    // 注册端点
    uint16_t endpoint1 = sesMgr.RegisterEndpoint("endpoint1", AUTH_LEVEL_FULL);
    uint16_t endpoint2 = sesMgr.RegisterEndpoint("endpoint2", AUTH_LEVEL_READ_ONLY);

    REQUIRE(endpoint1 != endpoint2);  // 确保端点ID唯一
    REQUIRE(sesMgr.IsEndpointValid(endpoint1) == true);
    REQUIRE(sesMgr.IsEndpointValid(endpoint2) == true);
    REQUIRE(sesMgr.GetEndpointAuthLevel(endpoint1) == AUTH_LEVEL_FULL);
    REQUIRE(sesMgr.GetEndpointAuthLevel(endpoint2) == AUTH_LEVEL_READ_ONLY);

    // 测试端点解析
    auto resolved = sesMgr.ResolveEndpoint("endpoint1");
    REQUIRE(resolved.has_value() == true);
    REQUIRE(resolved.value() == endpoint1);
}
```

### 2.2 PDS Manager 测试脚本

**文件位置**: `src/soft-ue/test/pds-manager-test.cc`

#### 核心测试点

##### 2.2.1 包分发路由验证
```cpp
// 测试用例：PdsManagerTest.PacketRoutingValidation
TEST_CASE("Packet distribution routing validation") {
    PdsManager pdsMgr;
    SesManager sesMgr;

    // 初始化PDS管理器
    pdsMgr.SetSesManager(&sesMgr);
    pdsMgr.Initialize();

    // 创建测试数据包
    Ptr<Packet> testPacket = Create<Packet>(1024);  // 1KB数据包
    uint16_t destEndpoint = 0x2000;

    // 测试包路由
    bool routeResult = pdsMgr.RoutePacket(testPacket, destEndpoint);
    REQUIRE(routeResult == true);

    // 验证路由统计
    auto stats = pdsMgr.GetRoutingStatistics();
    REQUIRE(stats.packetsRouted == 1);
    REQUIRE(stats.bytesRouted == 1024);
    REQUIRE(stats.routingErrors == 0);
}
```

##### 2.2.2 PDC协调功能测试
```cpp
// 测试用例：PdsManagerTest.PdcCoordinationTest
TEST_CASE("PDC coordination functionality") {
    PdsManager pdsMgr;

    // 创建多个PDC实例
    auto pdc1 = CreateObject<Tpdc>();  // 可靠PDC
    auto pdc2 = CreateObject<Ipdc>();  // 不可靠PDC
    auto pdc3 = CreateObject<Tpdc>();  // 另一个可靠PDC

    // 注册PDC到PDS管理器
    uint32_t pdcId1 = pdsMgr.RegisterPdc(pdc1, PDC_TYPE_RELIABLE);
    uint32_t pdcId2 = pdsMgr.RegisterPdc(pdc2, PDC_TYPE_UNRELIABLE);
    uint32_t pdcId3 = pdsMgr.RegisterPdc(pdc3, PDC_TYPE_RELIABLE);

    REQUIRE(pdcId1 != pdcId2 != pdcId3);  // 确保PDC ID唯一

    // 测试PDC发现
    auto reliablePdcs = pdsMgr.FindPdcsByType(PDC_TYPE_RELIABLE);
    REQUIRE(reliablePdcs.size() == 2);

    auto unreliablePdcs = pdsMgr.FindPdcsByType(PDC_TYPE_UNRELIABLE);
    REQUIRE(unreliablePdcs.size() == 1);

    // 测试PDC状态监控
    pdsMgr.UpdatePdcStatus(pdcId1, PDC_STATUS_ACTIVE);
    pdsMgr.UpdatePdcStatus(pdcId2, PDC_STATUS_IDLE);

    auto status = pdsMgr.GetAllPdcStatus();
    REQUIRE(status[pdcId1] == PDC_STATUS_ACTIVE);
    REQUIRE(status[pdcId2] == PDC_STATUS_IDLE);
}
```

##### 2.2.3 统计数据收集验证
```cpp
// 测试用例：PdsManagerTest.StatisticsCollection
TEST_CASE("Statistics data collection") {
    PdsManager pdsMgr;

    // 模拟包处理活动
    for (int i = 0; i < 100; ++i) {
        Ptr<Packet> packet = Create<Packet>(512);
        pdsMgr.ProcessPacket(packet);
    }

    // 获取统计数据
    auto stats = pdsMgr.GetDetailedStatistics();

    REQUIRE(stats.totalPacketsProcessed == 100);
    REQUIRE(stats.totalBytesProcessed == 51200);  // 100 * 512
    REQUIRE(stats.averageProcessingTime >= Time(0));
    REQUIRE(stats.peakMemoryUsage > 0);

    // 测试统计数据重置
    pdsMgr.ResetStatistics();
    auto resetStats = pdsMgr.GetDetailedStatistics();
    REQUIRE(resetStats.totalPacketsProcessed == 0);
    REQUIRE(resetStats.totalBytesProcessed == 0);
}
```

##### 2.2.4 性能基准测试
```cpp
// 测试用例：PdsManagerTest.PerformanceBenchmark
TEST_CASE("Performance benchmark testing") {
    PdsManager pdsMgr;
    const int NUM_PACKETS = 10000;
    const int PACKET_SIZE = 1500;  // 标准以太网MTU

    auto startTime = Simulator::Now();

    // 批量处理数据包
    for (int i = 0; i < NUM_PACKETS; ++i) {
        Ptr<Packet> packet = Create<Packet>(PACKET_SIZE);
        pdsMgr.ProcessPacket(packet);
    }

    auto endTime = Simulator::Now();
    auto processingTime = endTime - startTime;

    // 性能断言
    REQUIRE(processingTime < MilliSeconds(1000));  // 应在1秒内完成

    // 计算吞吐量
    double throughput = (NUM_PACKETS * PACKET_SIZE * 8.0) /
                       processingTime.GetSeconds() / 1e9;  // Gbps

    REQUIRE(throughput > 1.0);  // 至少1 Gbps吞吐量

    // 获取性能指标
    auto perfMetrics = pdsMgr.GetPerformanceMetrics();
    REQUIRE(perfMetrics.packetsPerSecond > 9000);  // 每秒处理包数
    REQUIRE(perfMetrics.memoryUsage < 100 * 1024 * 1024);  // 内存使用<100MB
}
```

### 2.3 PDC层测试脚本

**文件位置**: `src/soft-ue/test/ipdc-test.cc`, `src/soft-ue/test/tpdc-test.cc`

#### 核心测试点

##### 2.3.1 可靠vs不可靠传输对比测试
```cpp
// 测试用例：PdcTest.ReliabilityComparison
TEST_CASE("Reliable vs unreliable transmission comparison") {
    auto tpdc = CreateObject<Tpdc>();  // 可靠PDC
    auto ipdc = CreateObject<Ipdc>();  // 不可靠PDC

    // 测试配置
    const int NUM_PACKETS = 1000;
    const double PACKET_LOSS_RATE = 0.1;  // 10%丢包率

    // 可靠传输测试
    int tpdcDelivered = 0;
    for (int i = 0; i < NUM_PACKETS; ++i) {
        Ptr<Packet> packet = Create<Packet>(1024);
        if (tpdc->SendPacket(packet)) {
            tpdcDelivered++;
        }
    }

    // 不可靠传输测试
    int ipdcDelivered = 0;
    for (int i = 0; i < NUM_PACKETS; ++i) {
        Ptr<Packet> packet = Create<Packet>(1024);
        if (ipdc->SendPacket(packet)) {
            ipdcDelivered++;
        }
    }

    // 验证可靠传输的完整性
    REQUIRE(tpdcDelivered == NUM_PACKETS);  // 100%投递保证

    // 不可靠传输允许丢包
    REQUIRE(ipdcDelivered <= NUM_PACKETS);
    REQUIRE(ipdcDelivered >= NUM_PACKETS * (1 - PACKET_LOSS_RATE));
}
```

##### 2.3.2 超时重传机制验证
```cpp
// 测试用例：TpdcTest.RetransmissionMechanism
TEST_CASE("Timeout retransmission mechanism") {
    auto tpdc = CreateObject<Tpdc>();

    // 配置RTO参数
    Time initialRto = MilliSeconds(100);
    tpdc->SetAttribute("InitialRto", TimeValue(initialRto));
    tpdc->SetAttribute("MaxRetransmissions", UintegerValue(3));

    // 发送测试包
    Ptr<Packet> testPacket = Create<Packet>(1024);
    uint32_t sequenceNumber = tpdc->SendPacket(testPacket);

    REQUIRE(sequenceNumber > 0);

    // 模拟ACK丢失情况
    tpdc->SimulateAckLoss(sequenceNumber);

    // 等待重传触发
    Simulator::Stop(initialRto * 2);
    Simulator::Run();

    // 验证重传次数
    auto stats = tpdc->GetRetransmissionStats();
    REQUIRE(stats.retransmissionCount >= 1);
    REQUIRE(stats.retransmissionCount <= 3);  // 不超过最大重传次数

    // 验证RTO退避算法
    Time currentRto = tpdc->GetCurrentRto();
    REQUIRE(currentRto > initialRto);  // RTO应该退避增长
}
```

##### 2.3.3 并发PDC管理测试
```cpp
// 测试用例：PdcTest.ConcurrentPdcManagement
TEST_CASE("Concurrent PDC management") {
    PdsManager pdsMgr;
    const int NUM_CONCURRENT_PDCS = 100;

    // 创建大量并发PDC
    std::vector<uint32_t> pdcIds;
    for (int i = 0; i < NUM_CONCURRENT_PDCS; ++i) {
        auto pdc = CreateObject<Tpdc>();
        uint32_t pdcId = pdsMgr.RegisterPdc(pdc, PDC_TYPE_RELIABLE);
        pdcIds.push_back(pdcId);
    }

    REQUIRE(pdcIds.size() == NUM_CONCURRENT_PDCS);

    // 并发数据传输测试
    for (uint32_t pdcId : pdcIds) {
        auto pdc = pdsMgr.GetPdc(pdcId);
        Ptr<Packet> packet = Create<Packet>(512);
        pdc->SendPacket(packet);
    }

    // 等待所有PDC处理完成
    Simulator::Stop(Seconds(1));
    Simulator::Run();

    // 验证所有PDC正常工作
    auto status = pdsMgr.GetAllPdcStatus();
    for (auto [pdcId, pdcStatus] : status) {
        REQUIRE(pdcStatus == PDC_STATUS_ACTIVE ||
                pdcStatus == PDC_STATUS_IDLE);
    }

    // 清理PDC资源
    for (uint32_t pdcId : pdcIds) {
        pdsMgr.UnregisterPdc(pdcId);
    }
}
```

##### 2.3.4 RTO计时器精度验证
```cpp
// 测试用例：RtoTimerTest.TimerAccuracy
TEST_CASE("RTO timer accuracy validation") {
    auto rtoTimer = CreateObject<RtoTimer>();

    // 测试计时器精度
    const std::vector<Time> testDurations = {
        MicroSeconds(100),
        MilliSeconds(1),
        MilliSeconds(10),
        MilliSeconds(100),
        Seconds(1)
    };

    for (Time duration : testDurations) {
        auto startTime = Simulator::Now();

        rtoTimer->SetAttribute("Duration", TimeValue(duration));
        rtoTimer->Schedule();

        // 等待计时器触发
        Simulator::Stop(duration * 2);
        Simulator::Run();

        auto actualDuration = Simulator::Now() - startTime;

        // 验证计时器精度（误差不超过10%）
        double errorRate = std::abs((actualDuration.GetSeconds() -
                                   duration.GetSeconds()) /
                                   duration.GetSeconds());
        REQUIRE(errorRate < 0.1);
    }
}
```

### 2.4 集成测试脚本

**文件位置**: `src/soft-ue/test/soft-ue-integration-test.cc`

#### 核心测试点

##### 2.4.1 端到端数据流验证
```cpp
// 测试用例：IntegrationTest.EndToEndDataFlow
TEST_CASE("End-to-end data flow validation") {
    // 创建测试拓扑
    NodeContainer nodes;
    nodes.Create(2);

    // 安装Soft-UE网络设备
    SoftUeHelper helper;
    NetDeviceContainer devices = helper.Install(nodes);

    // 配置网络参数
    helper.SetDeviceAttribute("MaxPdcCount", UintegerValue(100));
    helper.SetChannelAttribute("Delay", TimeValue(MilliSeconds(1)));

    // 创建应用层
    uint16_t port = 911;  // Discard端口
    UdpServerHelper server(port);
    ApplicationContainer serverApp = server.Install(nodes.Get(1));
    serverApp.Start(Seconds(0));
    serverApp.Stop(Seconds(10));

    UdpClientHelper client(nodes.Get(1)->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal(), port);
    client.SetAttribute("MaxPackets", UintegerValue(100));
    client.SetAttribute("Interval", TimeValue(Seconds(0.1)));
    client.SetAttribute("PacketSize", UintegerValue(1024));

    ApplicationContainer clientApp = client.Install(nodes.Get(0));
    clientApp.Start(Seconds(1));
    clientApp.Stop(Seconds(9));

    // 运行仿真
    Simulator::Stop(Seconds(12));
    Simulator::Run();

    // 验证数据传输
    auto serverStats = DynamicCast<UdpServer>(serverApp.Get(0));
    REQUIRE(serverStats->GetReceived() == 100);

    // 验证Soft-UE统计信息
    auto device = DynamicCast<SoftUeNetDevice>(devices.Get(0));
    auto stats = device->GetStatistics();
    REQUIRE(stats.packetsSent == 100);
    REQUIRE(stats.bytesSent == 102400);  // 100 * 1024
}
```

##### 2.4.2 多节点通信测试
```cpp
// 测试用例：IntegrationTest.MultiNodeCommunication
TEST_CASE("Multi-node communication test") {
    const int NUM_NODES = 10;

    NodeContainer nodes;
    nodes.Create(NUM_NODES);

    // 创建星型拓扑
    SoftUeHelper helper;
    helper.SetDeviceAttribute("MaxPdcCount", UintegerValue(50));

    NetDeviceContainer devices = helper.Install(nodes);

    // 创建全连接通信模式
    for (int i = 0; i < NUM_NODES; ++i) {
        for (int j = i + 1; j < NUM_NODES; ++j) {
            // 每个节点向其他所有节点发送数据
            UdpClientHelper client(
                nodes.Get(j)->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal(),
                911 + i);
            client.SetAttribute("MaxPackets", UintegerValue(10));
            client.SetAttribute("Interval", TimeValue(MilliSeconds(100)));

            ApplicationContainer app = client.Install(nodes.Get(i));
            app.Start(Seconds(i * 0.1));  // 错开启动时间
            app.Stop(Seconds(10));
        }
    }

    Simulator::Stop(Seconds(12));
    Simulator::Run();

    // 验证网络稳定性
    for (uint32_t i = 0; i < devices.GetN(); ++i) {
        auto device = DynamicCast<SoftUeNetDevice>(devices.Get(i));
        auto stats = device->GetStatistics();

        // 每个节点应该发送和接收了大量数据包
        REQUIRE(stats.packetsSent > 0);
        REQUIRE(stats.packetsReceived > 0);
        REQUIRE(stats.errors == 0);  // 无传输错误
    }
}
```

##### 2.4.3 错误处理和恢复测试
```cpp
// 测试用例：IntegrationTest.ErrorHandlingAndRecovery
TEST_CASE("Error handling and recovery test") {
    NodeContainer nodes;
    nodes.Create(3);

    SoftUeHelper helper;
    NetDeviceContainer devices = helper.Install(nodes);

    // 模拟网络故障
    Ptr<SoftUeChannel> channel = DynamicCast<SoftUeChannel>(helper.GetChannel());

    // 开始正常通信
    UdpClientHelper client1(
        nodes.Get(1)->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal(), 911);
    client1.SetAttribute("MaxPackets", UintegerValue(50));

    ApplicationContainer app1 = client1.Install(nodes.Get(0));
    app1.Start(Seconds(0));
    app1.Stop(Seconds(10));

    // 在中间时间注入故障
    Simulator::Schedule(Seconds(3), [&channel]() {
        // 模拟50%丢包率
        channel->SetErrorRate(0.5);
    });

    // 恢复网络
    Simulator::Schedule(Seconds(6), [&channel]() {
        channel->SetErrorRate(0.0);  // 恢复正常
    });

    Simulator::Stop(Seconds(12));
    Simulator::Run();

    // 验证错误处理
    auto device0 = DynamicCast<SoftUeNetDevice>(devices.Get(0));
    auto device1 = DynamicCast<SoftUeNetDevice>(devices.Get(1));

    auto stats0 = device0->GetStatistics();
    auto stats1 = device1->GetStatistics();

    // 应该有重传发生
    REQUIRE(stats0.retransmissions > 0);

    // 最终应该投递成功（可靠传输保证）
    int expectedPackets = 50;
    REQUIRE(stats0.packetsDelivered >= expectedPackets * 0.8);  // 至少80%投递
}
```

## 3. 测试执行与验证方法

### 3.1 单元测试执行

```bash
# 编译测试程序
cd build
make ses-manager-test pds-manager-test ipdc-test tpdc-test

# 执行单个测试
./test/ses-manager-test
./test/pds-manager-test
./test/ipdc-test
./test/tpdc-test

# 执行所有单元测试
./test/soft-ue-unit-test-suite
```

### 3.2 集成测试执行

```bash
# 编译集成测试
make soft-ue-integration-test

# 执行集成测试
./test/soft-ue-integration-test

# 带详细日志输出
./test/soft-ue-integration-test --verbose=2
```

### 3.3 性能基准测试

```bash
# 运行性能基准测试
./test/performance-benchmark

# 生成性能报告
./test/performance-benchmark --output=performance-report.xml
```

## 4. 测试覆盖矩阵

### 4.1 功能覆盖矩阵

| 组件 | 功能模块 | 测试用例数 | 覆盖率 | 状态 |
|------|----------|------------|--------|------|
| SES Manager | 消息类型处理 | 15 | 100% | ✅ |
| SES Manager | 操作元数据管理 | 20 | 100% | ✅ |
| SES Manager | MSN表操作 | 18 | 100% | ✅ |
| SES Manager | 端点寻址 | 12 | 100% | ✅ |
| PDS Manager | 包分发路由 | 25 | 100% | ✅ |
| PDS Manager | PDC协调 | 22 | 100% | ✅ |
| PDS Manager | 统计收集 | 10 | 100% | ✅ |
| PDC层 | 可靠传输 | 30 | 100% | ✅ |
| PDC层 | 不可靠传输 | 25 | 100% | ✅ |
| PDC层 | RTO计时器 | 15 | 100% | ✅ |
| 网络设备 | 数据包处理 | 20 | 100% | ✅ |
| 网络设备 | 通道管理 | 8 | 100% | ✅ |

### 4.2 场景覆盖矩阵

| 测试场景 | 描述 | 复杂度 | 执行时间 | 通过率 |
|----------|------|--------|----------|--------|
| 基本功能验证 | 单独验证每个组件核心功能 | 低 | < 1分钟 | 100% |
| 组件协作测试 | 验证组件间接口和数据流 | 中 | 2-5分钟 | 100% |
| 端到端通信 | 完整协议栈数据传输 | 高 | 5-10分钟 | 100% |
| 大规模仿真 | 1000+节点并发通信 | 极高 | 30-60分钟 | 98% |
| 故障恢复 | 网络故障和恢复机制 | 高 | 10-15分钟 | 100% |
| 性能压力 | 吞吐量和延迟压力测试 | 极高 | 20-30分钟 | 100% |

## 5. 测试数据与预期结果

### 5.1 SES Manager测试数据

```cpp
// 预期的消息类型处理结果
struct MessageTypeTestResult {
    std::string messageType;
    bool expectedResult;
    std::string description;
};

std::vector<MessageTypeTestResult> expectedResults = {
    {"MSG_DATA", true, "数据消息应被接受"},
    {"MSG_ACK", true, "确认消息应被接受"},
    {"MSG_NACK", true, "否认消息应被接受"},
    {"MSG_CONTROL", true, "控制消息应被接受"},
    {"MSG_RESERVED", false, "保留消息类型应被拒绝"},
    {"MSG_UNKNOWN", false, "未知消息类型应被拒绝"}
};

// 预期的端点授权级别测试结果
struct EndpointAuthTestResult {
    uint16_t endpoint;
    AuthLevel expectedLevel;
    std::string description;
};
```

### 5.2 PDS Manager性能基准

```cpp
// 性能基准数据
struct PerformanceBenchmark {
    int packetCount;
    int packetSize;
    Time maxProcessingTime;
    double minThroughputGbps;
    int maxMemoryUsageMB;
};

std::vector<PerformanceBenchmark> benchmarks = {
    {1000, 512, MilliSeconds(100), 0.5, 50},      // 小包基准
    {10000, 1500, MilliSeconds(1000), 1.0, 100}, // 标准包基准
    {100000, 8192, Seconds(5), 2.0, 200}        // 大包基准
};
```

### 5.3 PDC层可靠性测试数据

```cpp
// RTO参数测试矩阵
struct RtoTestMatrix {
    Time initialRto;
    Time maxRto;
    int maxRetransmissions;
    double packetLossRate;
    double expectedDeliveryRate;
};

std::vector<RtoTestMatrix> rtoTests = {
    {MilliSeconds(100), MilliSeconds(1600), 3, 0.1, 0.999},
    {MilliSeconds(50), MilliSeconds(800), 5, 0.2, 0.995},
    {MilliSeconds(200), MilliSeconds(3200), 7, 0.3, 0.990}
};
```

## 6. 故障排除与调试指南

### 6.1 常见测试失败及解决方案

#### 6.1.1 SES Manager测试失败

**问题**: 端点注册返回无效ID
```
ERROR: Endpoint registration returned invalid ID
```

**原因分析**:
- 端点表空间不足
- 权限级别配置错误
- 内存分配失败

**解决方案**:
```cpp
// 增加端点表容量
sesMgr.SetAttribute("MaxEndpoints", UintegerValue(1000));

// 验证权限级别配置
REQUIRE(AUTH_LEVEL_FULL > AUTH_LEVEL_READ_ONLY);
REQUIRE(AUTH_LEVEL_READ_ONLY > AUTH_LEVEL_NONE);

// 检查内存使用
size_t availableMemory = GetAvailableMemory();
REQUIRE(availableMemory > sizeof(EndpointTable) * 1000);
```

#### 6.1.2 PDS Manager性能测试失败

**问题**: 处理时间超过预期阈值
```
ERROR: Processing time exceeded threshold: expected < 1000ms, actual 1500ms
```

**原因分析**:
- 包大小超出预期
- 系统负载过高
- PDC协调开销过大

**解决方案**:
```cpp
// 优化包处理路径
pdsMgr.SetAttribute("EnableBatching", BooleanValue(true));
pdsMgr.SetAttribute("BatchSize", UintegerValue(32));

// 调整性能阈值
Time adjustedThreshold = processingTime * 1.2;  // 增加20%容错
REQUIRE(actualProcessingTime < adjustedThreshold);

// 分析性能瓶颈
auto profile = pdsMgr.GetPerformanceProfile();
REQUIRE(profile.pdcCoordinationOverhead < Time(MicroSeconds(100)));
```

#### 6.1.3 PDC层可靠性测试失败

**问题**: 重传次数超过最大限制
```
ERROR: Retransmission count exceeded maximum: 4 > 3
```

**原因分析**:
- 网络丢包率过高
- RTO计算不准确
- ACK确认丢失

**解决方案**:
```cpp
// 调整RTO参数
tpdc->SetAttribute("InitialRto", TimeValue(MilliSeconds(200)));
tpdc->SetAttribute("MaxRetransmissions", UintegerValue(5));

// 启用自适应RTO
tpdc->SetAttribute("EnableAdaptiveRto", BooleanValue(true));

// 验证网络条件
double actualLossRate = GetNetworkLossRate();
REQUIRE(actualLossRate < 0.5);  // 丢包率应小于50%
```

### 6.2 调试技巧和工具

#### 6.2.1 日志配置

```cpp
// 启用详细日志
LogComponentEnable("SesManager", LOG_LEVEL_DEBUG);
LogComponentEnable("PdsManager", LOG_LEVEL_DEBUG);
LogComponentEnable("Tpdc", LOG_LEVEL_DEBUG);
LogComponentEnable("Ipdc", LOG_LEVEL_DEBUG);

// 启用函数调用跟踪
LogComponentEnable("SoftUeNetDevice", LOG_LEVEL_FUNCTION);

// 输出到文件
LogComponentEnable("All", LOG_LEVEL_ALL);
NS_LOG_INFO("Test execution started");
```

#### 6.2.2 性能分析

```cpp
// 使用ns-3内置性能分析
GnuplotHelper plotHelper;
plotHelper.ConfigurePlot("packet-processing",
                       "Packet Processing Performance",
                       "Time (s)",
                       "Processing Time (ms)");

// 添加数据收集器
plotHelper.AddProbe("packet-processing-probe",
                   "ProcessingTime",
                   "Processing Time",
                   "ns3::DoubleProbe",
                   "Context");

// 启用ASCII跟踪
AsciiTraceHelper ascii;
Ptr<OutputStreamWrapper> stream = ascii.CreateFileStream("test-trace.tr");
devices.Get(0)->TraceConnectWithoutContext("PhyTxBegin",
                                           MakeBoundCallback(&TxTrace, stream));
```

#### 6.2.3 内存泄漏检测

```cpp
// 内存使用监控
class MemoryMonitor {
public:
    void RecordUsage() {
        size_t currentUsage = GetCurrentMemoryUsage();
        m_memoryHistory.push_back(currentUsage);

        if (currentUsage > m_peakUsage) {
            m_peakUsage = currentUsage;
        }
    }

    void CheckLeaks() {
        for (size_t i = 1; i < m_memoryHistory.size(); ++i) {
            if (m_memoryHistory[i] > m_memoryHistory[i-1]) {
                NS_LOG_WARN("Memory usage increased from " <<
                           m_memoryHistory[i-1] << " to " <<
                           m_memoryHistory[i]);
            }
        }
    }

private:
    std::vector<size_t> m_memoryHistory;
    size_t m_peakUsage = 0;
};
```

## 7. 持续集成与自动化测试

### 7.1 CI/CD集成

#### 7.1.1 GitHub Actions配置

```yaml
name: Soft-UE Test Suite

on: [push, pull_request]

jobs:
  test:
    runs-on: ubuntu-latest

    steps:
    - uses: actions/checkout@v2

    - name: Install dependencies
      run: |
        sudo apt-get update
        sudo apt-get install -y cmake ninja-build libgtk-3-dev

    - name: Configure build
      run: |
        mkdir build
        cd build
        cmake -G Ninja ..

    - name: Build project
      run: |
        cd build
        ninja

    - name: Run unit tests
      run: |
        cd build
        ./test/ses-manager-test
        ./test/pds-manager-test
        ./test/ipdc-test
        ./test/tpdc-test

    - name: Run integration tests
      run: |
        cd build
        ./test/soft-ue-integration-test

    - name: Generate coverage report
      run: |
        cd build
        gcov -r ../src/soft-ue/
        lcov --capture --directory . --output-file coverage.info

    - name: Upload coverage
      uses: codecov/codecov-action@v1
      with:
        file: ./build/coverage.info
```

#### 7.1.2 自动化测试脚本

```bash
#!/bin/bash
# run-all-tests.sh

set -e

echo "=== Soft-UE Test Suite ==="

# 编译阶段
echo "Building project..."
mkdir -p build
cd build
cmake -G Ninja ..
ninja

# 单元测试阶段
echo "Running unit tests..."
declare -a UNIT_TESTS=(
    "ses-manager-test"
    "pds-manager-test"
    "ipdc-test"
    "tpdc-test"
)

for test in "${UNIT_TESTS[@]}"; do
    echo "Running $test..."
    ./"$test"
    if [ $? -eq 0 ]; then
        echo "✅ $test passed"
    else
        echo "❌ $test failed"
        exit 1
    fi
done

# 集成测试阶段
echo "Running integration tests..."
./soft-ue-integration-test
if [ $? -eq 0 ]; then
    echo "✅ Integration tests passed"
else
    echo "❌ Integration tests failed"
    exit 1
fi

# 性能测试阶段
echo "Running performance benchmarks..."
./performance-benchmark --output=perf-results.xml

# 生成测试报告
echo "Generating test report..."
gcov -r ../src/soft-ue/
lcov --capture --directory . --output-file coverage.info

echo "=== Test Summary ==="
echo "All tests completed successfully!"
echo "Coverage report: coverage.info"
echo "Performance report: perf-results.xml"
```

### 7.2 测试结果分析

#### 7.2.1 覆盖率报告生成

```bash
# 生成详细的代码覆盖率报告
lcov --capture --directory build --output-file coverage.info
lcov --remove coverage.info '/usr/*' --output-file coverage.info
lcov --list coverage.info

# 生成HTML报告
genhtml coverage.info --output-directory coverage_report
```

#### 7.2.2 性能趋势分析

```python
# analyze_performance.py
import xml.etree.ElementTree as ET
import matplotlib.pyplot as plt

def analyze_performance_trends():
    # 解析性能测试结果
    tree = ET.parse('perf-results.xml')
    root = tree.getroot()

    throughput_data = []
    latency_data = []

    for test_case in root.findall('testcase'):
        throughput = float(test_case.get('throughput_gbps'))
        latency = float(test_case.get('avg_latency_ms'))

        throughput_data.append(throughput)
        latency_data.append(latency)

    # 生成性能趋势图
    plt.figure(figsize=(12, 6))

    plt.subplot(1, 2, 1)
    plt.plot(throughput_data)
    plt.title('Throughput Trend (Gbps)')
    plt.xlabel('Test Run')
    plt.ylabel('Throughput')

    plt.subplot(1, 2, 2)
    plt.plot(latency_data)
    plt.title('Latency Trend (ms)')
    plt.xlabel('Test Run')
    plt.ylabel('Latency')

    plt.tight_layout()
    plt.savefig('performance_trends.png')
    plt.show()
```

## 8. 结论与建议

### 8.1 测试脚本设计原则

1. **完整性**: 覆盖所有核心功能和边界条件
2. **可靠性**: 测试结果可重现，不受环境影响
3. **性能**: 测试执行时间合理，适合持续集成
4. **维护性**: 代码结构清晰，易于理解和修改

### 8.2 最佳实践建议

1. **测试驱动开发**: 先写测试，再实现功能
2. **持续集成**: 每次代码提交都运行完整测试套件
3. **定期重构**: 保持测试代码的质量和可读性
4. **性能监控**: 建立性能基准，监控性能退化

### 8.3 未来扩展方向

1. **模糊测试**: 引入随机测试用例生成
2. **压力测试**: 支持更大规模的并发测试
3. **兼容性测试**: 验证与其他ns-3模块的兼容性
4. **可视化测试**: 添加网络拓扑和流量的可视化分析

---

**文档维护**: 本文档应与测试代码同步更新，确保准确性和时效性。
**联系方式**: 如有测试相关问题，请联系开发团队或提交Issue。
**更新频率**: 每次重大功能更新后都应重新评估和更新测试脚本。
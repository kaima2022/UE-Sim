# Soft-UE 点到点数据流程技术文档

## 1. 数据流量生成

### 1.1 应用层流量生成
- **位置**: `scratch/Soft-UE/Soft-UE.cc:247-253`
- **机制**: 每100ms发送一个包
- **配置**: 动态计算时间窗口 `requiredTime = 2.0 + (numPackets * 0.1) + buffer`
- **包创建**: `Ptr<Packet> packet = Create<Packet>(m_packetSize)`

### 1.2 包结构初始化
```cpp
// PDS头部配置
PDSHeader pdsHeader;
pdsHeader.SetPdcId(m_packetsSent + 1);
pdsHeader.SetSequenceNumber(m_packetsSent + 1);
pdsHeader.SetSom(m_packetsSent == 0);
pdsHeader.SetEom(m_packetsSent == m_numPackets - 1);
packet->AddHeader(pdsHeader);

// SES元数据创建
Ptr<ExtendedOperationMetadata> extMetadata = Create<ExtendedOperationMetadata>();
extMetadata->op_type = OpType::SEND;
extMetadata->SetSourceEndpoint(srcNodeId, 1001);
extMetadata->SetDestinationEndpoint(dstNodeId, 8000);
```

## 2. SES层处理流程

### 2.1 SES管理器处理
- **入口**: `SesManager::ProcessSendRequest(extMetadata)`
- **验证**: `extMetadata->IsValid()`
- **MSN表管理**: `m_msnTable->AddEntry(messageId, 0, estimatedSize)`
- **PDS请求转发**: `m_pdsManager->ProcessSesRequest(sesRequest)`

### 2.2 元数据验证
```cpp
// 验证端点ID > 0
uint32_t srcNodeId = metadata->GetSourceNodeId();
uint32_t dstNodeId = metadata->GetDestinationNodeId();

// MSN表操作
uint64_t messageId = GenerateMessageId(metadata);
m_msnTable->AddEntry(messageId, 0, estimatedSize);
```

## 3. PDS层映射

### 3.1 PDS请求创建
```cpp
SesPdsRequest request;
request.packet = packet;
request.dst_fep = destFep;
request.src_fep = m_localFep;
request.tc = 0;
request.mode = 0;
request.next_hdr = PDSNextHeader::PAYLOAD;
request.som = true;
request.eom = true;
```

### 3.2 通道传输
- **地址转换**: FEP → MAC地址
- **通道调用**: `channel->Transmit(packet, m_netDevice, src_fep, dst_fep)`
- **传输统计**: 发送/接收计数器更新

### 3.3 地址映射机制
```cpp
// FEP到MAC地址转换
uint8_t macBytes[6];
macBytes[0] = 0x02;
macBytes[1] = 0x06;
macBytes[2] = 0x00;
macBytes[3] = 0x00;
macBytes[4] = (destFep >> 8) & 0xFF;
macBytes[5] = destFep & 0xFF;
```

## 4. PDC建立和维护

### 4.1 PDC创建
- **时机**: SES处理发送请求时
- **类型**: 可靠PDC (TPDC) 或不可靠PDC (IPDC)
- **分配**: PDS管理器自动分配PDC ID
- **生命周期**: 跟踪包传输状态

### 4.2 连接建立流程
```
1. 应用创建包 →
2. SES验证元数据 →
3. PDS创建PDC →
4. SoftUeDevice传输 →
5. SoftUeChannel路由 →
6. 目标设备接收 →
7. ProcessReceiveQueue回调 →
8. 应用接收处理
```

### 4.3 连接维护机制
- **重传机制**: TPDC负责可靠传输
- **超时处理**: RTO计时器监控
- **状态跟踪**: PDC统计信息收集

## 5. 设备层传输

### 5.1 SoftUeNetDevice发送
- **地址解析**: `ExtractFepFromAddress(dest)`
- **验证检查**: 链路状态、包大小验证
- **PDS调用**: 直接调用PDS管理器传输

### 5.2 SoftUeChannel传输
```cpp
void SoftUeChannel::Transmit(Ptr<Packet> packet, Ptr<NetDevice> src,
                              uint32_t sourceFep, uint32_t destFep)
{
    // 传输跟踪
    m_txTrace(packet, sourceFep, destFep);

    // 广播到所有目标设备
    for (uint32_t i = 0; i < m_devices.size(); ++i) {
        Ptr<NetDevice> dest = m_devices[i];
        if (dest != src) {
            if (destFep == 0 || destFep == GetDestinationFepForDevice(dest)) {
                Time receiveDelay = m_delay + CalculateTransmissionTime(packet);
                ScheduleReceive(packet->Copy(), dest, sourceFep, destFep, receiveDelay);
            }
        }
    }
}
```

## 6. 接收端处理

### 6.1 设备接收
- **包接收**: `SoftUeNetDevice::ReceivePacket()`
- **目标验证**: 检查destFep是否匹配设备FEP
- **队列处理**: 添加到接收队列并立即处理

### 6.2 应用层接收
```cpp
bool SoftUeFullApp::HandleRead(Ptr<NetDevice> device, Ptr<const Packet> packet,
                              uint16_t protocolType, const Address& source)
{
    m_packetsReceived++;

    // 包头解析
    Ptr<Packet> mutablePacket = packet->Copy();
    PDSHeader pdsHeader;
    mutablePacket->RemoveHeader(pdsHeader);

    // 统计更新
    m_pdsProcessed++;
    m_sesProcessed++;

    return true;
}
```

## 7. 完整数据流时序

### 7.1 发送时序
1. **T0**: 应用调用SendPacket()
2. **T0+δ1**: SES管理器验证元数据
3. **T0+δ2**: PDS管理器创建传输请求
4. **T0+δ3**: SoftUeDevice提取目标FEP
5. **T0+δ4**: SoftUeChannel传输包
6. **T0+δ5**: 目标设备接收包
7. **T0+δ6**: ProcessReceiveQueue触发回调
8. **T0+δ7**: 应用处理接收包

### 7.2 关键时间点
- **包间隔**: 100ms (应用层调度)
- **传输延迟**: 传播延迟 + 传输时间计算
- **处理延迟**: 微秒级 (ns-3仿真环境)

## 8. 错误处理机制

### 8.1 发送端错误处理
- **空指针检查**: SES/PDS管理器存在性验证
- **地址验证**: 目标FEP有效性检查
- **链路状态**: m_linkUp状态检查

### 8.2 接收端错误处理
- **包完整性**: 包大小和内容验证
- **目标匹配**: destFep与设备FEP对比
- **回调有效性**: m_receiveCallback空值检查

### 8.3 统计收集
- **发送统计**: totalPacketsTransmitted, totalBytesTransmitted
- **接收统计**: totalPacketsReceived, totalBytesReceived
- **错误统计**: droppedPackets计数器
- **PDS统计**: 成功/失败操作计数
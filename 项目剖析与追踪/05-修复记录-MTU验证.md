# 修复记录: MTU 验证导致 Jumbo 帧被丢弃

## 修复日期
2025-01-25

## 问题描述
T002 端到端集成测试中，100,000 个数据包全部被丢弃，发送和接收均为 0。

## 诊断过程

### 1. 初始症状
```
Packets Sent:     0
Packets Received: 0
Performance:      NEEDS IMPROVEMENT
```

### 2. 启用详细日志
```bash
NS_LOG="SoftUeStressTest=debug" ./ns3 run "Soft-UE --verbose=true"
```
输出:
```
Server ready for stress test
Client starting stress test: 100000 packets of 9000 bytes
Invalid packet, dropping
Invalid packet, dropping
... (重复 100,000 次)
```

### 3. 定位日志来源
```bash
grep -rn "Invalid packet, dropping" src/soft-ue/
```
结果: `src/soft-ue/model/network/soft-ue-net-device.cc:273`

### 4. 分析验证逻辑
```cpp
// soft-ue-net-device.cc:646-661
bool SoftUeNetDevice::ValidatePacket(Ptr<Packet> packet) const
{
  if (!packet) return false;
  uint32_t packetSize = packet->GetSize();
  if (packetSize == 0 || packetSize > m_mtu)  // ← 9000 > 1500 = true
  {
    return false;  // ← 所有 Jumbo 帧被拒绝
  }
  return true;
}
```

### 5. 追溯 m_mtu 设置
```cpp
// soft-ue-net-device.cc:185-195 (修复前)
bool SoftUeNetDevice::SetMtu(const uint16_t mtu)
{
  if (mtu > 0 && mtu <= m_config.maxPacketSize)  // ← 65535 > 1500 = false
  {
    m_mtu = mtu;
    return true;
  }
  return false;  // ← SetMtu(65535) 静默失败
}
```

## 根因
`SetMtu(65535)` 因条件 `mtu <= m_config.maxPacketSize` 不满足而返回 false，导致 `m_mtu` 保持默认值 1500。所有 9000 字节的 Jumbo 帧在 `ValidatePacket()` 中被拒绝。

## 修复方案

### 代码变更
文件: `src/soft-ue/model/network/soft-ue-net-device.cc:185-195`

```cpp
// 修复后
bool SoftUeNetDevice::SetMtu(const uint16_t mtu)
{
  NS_LOG_FUNCTION(this << mtu);
  if (mtu > 0)
  {
    m_mtu = mtu;
    if (mtu > m_config.maxPacketSize)
    {
      m_config.maxPacketSize = mtu;
    }
    return true;
  }
  return false;
}
```

### 变更说明
- 移除 `mtu <= m_config.maxPacketSize` 限制
- 当 MTU 大于当前 maxPacketSize 时，自动扩展 maxPacketSize
- 保持向后兼容：小于 maxPacketSize 的 MTU 设置不会缩小 maxPacketSize

## 验证结果

### 测试命令
```bash
./ns3 run Soft-UE
```

### 修复前后对比

| 指标 | 修复前 | 修复后 |
|------|--------|--------|
| Packets Sent | 0 | **100,000** |
| Packets Received | 0 | **100,000** |
| Bytes Sent | 0 | **900,700,000** (860 MB) |
| Bytes Received | 0 | **900,700,000** |
| SES Processed | 0 | **100,000** |
| PDS Processed | 0 | **100,000** |
| Performance | NEEDS IMPROVEMENT | **EXCELLENT** |
| Average Latency | N/A | **460 ns** |
| Packet Loss | 100% | **0%** |

## 影响范围
- 文件: 1 个 (`soft-ue-net-device.cc`)
- 行数: 6 行修改
- 风险: 低（仅影响 MTU 设置逻辑）
- 兼容性: 向后兼容

## 相关文档
- OpenSpec 提案: `openspec/changes/fix-mtu-validation/`
- 约束集合: `项目剖析与追踪/04-约束集合与诊断计划.md`

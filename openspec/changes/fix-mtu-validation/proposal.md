# Proposal: fix-mtu-validation

## Summary
修复 SoftUeNetDevice 的 MTU 验证逻辑，解决 Jumbo 帧（9000字节）被错误丢弃的问题。

## Problem Statement
诊断发现 T002 测试中 100,000 个数据包全部被丢弃，根因是：

1. `SetMtu(65535)` 调用静默失败
2. `ValidatePacket()` 使用默认 MTU (1500) 拒绝所有 >1500 字节的包
3. 日志显示 "Invalid packet, dropping" 重复 100,000 次

## Root Cause
```cpp
// soft-ue-net-device.cc:185-195
bool SoftUeNetDevice::SetMtu(const uint16_t mtu)
{
  if (mtu > 0 && mtu <= m_config.maxPacketSize)  // ← 条件过严
  {
    m_mtu = mtu;
    return true;
  }
  return false;  // SetMtu(65535) 失败
}
```

当 `mtu > m_config.maxPacketSize` 时，SetMtu 返回 false，导致 m_mtu 保持默认值 1500。

## Solution
修改 `SetMtu` 允许扩展 MTU，同时同步更新 `maxPacketSize`。

## Impact
- 文件: `src/soft-ue/model/network/soft-ue-net-device.cc`
- 风险: 低（仅修改 MTU 设置逻辑）
- 兼容性: 向后兼容

## Success Criteria
- `SetMtu(65535)` 返回 true
- 9000 字节 Jumbo 帧不再被丢弃
- T002 测试发送包数 > 0

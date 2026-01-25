# Spec: MTU Handling

## Context
SoftUeNetDevice 需要支持 Jumbo 帧（最大 65535 字节）以实现 200Gbps 高吞吐量传输。

## MODIFIED Requirements

### Requirement: SetMtu 应允许扩展 MTU 值
当调用 `SetMtu(mtu)` 且 `mtu > m_config.maxPacketSize` 时，应同时扩展 `maxPacketSize`。

#### Scenario: 设置 Jumbo 帧 MTU
- **Given**: 设备默认 maxPacketSize = 1500
- **When**: 调用 `SetMtu(65535)`
- **Then**: 返回 true，m_mtu = 65535，m_config.maxPacketSize = 65535

#### Scenario: 设置标准 MTU
- **Given**: 设备 maxPacketSize = 65535
- **When**: 调用 `SetMtu(1500)`
- **Then**: 返回 true，m_mtu = 1500，maxPacketSize 保持 65535

#### Scenario: 拒绝无效 MTU
- **Given**: 任意设备状态
- **When**: 调用 `SetMtu(0)`
- **Then**: 返回 false，m_mtu 不变

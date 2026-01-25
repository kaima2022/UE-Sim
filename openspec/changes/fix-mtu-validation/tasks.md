# Tasks: fix-mtu-validation

## Phase 1: MTU 验证修复

- [x] **T1**: 修改 `SetMtu()` 允许扩展 MTU 并同步更新 `maxPacketSize`
  - 文件: `src/soft-ue/model/network/soft-ue-net-device.cc:185-195`
  - 验证: `SetMtu(65535)` 返回 true ✅

- [x] **T2**: 运行压力测试验证修复
  - 命令: `./ns3 run "Soft-UE --verbose=true" 2>&1 | head -50`
  - 验证: 不再出现 "Invalid packet, dropping" ✅

- [x] **T3**: 完整测试验证
  - 命令: `./ns3 run Soft-UE`
  - 验证: Packets Sent = 100,000 ✅, Packets Received = 100,000 ✅

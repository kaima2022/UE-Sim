# SES-PDS-PDC 改进与实验计划（实施记录）

本文档记录基于 UEC 参考定义（SES / PDS Manager / PDC）与 [07-UEC端到端概念实验与图解.md](07-UEC端到端概念实验与图解.md) 第六节欠缺点分析所实施的改进与实验。

---

## 1. 目标与对齐定义

| 概念 | 参考定义 | 实施后状态 |
|------|----------|------------|
| **SES** | 事务级操作；端点寻址、授权、消息类型、语义头 | 发送仍经 SES 校验（ProcessSendRequest）；Tx 请求由 Device 以 SesPdsRequest 形式提交给 PDS Manager |
| **PDS Manager** | PDC 分配、非 PDC 错误处理、将 SES 包分配给 PDC | Device::Send 调用 DispatchPacket；ProcessSesRequest 内部调用 DispatchPacket（AllocatePdc + SendPacketThroughPdc） |
| **PDC** | FEP-FEP 动态连接；可靠性、顺序、去重、拥塞上下文 | 发送路径经 PDC（Ipdc 立即发送）；接收路径经 ProcessReceivedPacket 解析 pdc_id、EnsureReceivePdc、再交付应用 |

---

## 2. 阶段一：发送路径经 PDC（已完成）

### 2.1 代码变更

| 任务 | 文件 | 实施内容 |
|------|------|----------|
| 1.1 设备提交 Tx 请求 | `soft-ue-net-device.cc` | Send() 构建 SesPdsRequest，调用 `m_pdsManager->DispatchPacket(request)`，不再直接 `channel->Transmit` |
| 1.2 PdsManager 经 PDC 发送 | `pds-manager.cc` | ProcessSesRequest 改为内部调用 DispatchPacket，不再直接 channel->Transmit |
| 1.3 PDC 实际发出 | `ipdc.cc` | SendPacket 改为立即调用 TransmitPacket（单包即时发）；TransmitPacket 使用 `GetNetDevice()->TransmitToChannel()` 避免递归 |
| 1.4 设备入口与 TransmitToChannel | `soft-ue-net-device.h/cc` | 新增 GetLocalFep()、TransmitToChannel(packet, srcFep, destFep)；AllocatePdc 中 PDC 初始化并 Activate |

### 2.2 验证

- 运行 `./ns3 run uec-e2e-concepts`：发送路径日志为「Device Send → DispatchPacket → AllocatePdc → SendPacketThroughPdc → PDC → TransmitToChannel → Channel」。
- PDS 统计：Node 0 发送包数/字节正确；Node 1 接收包数/字节与发送一致。

---

## 3. 阶段二：接收路径经 PDS/PDC（已完成，PDC 收包处理暂跳过）

### 3.1 代码变更

| 任务 | 文件 | 实施内容 |
|------|------|----------|
| 2.1 设备收包交 PDS Manager | `soft-ue-net-device.cc` | ReceivePacket 调用 `m_pdsManager->ProcessReceivedPacket(packet, sourceFep, destFep)`，不再直接入队与 ProcessReceiveQueue |
| 2.2 ProcessReceivedPacket 按 pdc_id 分发 | `pds-manager.cc` | 解析 PDS 头得 pdc_id，EnsureReceivePdc(pdcId, sourceFep) 被动创建接收侧 PDC；可选调用 pdc->HandleReceivedPacket（当前为规避崩溃暂不调用） |
| 2.3 交付应用层 | `pds-manager.cc` / `soft-ue-net-device.cc` | ProcessReceivedPacket 去 PDS 头后调用 `m_netDevice->DeliverReceivedPacket(payload)`；DeliverReceivedPacket 入队并调用 ProcessReceiveQueue，最终触发应用 HandleRead |

### 3.2 已知限制

- **pdc->HandleReceivedPacket**：在 E2E 实验中约第 11 包时发生段错误，暂时注释调用。接收路径仍满足：Channel → Device → PdsManager::ProcessReceivedPacket → 解析 pdc_id、EnsureReceivePdc、去头、DeliverReceivedPacket → 应用。PDC 存在性与 pdc_id 分发已实现；每包经 PDC 的收包处理待后续排查（可能与 PDC 队列/调度或 Ipdc/PdcBase 收包逻辑有关）。

### 3.3 验证

- 运行 `./ns3 run uec-e2e-concepts`：接收路径日志为「Device ReceivePacket → ProcessReceivedPacket → … → DeliverReceivedPacket → ProcessReceiveQueue → HandleRead」。
- 20 包收发一致；PDS 统计（Node 1）Received Packets、Total Bytes Received、Average Latency 等正常。

---

## 4. 阶段三：控制面占位（未实施）

计划中的 Tx rsp、Error event、Eager size/Pause、Rx req/Rx rsp 等占位未在本轮实施；可在阶段一、二稳定后于现有链路上挂接。

---

## 5. 文档与回归

- **文档**：本文件为 [07-UEC端到端概念实验与图解.md](07-UEC端到端概念实验与图解.md) 第六节「改进与实验计划」的落地记录。
- **回归**：每阶段后运行 `./ns3 run uec-e2e-concepts` 与 `./ns3 run Soft-UE`（若保留）；单元测试可通过 `./ns3 build` 后运行对应 test 可执行文件进行验证。

---

## 6. 关键文件索引

- [src/soft-ue/model/network/soft-ue-net-device.cc](src/soft-ue/model/network/soft-ue-net-device.cc)：Send → DispatchPacket；ReceivePacket → ProcessReceivedPacket；TransmitToChannel；DeliverReceivedPacket
- [src/soft-ue/model/pds/pds-manager.cc](src/soft-ue/model/pds/pds-manager.cc)：ProcessSesRequest → DispatchPacket；ProcessReceivedPacket（解析 pdc_id、EnsureReceivePdc、去头、DeliverReceivedPacket）；EnsureReceivePdc；AllocatePdc（PDC 初始化与 Activate）
- [src/soft-ue/model/pdc/ipdc.cc](src/soft-ue/model/pdc/ipdc.cc)：SendPacket 立即 TransmitPacket；TransmitPacket 使用 TransmitToChannel

---

## 7. 日志与参考图对齐说明

**为何之前日志“感觉怪怪的”？**  
参考图（Core Components / System Architecture）中，数据路径应**显式经过 PDS Manager 与 PDC 实例**；之前 E2E 日志只打出「Device → 经 PDS Manager DispatchPacket」和「Channel Transmit」，中间**没有**打出 PDS Manager 的 AllocatePdc、SendPacketThroughPdc 以及 PDC 的 TransmitPacket，所以看起来像“设备直接到信道”，与图中 **SES → PDS Manager → PDC → Channel** 不一致。

**发送路径顺序（先 SES，后 PDS/PDC）：**  
参考图中发送路径为 **SES → PDS Manager → PDC → Channel**。当前实现也是：① App 构造包；② App 准备 PDS 头字段（仅是应用侧准备 pdc_id/seq/SOM/EOM，**尚未经 PDS 层**）；③ **SES 元数据 + ProcessSendRequest（校验）**；④ SES 校验通过后 App 调用 device->Send()；⑤ 之后才经 **PDS Manager → PDC → Channel**。因此是 **先过 SES、再过 PDS/PDC**；② 的「PDS 头」易被误读为「先过 PDS」，已改为「App 准备 PDS 头字段（尚未经 PDS 层）」以区分。

**已做的对齐：**  
在 PdsManager 与 Ipdc 中增加了 `[UEC-E2E]` 日志，使发送/接收路径在日志中可见：

- **发送路径**（与图一致）：  
  `Device Send` → `[PDS] PDS Manager AllocatePdc pdc_id=… → SendPacketThroughPdc（经 PDC 发）` → `[PDS] SendPacketThroughPdc pdc_id=… → PDC 实例发送` → `[PDC] PDC pdc_id=… TransmitPacket → TransmitToChannel（到信道）` → `Channel Transmit`。
- **接收路径**（与图部分一致）：  
  `Channel ReceivePacket` → `Device ReceivePacket` → `[PDS] ProcessReceivedPacket 解析 pdc_id=… 按 pdc_id 分发（收端 PDC）` → `[PDS] 去 PDS 头 → DeliverReceivedPacket 递交应用层（收端未经 SES 层）` → `Device ProcessReceiveQueue` → `App HandleRead`。

**与参考图的剩余差异：**

- **收端 SES**：图中收端应为 **PDS Manager → PDC → Right SES（Rx req）→ App**。当前实现是 PDS Manager 去头后直接 `DeliverReceivedPacket` → 设备回调 → App，**收端未经过 SesManager::ProcessReceiveRequest**，因此日志中注明「收端未经 SES 层」。
- **控制面**：图中 Tx rsp、Eager size、Pause、Error event、Rx req/Rx rsp 等信令尚未实现，仅数据路径与部分日志与图对齐。

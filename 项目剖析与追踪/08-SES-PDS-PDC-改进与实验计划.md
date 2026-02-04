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

## 3. 阶段二：接收路径经 PDS/PDC（已完成，含 B2 收端经 PDC）

### 3.1 代码变更
| 任务 | 文件 | 实施内容 |
|------|------|----------|
| 2.1 设备收包交 PDS Manager | `soft-ue-net-device.cc` | ReceivePacket 调用 `m_pdsManager->ProcessReceivedPacket(packet, sourceFep, destFep)`，不再直接入队与 ProcessReceiveQueue |
| 2.2 ProcessReceivedPacket 按 pdc_id 分发 | `pds-manager.cc` | 解析 PDS 头得 pdc_id，EnsureReceivePdc(pdcId, sourceFep) 被动创建接收侧 PDC；调用 `pdc->HandleReceivedPacket(packet->Copy(), sourceEndpoint)`（B2 已修复） |
| 2.3 交付应用层 | `pds-manager.cc` / `soft-ue-net-device.cc` | ProcessReceivedPacket 去 PDS 头后调用 `m_netDevice->DeliverReceivedPacket(payload)`；DeliverReceivedPacket 入队并调用 ProcessReceiveQueue，最终触发应用 HandleRead |

### 3.2 B2 排查记录（收端 PDC 约第 11 包崩溃）

- **根因**：Ipdc 存在「双队列」——PdcBase::HandleReceivedPacket 将 packet 入队到 PdcBase::m_receiveQueue（DropTailQueue），Ipdc::HandleReceivedPacket 再调用 EnqueuePacket 将同一 packet 入队到 Ipdc::m_receiveQueue（std::queue<QueuedPacket>），同一 packet 两次入队导致生命周期/访问违规，约第 11 包时触发段错误。
- **修复**：在 PdcBase 中新增 `ValidateAndRecordReceivedPacket(packet, sourceFep)`（仅校验与 RecordPacketEntry，不入队）；Ipdc::HandleReceivedPacket 改为调用 `ValidateAndRecordReceivedPacket` 后**仅**入队到 Ipdc 的 m_receiveQueue，不再调用 PdcBase::HandleReceivedPacket（避免入队到 base 的 DropTailQueue）。收端路径传 `packet->Copy()` 给 PDC，去头仅作用于原 packet，交付仍由 PDS → SES → App 一次完成。
- **验证**：`./ns3 run uec-e2e-concepts -- --packetCount=25` 无崩溃；接收路径日志出现「[PDC] 收端 HandleReceivedPacket pdc_id=… → 入队（随后 PDS → SES → App）」。

### 3.3 验证

- 运行 `./ns3 run uec-e2e-concepts`：接收路径日志为「Device ReceivePacket → ProcessReceivedPacket → [PDC] 收端 HandleReceivedPacket → … → SesManager::ProcessReceiveRequest → DeliverReceivedPacket → ProcessReceiveQueue → HandleRead」。
- 25 包及以上收发一致；PDS 统计（Node 1）Received Packets、Total Bytes Received、Average Latency 等正常。

---

## 4. 阶段三：控制面占位（已实施）

- **B3**：在 PDS Manager 与 SES 间增加占位：发送完成后 `NotifyTxResponse`（Tx rsp）；错误时 `NotifyPdsErrorEvent`（Error event）；收端 `ProcessReceiveRequest` 内打日志「Rx req/Rx rsp (placeholder)」；流控 `NotifyEagerSize`、`NotifyPause` 占位（可先打日志）。见 `ses-manager.cc/h`、`pds-manager.cc`。

## 5. 功能对齐表（与三条参考定义）

| 定义项 | 参考定义 | 实现状态 | 对应任务 |
|--------|----------|----------|----------|
| **SES 端点寻址** | Endpoint addressing | 已对齐 | ValidateOperationMetadata、ProcessSendRequest |
| **SES 授权** | Authorization | 占位 | C2：ValidateAuthorization（当前恒 true） |
| **SES 消息类型/语义头** | Message types, semantic header | 部分对齐 | ExtendedOperationMetadata、InitializeSesHeader |
| **SES 事务→多包** | Breaks down each transaction into multiple packets | 已对齐 | A1/A2/A3：CalculatePacketCount、RequiresFragmentation、ProcessSendRequest(metadata, packet) 拆包 |
| **PDS PDC 分配** | Allocation of PDCs | 已对齐 | AllocatePdc、EnsureReceivePdc |
| **PDS 非 PDC 错误上报 SES** | Handling error events not associated with a specific PDC | 已对齐 | C1：NotifyPdsErrorEvent(0, …) |
| **PDS 将 SES 包分配给 PDC** | Assignment of SES packets to PDCs | 已对齐 | DispatchPacket |
| **PDS 状态/流控对 SES 可见** | Eager size, Pause | 占位 | B3：NotifyEagerSize、NotifyPause（占位） |
| **收端经 SES** | PDS → (PDC) → Right SES (Rx req) → App | 已对齐 | B1：ProcessReceivedPacket → ProcessReceiveRequest → DeliverReceivedPacket |
| **收端经 PDC 处理** | pdc->HandleReceivedPacket | 已对齐 | B2：双队列修复后恢复调用，路径 PDS → PDC → SES → App |
| **控制面 Tx rsp / Error / Rx req·rsp** | Tx rsp, Error event, Rx req/Rx rsp | 占位 | B3：NotifyTxResponse、NotifyPdsErrorEvent、Rx req/Rx rsp 日志占位 |
| **控制面 Eager size / Pause** | Eager size, Pause | 占位 | B3：DispatchPacket 内 NotifyEagerSize(1372)、NotifyPause(true/false) |

## 6. IPDC vs TPDC 与可靠性（C3）

- **IPDC**（Unreliable）：`src/soft-ue/model/pdc/ipdc.cc`，当前 E2E 使用 IPDC（pdc_id 1..maxPdcCount），无 RTO/重传/ACK。
- **TPDC**（Reliable）：`src/soft-ue/model/pdc/tpdc.cc`，含 RTO、重传、ACK、duplicate ACK 处理；可靠性/顺序/去重在 TPDC 中实现。若需在 E2E 或单测中体现可靠性，可选用 TPDC（pdc_id >= MAX_PDC）或增加一条 TPDC 路径/文档说明。

---

## 7. 文档与回归

- **文档**：本文件为 [07-UEC端到端概念实验与图解.md](07-UEC端到端概念实验与图解.md) 第六节「改进与实验计划」的落地记录。
- **回归**：每阶段后运行 `./ns3 run uec-e2e-concepts` 与 `./ns3 run Soft-UE`（若保留）；单元测试可通过 `./ns3 build` 后运行对应 test 可执行文件进行验证。

---

## 8. 关键文件索引

- [src/soft-ue/model/network/soft-ue-net-device.cc](src/soft-ue/model/network/soft-ue-net-device.cc)：Send → DispatchPacket；ReceivePacket → ProcessReceivedPacket；TransmitToChannel；DeliverReceivedPacket
- [src/soft-ue/model/pds/pds-manager.cc](src/soft-ue/model/pds/pds-manager.cc)：ProcessSesRequest → DispatchPacket；ProcessReceivedPacket（解析 pdc_id、EnsureReceivePdc、去头、DeliverReceivedPacket）；EnsureReceivePdc；AllocatePdc（PDC 初始化与 Activate）
- [src/soft-ue/model/pdc/ipdc.cc](src/soft-ue/model/pdc/ipdc.cc)：SendPacket 立即 TransmitPacket；TransmitPacket 使用 TransmitToChannel

---

## 9. 日志与参考图对齐说明

**为何之前日志“感觉怪怪的”？**  
参考图（Core Components / System Architecture）中，数据路径应**显式经过 PDS Manager 与 PDC 实例**；之前 E2E 日志只打出「Device → 经 PDS Manager DispatchPacket」和「Channel Transmit」，中间**没有**打出 PDS Manager 的 AllocatePdc、SendPacketThroughPdc 以及 PDC 的 TransmitPacket，所以看起来像“设备直接到信道”，与图中 **SES → PDS Manager → PDC → Channel** 不一致。

**发送路径顺序（先 SES，后 PDS/PDC）：**  
参考图中发送路径为 **SES → PDS Manager → PDC → Channel**。当前实现也是：① App 构造包；② App 准备 PDS 头字段（仅是应用侧准备 pdc_id/seq/SOM/EOM，**尚未经 PDS 层**）；③ **SES 元数据 + ProcessSendRequest（校验）**；④ SES 校验通过后 App 调用 device->Send()；⑤ 之后才经 **PDS Manager → PDC → Channel**。因此是 **先过 SES、再过 PDS/PDC**；② 的「PDS 头」易被误读为「先过 PDS」，已改为「App 准备 PDS 头字段（尚未经 PDS 层）」以区分。

**已做的对齐：**  
在 PdsManager 与 Ipdc 中增加了 `[UEC-E2E]` 日志，使发送/接收路径在日志中可见：

- **发送路径**（与图一致）：  
  `Device Send` → `[PDS] PDS Manager AllocatePdc pdc_id=… → SendPacketThroughPdc（经 PDC 发）` → `[PDS] SendPacketThroughPdc pdc_id=… → PDC 实例发送` → `[PDC] PDC pdc_id=… TransmitPacket → TransmitToChannel（到信道）` → `Channel Transmit`。
- **接收路径**（与图一致）：  
  `Channel ReceivePacket` → `Device ReceivePacket` → `[PDS] ProcessReceivedPacket 解析 pdc_id=…` → `[PDC] 收端 HandleReceivedPacket pdc_id=… → 入队` → `[PDS] 去 PDS 头 → SesManager::ProcessReceiveRequest（收端经 SES → App）` → `[SES] ProcessReceiveRequest … → DeliverReceivedPacket` → `Device ProcessReceiveQueue` → `App HandleRead`。

**与参考图审阅表（剩余差异已闭合）：**

| 参考图要素 | 实现位置 | 验证方式 |
|------------|----------|----------|
| 收端 PDS → PDC → SES → App | ProcessReceivedPacket → pdc->HandleReceivedPacket(packet->Copy()) → 去头 → ProcessReceiveRequest → DeliverReceivedPacket | E2E 日志 `[PDC] 收端 HandleReceivedPacket`、`[SES] ProcessReceiveRequest` |
| Tx rsp | DispatchPacket 成功后 NotifyTxResponse(pdcId) | 日志 `[Control] Tx rsp (placeholder)` |
| Error event | HandlePdcError / ProcessSesRequest 校验失败等 NotifyPdsErrorEvent | 日志 `[Control] Error event (placeholder)` |
| Eager size / Pause | DispatchPacket 内 NotifyEagerSize(1372)、NotifyPause(true/false) | 日志 `[Control] Eager size (placeholder)`、`Pause (placeholder)` |
| Rx req/Rx rsp | ProcessReceiveRequest 内占位日志 | 日志 `[Control] Rx req/Rx rsp (placeholder)` |

**剩余差异验收**：运行 `./ns3 run uec-e2e-concepts -- --packetCount=25` 及 `--largeTransactionSize=4000 --packetCount=2` 无崩溃；收发包一致；控制面占位日志齐全。回归命令：`./ns3 build`，`./ns3 run uec-e2e-concepts -- --packetCount=20`。

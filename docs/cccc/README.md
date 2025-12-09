# Soft-UE 技术文档索引

## 文档概述

本文档集合包含Soft-UE ns-3模块的完整技术文档，涵盖了数据流程、问题解决、系统架构和可视化图表。

## 文档列表

### 1. 数据流程文档
**文件**: `01-dataflow.md`

详细描述了Soft-UE点到点数据传输的完整流程：
- 流量生成机制
- SES层处理流程
- PDS层映射逻辑
- PDC建立和维护
- 设备层传输
- 接收端处理
- 完整数据流时序
- 错误处理机制

### 2. 问题与解决方案文档
**文件**: `02-problems-solutions.md`

记录了开发和测试过程中遇到的所有关键问题及其解决方案：
- SES层空指针问题
- 服务器端包接收问题
- PDS管理器通道发送功能缺失
- 重复传输和循环调用问题
- 应用程序时间窗口不足问题
- 技术债务和代码质量改进
- 性能优化建议

### 3. 项目概述文档
**文件**: `project-overview.md`

提供项目的总体概述和关键成就：
- 项目定位与目标
- 技术突破和性能数据
- 核心架构设计
- 开发里程碑和成果统计

### 4. 图表文档
**文件**: `03-diagrams.md`

包含所有技术图表的Mermaid源代码：
- 系统架构关系图
- 模块依赖关系图
- 点对点连接拓扑图
- 端到端数据传输时序图
- 管理器初始化时序图
- PDC生命周期时序图
- 开发里程碑甘特图
- 包传输处理时间线
- 包处理状态机
- PDC状态机
- 核心类关系图
- ns-3集成部署图

## 快速参考

### 核心组件映射
```
应用层: SoftUeFullApp
   ↓
SES层: SesManager + MsnTable
   ↓
PDS层: PdsManager + PdsStatistics
   ↓
PDC层: TPDC + IPDC + RtoTimer
   ↓
设备层: SoftUeNetDevice
   ↓
通道层: SoftUeChannel
```

### 关键文件位置
```
核心实现: src/soft-ue/model/
测试代码: scratch/Soft-UE/Soft-UE.cc
Helper层: src/soft-ue/helper/soft-ue-helper.h/cc
文档: docs/cccc/
```

### 性能指标
```
吞吐量: 10包/秒
可靠性: 100% (1000/1000包成功)
延迟: 微秒级端到端延迟
可扩展性: 已验证1000包无性能下降
```

## Mermaid图表使用说明

所有图表都使用Mermaid语法编写，可以在支持Mermaid的Markdown查看器中渲染显示。

### 渲染工具推荐
- VS Code + Mermaid Preview插件
- GitHub README (自动渲染)
- Mermaid Live Editor (https://mermaid.live/)
- Typora (Markdown编辑器)

### 图表类型
- **流程图**: 数据流程、状态机
- **时序图**: 交互时序、生命周期
- **关系图**: 架构图、依赖关系
- **甘特图**: 项目时间线、处理时间
- **类图**: 面向对象设计

## 技术术语表

| 缩写 | 全称 | 说明 |
|------|------|------|
| SES | Semantic Execution Services | 语义执行服务层 |
| PDS | Packet Delivery Sub-layer | 包分发子层 |
| PDC | Packet Delivery Context | 包分发上下文 |
| FEP | Fabric End Point | 结构端点 |
| MSN | Message Sequence Number | 消息序列号 |
| RTO | Retransmission Timeout | 重传超时 |
| TPDC | Transport PDC | 传输PDC |
| IPDC | Inexpensive PDC | 非可靠PDC |

## 开发里程碑

### 已完成功能
- ✅ 完整的Ultra Ethernet三层架构实现
- ✅ ns-3框架完全集成
- ✅ 端到端通信100%可靠性
- ✅ 大规模数据传输验证（1000包）
- ✅ 完善的错误处理机制
- ✅ 详细的统计收集功能

### 技术成就
- **零包丢失**: 所有测试规模下100%成功率
- **线性可扩展性**: 性能不随包数量下降
- **完整协议栈**: 从应用层到物理层的完整实现
- **工程级质量**: 符合生产环境标准

## 维护说明

### 文档更新
- 当代码结构发生变化时，更新`01-dataflow.md`
- 发现新问题时，更新`02-problems-solutions.md`
- 架构变更时，更新`03-diagrams.md`

### 版本控制
- 所有文档与代码库同步版本控制
- 重要变更记录在相应文档中
- 使用Markdown格式便于版本对比

### 贡献指南
- 遵循现有文档结构和格式
- 保持技术术语的一致性
- 确保Mermaid图表的正确性
- 提供具体的技术细节和数据

---

**文档版本**: v1.0
**最后更新**: 2025-12-09
**维护者**: Soft UE Project Team
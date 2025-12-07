# Soft-UE 到 ns-3 的技术迁移规划

## 项目概述

本文档详细描述了将 Soft-UE（Ultra Ethernet 软件原型）从独立的 C++ 程序迁移到 ns-3 离散事件网络仿真环境的技术规划。

## 原始架构分析

### 核心组件层次结构

基于对原始代码的分析，Soft-UE 采用分层架构设计：

```
应用层接口 (Application Layer Interface)
    ↓
语义子层 SES (Semantic Sub-layer)
    ↓
分组传输层 PDS (Packet Delivery Sub-layer)
    ↓
包传输上下文 PDC (Packet Delivery Context)
    ↓
网络层适配 (Network Layer Adaptation)
```

### 关键组件功能

#### 1. SES (Semantic Sub-layer)
- **功能**: 端点寻址、授权、消息类型定义、协议和语义头格式
- **核心类**: `SESManager`
- **主要职责**:
  - 操作元数据 (`OperationMetadata`) 处理
  - 标准SES头 (`SES_Standard_Header`) 初始化
  - 长消息分片处理
  - MSN表维护和消息顺序验证
  - 内存区域管理和权限验证

#### 2. PDS (Packet Delivery Sub-layer)
- **功能**: 分组接收、分发到对应PDC、处理来自SES层的命令和数据
- **核心组件**:
  - `PDSManager`: 通用功能管理
  - PDC分配算法
  - 错误事件处理
  - SES分组分配到PDC

#### 3. PDC (Packet Delivery Context)
- **功能**: 动态建立的FEP到FEP连接，提供可靠性、排序、重复包消除、拥塞管理
- **类型**:
  - `IPDC`: 不可靠PDC (1-16)
  - `TPDC`: 可靠PDC (17-32)
- **核心实现**: `IPDC`, `TPDC`

#### 4. 网络层适配
- **原始实现**: UDP网络层和软件环回链路
- **功能**: 提供底层数据传输能力

## ns-3 迁移设计

### 设计原则

1. **保持协议完整性**: 确保Ultra Ethernet协议规范完整实现
2. **适配仿真环境**: 利用ns-3的离散事件调度和节点模型
3. **模块化设计**: 遵循ns-3模块结构，便于集成和扩展
4. **测试驱动**: 每个组件都有对应的测试用例
5. **性能优化**: 适应大规模仿真场景

### ns-3 模块结构设计

```
src/soft-ue/
├── CMakeLists.txt                    # ns-3模块构建配置
├── doc/                             # 文档目录
│   └── soft-ue.rst                  # Sphinx文档
├── examples/                        # 示例应用
│   ├── soft-ue-simple-example.cc    # 基础示例
│   ├── ses-layer-test.cc            # SES层测试
│   └── pds-layer-test.cc            # PDS层测试
├── helper/                          # 辅助类
│   ├── soft-ue-helper.cc            # 主要辅助类
│   └── soft-ue-helper.h
├── model/                           # 核心实现
│   ├── ses/                         # SES层实现
│   │   ├── ses-manager.cc
│   │   ├── ses-manager.h
│   │   ├── operation-metadata.h
│   │   └── msn-entry.h
│   ├── pds/                         # PDS层实现
│   │   ├── pds-manager.cc
│   │   ├── pds-manager.h
│   │   └── pds-common.h
│   ├── pdc/                         # PDC实现
│   │   ├── pdc-base.cc
│   │   ├── pdc-base.h
│   │   ├── ipdc.cc
│   │   ├── ipdc.h
│   │   ├── tpdc.cc
│   │   ├── tpdc.h
│   │   └── rto-timer/
│   ├── network/                     # 网络层适配
│   │   ├── soft-ue-net-device.cc
│   │   ├── soft-ue-net-device.h
│   │   └── soft-ue-channel.cc
│   └── common/                      # 通用定义
│       ├── transport-layer.h       # 传输层协议定义
│       └── soft-ue-packet-tag.h    # 数据包标签
├── test/                           # 测试代码
│   ├── ses-manager-test.cc
│   ├── pds-manager-test.cc
│   ├── ipdc-test.cc
│   ├── tpdc-test.cc
│   └── soft-ue-integration-test.cc
└── binding/                        # Python绑定（可选）
    └── docsoft-ue.rst
```

### 核心类设计

#### 1. SoftUeNetDevice (主要网络设备)
```cpp
class SoftUeNetDevice : public NetDevice {
private:
    Ptr<SesManager> m_sesManager;        // SES管理器
    Ptr<PdsManager> m_pdsManager;        // PDS管理器
    Ptr<Node> m_node;                    // 关联节点
    Mac48Address m_address;             // MAC地址
    // ... 其他成员
};
```

#### 2. SesManager (SES管理器)
```cpp
class SesManager : public Object {
public:
    // 处理发送请求
    void ProcessSendRequest(const OperationMetadata& metadata);

    // 处理接收到的请求包
    void ProcessReceiveRequest(const PdcSesRequest& request);

    // 处理接收到的响应包
    void ProcessReceiveResponse(const PdcSesResponse& response);

private:
    Ptr<PdsManager> m_pdsManager;
    std::queue<OperationMetadata> m_requestQueue;
    std::unordered_map<uint64_t, MsnEntry> m_msnTable;
};
```

#### 3. PdsManager (PDS管理器)
```cpp
class PdsManager : public Object {
public:
    // 分配PDC
    uint16_t AllocatePdc(uint32_t destFa, uint8_t tc, uint8_t dm);

    // 释放PDC
    void ReleasePdc(uint16_t pdcId);

    // 分发包到PDC
    void DispatchPacket(const SesPdsRequest& request);

private:
    std::array<Ptr<PdcBase>, MAX_PDC> m_pdcs;
    PdcAllocator m_allocator;
};
```

#### 4. PdcBase (PDC基类)
```cpp
class PdcBase : public Object {
public:
    virtual void SendPacket(const SesPdsRequest& request) = 0;
    virtual void ReceivePacket(const Packet& packet) = 0;
    virtual bool IsOpen() const = 0;

protected:
    uint16_t m_pdcId;
    PdcType m_type;
    bool m_isOpen;
};
```

### 关键技术挑战与解决方案

#### 1. 离散事件调度适配
**挑战**: 原始代码使用同步编程模型，ns-3使用异步事件调度
**解决方案**:
- 使用ns-3的`Simulator::Schedule()`进行定时器管理
- 将回调机制适配为事件调度
- 使用`EventId`管理异步操作

#### 2. 数据包处理
**挑战**: 原始代码使用自定义数据结构，ns-3使用Packet类
**解决方案**:
- 创建`SoftUePacketTag`存储协议头信息
- 实现序列化/反序列化方法
- 保持原始协议字段的完整性

#### 3. 内存管理
**挑战**: ns-3使用引用计数，原始代码使用手动内存管理
**解决方案**:
- 使用Ptr智能指针管理对象生命周期
- 避免显式内存分配/释放
- 适配RAII模式

#### 4. 并发处理
**挑战**: 原始代码使用队列处理并发，ns-3是单线程事件驱动
**解决方案**:
- 将队列转换为事件调度
- 使用回调函数处理异步消息
- 保持处理顺序的确定性

## 实施计划

### 阶段1: 基础设施搭建 (1-2周)
1. **创建模块结构**
   - CMakeLists.txt配置
   - 基础目录结构
   - 文档框架

2. **移植协议定义**
   - Transport_Layer.hpp定义
   - 数据结构适配
   - 常量和枚举定义

3. **基础类实现**
   - SoftUeNetDevice框架
   - PdcBase基类
   - 通用工具类

### 阶段2: PDC层实现 (2-3周)
1. **IPDC实现**
   - 基础PDC功能
   - 数据包发送/接收
   - 错误处理

2. **TPDC实现**
   - 可靠传输机制
   - 重传逻辑
   - ACK/NACK处理

3. **RTO定时器**
   - 定时器管理
   - 超时处理
   - 动态调整算法

4. **单元测试**
   - IPDC功能测试
   - TPDC可靠性测试
   - 性能测试

### 阶段3: PDS层实现 (2-3周)
1. **PDS管理器**
   - PDC分配算法
   - 资源管理
   - 错误事件处理

2. **包分发机制**
   - SES到PDC映射
   - 负载均衡
   - 拥塞控制

3. **集成测试**
   - PDS层功能测试
   - 多PDC协作测试

### 阶段4: SES层实现 (3-4周)
1. **SES管理器核心**
   - 操作元数据处理
   - SES头初始化
   - 消息分片/重组

2. **MSN表管理**
   - 消息顺序跟踪
   - 内存区域管理
   - 权限验证

3. **应用接口**
   - 发送接口
   - 接收回调
   - 错误处理

### 阶段5: 网络层集成 (1-2周)
1. **NetDevice集成**
   - 与ns-3网络栈集成
   - 数据包标签处理
   - 地址管理

2. **Channel适配**
   - 点对点连接
   - 多播支持
   - 延迟和带宽建模

### 阶段6: 示例和测试 (2周)
1. **示例应用**
   - 基础通信示例
   - 性能测试示例
   - 多节点场景

2. **完整测试套件**
   - 单元测试
   - 集成测试
   - 性能基准测试

## 验证标准

### 功能正确性
- [ ] SES层功能完整实现
- [ ] PDS层包分发正确
- [ ] PDC层可靠传输
- [ ] 与原始规范一致

### 性能指标
- [ ] 单节点处理延迟 < 1ms
- [ ] 支持至少1000个并发PDC
- [ ] 内存使用合理
- [ ] 可扩展至100+节点仿真

### 代码质量
- [ ] 测试覆盖率 > 80%
- [ ] 遵循ns-3编码规范
- [ ] 完整的Doxygen文档
- [ ] 通过所有静态分析检查

## 风险评估与缓解

### 技术风险
1. **复杂度风险**: Ultra Ethernet协议复杂度高
   - *缓解*: 分阶段实施，先实现基础功能

2. **性能风险**: 大规模仿真性能问题
   - *缓解*: 早期性能测试，优化关键路径

3. **兼容性风险**: ns-3版本兼容性
   - *缓解*: 使用稳定API，避免依赖内部实现

### 项目风险
1. **时间风险**: 实施时间可能超出预期
   - *缓解*: 采用敏捷开发，定期评估进度

2. **资源风险**: 开发资源不足
   - *缓解*: 优先实现核心功能，可选功能后续迭代

## 后续扩展

### 短期扩展 (3-6个月)
- Python绑定支持
- 更多传输模式支持 (UUD, RUDI)
- 图形化监控工具

### 长期扩展 (6-12个月)
- 与真实网络设备集成
- 更多Ultra Ethernet特性支持
- 分布式仿真支持

---

本规划文档将随着实施进展持续更新和完善。
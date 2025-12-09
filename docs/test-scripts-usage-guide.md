# Soft-UE 测试脚本使用指南

## 概述

本指南详细说明了Soft-UE协议栈测试脚本的使用方法、配置选项和结果解读。

## 文件结构

```
Soft-UE-ns3/
├── scripts/                          # 测试脚本目录
│   ├── run-soft-ue-tests.sh         # 完整测试套件
│   ├── soft-ue-validation-test.sh   # 验证测试脚本
│   └── quick-test.sh                # 快速测试脚本
├── scratch/                          # 测试程序目录
│   └── soft-ue-integration-test.cc  # 集成测试程序
├── docs/
│   ├── test-scripts-documentation.md # 详细技术文档
│   └── test-scripts-usage-guide.md   # 本使用指南
└── test-results/                     # 测试结果目录（自动创建）
    ├── logs/                         # 详细日志
    ├── coverage/                     # 覆盖率报告
    └── performance/                  # 性能测试结果
```

## 快速开始

### 1. 基本验证

运行快速测试来验证基本功能：

```bash
# 进入项目目录
cd /home/makai/Soft-UE-ns3

# 运行快速测试
./scripts/quick-test.sh
```

预期输出：
```
================================================
        Soft-UE Quick Test Script
================================================
[INFO] Soft-UE library found
[INFO] Testing component creation...
[SUCCESS] ✅ Component creation test passed
[INFO] Running integration test...
[SUCCESS] ✅ Integration test passed
[SUCCESS] 🎉 All integration tests passed!
[INFO] Testing basic packet flow...
[SUCCESS] ✅ Basic functionality test passed
================================================
```

### 2. 完整测试套件

运行全面的测试套件：

```bash
# 运行所有测试（包括性能和压力测试）
./scripts/run-soft-ue-tests.sh

# 运行测试但跳过耗时的性能测试
./scripts/run-soft-ue-tests.sh --no-performance --no-stress

# 静默模式运行
./scripts/run-soft-ue-tests.sh --quiet
```

### 3. 验证测试

运行专项验证测试：

```bash
# 运行验证测试套件
./scripts/soft-ue-validation-test.sh
```

## 测试类型详解

### 1. 单元测试

**目的**: 验证各个组件的独立功能

**包含测试**:
- SES管理器功能测试
- PDS管理器功能测试
- PDC层可靠性测试
- 网络设备基础功能测试

**执行命令**:
```bash
# 编译并运行单元测试
cd build
make test/ses-manager-test
make test/pds-manager-test
make test/pds-full-test

# 运行所有单元测试
./test/ses-manager-test
./test/pds-manager-test
./test/pds-full-test
```

### 2. 集成测试

**目的**: 验证组件间的协作和端到端通信

**测试场景**:
- 端到端数据流验证
- 多节点通信测试
- 错误处理和恢复
- 协议栈层间协调

**执行命令**:
```bash
# 运行集成测试
./ns3 run soft-ue-integration-test

# 详细输出模式
./ns3 run "soft-ue-integration-test --verbose=true"
```

**预期结果**:
```
=== Soft-UE Protocol Stack Integration Test ===
============================================================
Running: Basic Component Creation
============================================================
✅ TEST PASSED: Basic Component Creation

Running: SES-PDS Integration
============================================================
✅ TEST PASSED: SES-PDS Integration

Running: End-to-End Communication
============================================================
✅ TEST PASSED: End-to-End Communication

============================================================
Integration Test Summary
============================================================
Total Tests: 5
Passed: 5
Failed: 0
Success Rate: 100%
🎉 ALL INTEGRATION TESTS PASSED!
```

### 3. 性能测试

**目的**: 验证系统在负载下的性能表现

**测试指标**:
- 吞吐量 (Gbps)
- 延迟 (ms)
- 内存使用
- CPU利用率

**测试配置**:
```bash
# 小包性能测试
--packets=1000 --size=512

# 标准包性能测试
--packets=10000 --size=1500

# 大包性能测试
--packets=100000 --size=8192
```

### 4. 压力测试

**目的**: 验证系统在极限条件下的稳定性

**测试场景**:
- 大规模节点仿真 (100-1000节点)
- 高并发PDC连接 (1000+连接)
- 长时间稳定性测试 (60分钟+)

**执行命令**:
```bash
# 大规模节点测试
./ns3 run "soft-ue-stress-test --nodes=100 --duration=60"

# 高并发测试
./ns3 run "soft-ue-stress-test --concurrency=1000 --packets=100000"
```

## 配置选项

### run-soft-ue-tests.sh 选项

| 选项 | 说明 | 默认值 |
|------|------|--------|
| `--no-coverage` | 禁用代码覆盖率分析 | 启用 |
| `--no-performance` | 禁用性能测试 | 启用 |
| `--no-stress` | 禁用压力测试 | 启用 |
| `--quiet` | 静默模式输出 | 详细输出 |
| `--help` | 显示帮助信息 | - |

### 示例用法

```bash
# 快速验证（仅单元和集成测试）
./scripts/run-soft-ue-tests.sh --no-performance --no-stress

# CI/CD环境（静默模式，生成覆盖率报告）
./scripts/run-soft-ue-tests.sh --quiet

# 完整测试（包括所有性能和压力测试）
./scripts/run-soft-ue-tests.sh
```

## 结果解读

### 1. 测试通过标准

**单元测试**: 100% 通过率
- 所有基础功能必须正常工作
- 边界条件处理正确
- 错误处理机制有效

**集成测试**: 100% 通过率
- 端到端通信成功
- 数据完整性保证
- 错误恢复正常

**性能测试**: 达到基准指标
- 吞吐量 > 1 Gbps (标准包)
- 平均延迟 < 10 ms
- 内存使用 < 100 MB

**压力测试**: 稳定性验证
- 大规模节点通信正常
- 长时间运行无内存泄漏
- 高并发处理无崩溃

### 2. 日志文件分析

**测试日志位置**: `test-results/logs/`

**关键日志内容**:
```
test-results/logs/
├── ses-manager-test.log              # SES管理器测试日志
├── pds-manager-test.log              # PDS管理器测试日志
├── pds-full-test.log                 # PDS完整功能测试日志
├── integration-test.log              # 集成测试日志
├── perf-*.log                        # 性能测试日志
└── stress-*.log                      # 压力测试日志
```

**分析命令**:
```bash
# 查看测试失败原因
grep -i "failed\|error" test-results/logs/*.log

# 查看性能指标
grep "throughput\|latency\|memory" test-results/logs/perf-*.log

# 查看压力测试结果
grep "nodes\|concurrency\|stability" test-results/logs/stress-*.log
```

### 3. 覆盖率报告

**HTML报告位置**: `test-results/coverage/html/index.html`

**关键指标**:
- 总体代码覆盖率: 应 > 80%
- 核心组件覆盖率: 应 > 90%
- 测试覆盖范围: 包含所有公共接口

**查看报告**:
```bash
# 在浏览器中打开
firefox test-results/coverage/html/index.html

# 命令行查看摘要
lcov --summary test-results/coverage/coverage.info
```

## 故障排除

### 常见问题及解决方案

#### 1. 编译失败

**错误信息**:
```
error: 'SoftUeHelper' was not declared in this scope
```

**解决方案**:
```bash
# 确保项目正确构建
cd /home/makai/Soft-UE-ns3
./ns3 configure --enable-examples --enable-tests
./ns3 build
```

#### 2. 测试超时

**错误信息**:
```
Timeout: test execution exceeded time limit
```

**解决方案**:
```bash
# 增加超时时间
timeout 300s ./ns3 run soft-ue-integration-test

# 或者跳过耗时的测试
./scripts/run-soft-ue-tests.sh --no-stress
```

#### 3. 内存不足

**错误信息**:
```
terminate called after throwing an instance of 'std::bad_alloc'
```

**解决方案**:
```bash
# 降低测试规模
./ns3 run "soft-ue-stress-test --nodes=50 --duration=10"

# 或者增加系统内存
# 在虚拟机中分配更多内存
```

#### 4. 权限问题

**错误信息**:
```
Permission denied: ./scripts/run-soft-ue-tests.sh
```

**解决方案**:
```bash
# 添加执行权限
chmod +x scripts/*.sh
```

### 调试技巧

#### 1. 启用详细日志

```bash
# 启用调试日志
export NS_LOG="SoftUeIntegrationTest=debug:SoftUeNetDevice=debug:SesManager=debug:PdsManager=debug"

# 运行测试
./ns3 run soft-ue-integration-test
```

#### 2. 单步调试

```bash
# 使用gdb调试
gdb --args ./ns3 run soft-ue-integration-test

# gdb常用命令
(gdb) run
(gdb) bt
(gdb) p variable_name
(gdb) continue
```

#### 3. 内存检查

```bash
# 使用valgrind检查内存泄漏
valgrind --leak-check=full ./ns3 run soft-ue-integration-test
```

## 自动化集成

### 1. CI/CD集成

**GitHub Actions示例**:
```yaml
name: Soft-UE Tests

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

    - name: Build project
      run: |
        ./ns3 configure
        ./ns3 build

    - name: Run tests
      run: |
        ./scripts/run-soft-ue-tests.sh --quiet

    - name: Upload results
      uses: actions/upload-artifact@v2
      with:
        name: test-results
        path: test-results/
```

### 2. 本地自动化

**定时测试脚本**:
```bash
#!/bin/bash
# daily-test.sh

cd /home/makai/Soft-UE-ns3

# 更新代码
git pull origin main

# 构建项目
./ns3 build

# 运行测试
./scripts/run-soft-ue-tests.sh --quiet

# 发送结果通知
if [ $? -eq 0 ]; then
    echo "✅ Daily tests passed" | mail -s "Soft-UE Test Results" your-email@example.com
else
    echo "❌ Daily tests failed" | mail -s "Soft-UE Test Results" your-email@example.com
fi
```

**设置定时任务**:
```bash
# 编辑crontab
crontab -e

# 添加每日测试任务（凌晨2点）
0 2 * * * /home/makai/Soft-UE-ns3/scripts/daily-test.sh
```

## 贡献指南

### 添加新测试

1. **创建测试文件**:
```bash
# 在适当位置创建测试文件
touch src/soft-ue/test/new-component-test.cc
```

2. **实现测试逻辑**:
```cpp
#include "ns3/test.h"
#include "ns3/soft-ue-module.h"

class NewComponentTest : public TestCase
{
public:
    NewComponentTest () : TestCase ("New Component Test") {}

private:
    virtual void DoRun () override
    {
        // 实现测试逻辑
        NS_TEST_ASSERT_MSG_EQ (true, true, "Basic assertion");
    }
};

static NewComponentTest g_newComponentTest;
```

3. **更新CMakeLists.txt**:
```cmake
add_executable(new-component-test test/new-component-test.cc)
target_link_libraries(new-component-test ${libsoft-ue})
```

4. **更新测试脚本**:
```bash
# 在run-soft-ue-tests.sh中添加新测试
declare -a UNIT_TESTS=(
    "test/ses-manager-test"
    "test/pds-manager-test"
    "test/pds-full-test"
    "test/new-component-test"  # 新添加
)
```

### 代码规范

- **测试命名**: 使用描述性名称，如`SesManagerEndpointTest`
- **断言使用**: 优先使用NS_TEST_ASSERT_MSG_*宏
- **注释规范**: 为每个测试用例添加清晰注释
- **错误处理**: 测试应该验证错误情况的处理

## 总结

通过使用这套完整的测试脚本，您可以：

1. **快速验证**: 使用`quick-test.sh`进行基本功能验证
2. **全面测试**: 使用`run-soft-ue-tests.sh`进行完整测试套件
3. **专项验证**: 使用`soft-ue-validation-test.sh`进行特定功能验证
4. **持续监控**: 集成到CI/CD流程中实现自动化测试

这些测试脚本为Soft-UE协议栈提供了全面的质量保证，确保在各种条件下的功能正确性和性能稳定性。
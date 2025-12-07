# 贡献指南

感谢您对Soft-UE项目的关注！我们欢迎各种形式的贡献，包括但不限于代码、文档、问题反馈和功能建议。

## 🤝 贡献方式

### 报告问题
- 使用 [GitHub Issues](https://github.com/soft-ue-project/soft-ue-ns3/issues) 报告bug
- 提供详细的复现步骤和环境信息
- 包含相关的错误日志和截图

### 功能建议
- 在Issues中使用 "enhancement" 标签
- 详细描述功能需求和使用场景
- 讨论技术实现方案

### 代码贡献
- Fork项目仓库
- 创建功能分支 (`git checkout -b feature/amazing-feature`)
- 提交更改 (`git commit -m 'Add some AmazingFeature'`)
- 推送到分支 (`git push origin feature/amazing-feature`)
- 创建Pull Request

## 🏗️ 开发环境设置

### 系统要求
- Linux/macOS/Windows (WSL2)
- ns-3.44 或更高版本
- CMake 3.20+
- GCC 11+ 或 Clang 12+
- Python 3.8+

### 安装步骤

1. **获取源码**
```bash
git clone https://github.com/soft-ue-project/soft-ue-ns3.git
cd soft-ue-ns3
```

2. **安装依赖**
```bash
# Ubuntu/Debian
sudo apt-get update
sudo apt-get install cmake ninja-build libgtk-3-dev \
    libxml2-dev python3-dev python3-pip libgsl-dev

# macOS (使用Homebrew)
brew install cmake ninja gtk+3 xml2 python3 gsl
```

3. **配置项目**
```bash
./ns3 configure --enable-examples --enable-tests --build-profile=debug
```

4. **编译项目**
```bash
./ns3 build
```

5. **验证安装**
```bash
./ns3 run "test-runner --test-name=soft-ue-ses"
./ns3 run "test-runner --test-name=soft-ue-pds"
```

## 📝 代码规范

### 编码标准
我们严格遵循 [ns-3编码规范](https://www.nsnam.org/docs/contributing/contributing.html)：

#### 命名规范
```cpp
// 类名：PascalCase
class SoftUeNetDevice;

// 方法名：PascalCase
void SendPacket();

// 变量名：camelCase
uint32_t packetCount;

// 常量：kPascalCase
static const uint32_t kDefaultTimeout = 1000;
```

#### 文件组织
```cpp
// 头文件 (.h)
#ifndef SOFT_UE_NET_DEVICE_H
#define SOFT_UE_NET_DEVICE_H

#include "ns3/object.h"

namespace ns3 {

/**
 * \brief Soft-UE network device implementation
 */
class SoftUeNetDevice : public NetDevice
{
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  // ... public interface

private:
  // ... private members
};

} // namespace ns3

#endif /* SOFT_UE_NET_DEVICE_H */
```

#### 注释规范
```cpp
/**
 * \brief Send a packet through the Soft-UE protocol stack
 * \param packet the packet to send
 * \param source the source address
 * \param dest the destination address
 * \param protocolNumber the protocol number
 * \returns true if the packet was successfully queued for transmission
 */
bool SendPacket (Ptr<Packet> packet,
                 const Address& source,
                 const Address& dest,
                 uint16_t protocolNumber);
```

### 代码格式化
使用项目提供的格式化脚本：
```bash
./scripts/format-code.sh
```

或者使用clang-format手动格式化：
```bash
find src/soft-ue -name "*.cc" -o -name "*.h" | xargs clang-format -i
```

## 🧪 测试要求

### 单元测试
所有新功能必须包含相应的单元测试：

```cpp
// test/soft-ue-new-feature-test.cc
class SoftUeNewFeatureTestCase : public TestCase
{
public:
  SoftUeNewFeatureTestCase ();
  virtual ~SoftUeNewFeatureTestCase ();

private:
  virtual void DoRun (void);
};

SoftUeNewFeatureTestCase::SoftUeNewFeatureTestCase ()
  : TestCase ("SoftUe New Feature Test")
{
}

SoftUeNewFeatureTestCase::~SoftUeNewFeatureTestCase ()
{
}

void
SoftUeNewFeatureTestCase::DoRun (void)
{
  // Test implementation
  NS_TEST_ASSERT_MSG_EQ (result, expected, "Test description");
}
```

### 运行测试
```bash
# 运行所有Soft-UE测试
./ns3 test soft-ue

# 运行特定测试
./ns3 run "test-runner --test-name=soft-ue-new-feature"

# 运行完整测试套件
./scripts/run-tests.sh
```

### 测试覆盖率
维护 >80% 的测试覆盖率。检查覆盖率：
```bash
./scripts/check-coverage.sh
```

## 📚 文档要求

### API文档
所有公共接口必须有完整的Doxygen注释：

```cpp
/**
 * \ingroup softue
 * \brief Manages the Soft-UE packet delivery context
 *
 * This class provides reliable and unreliable packet delivery
 * services for the Soft-UE protocol stack.
 */
class PdcManager : public Object
{
public:
  /**
   * \enum PdcType
   * \brief Type of packet delivery context
   */
  enum PdcType {
    IPDC,  ///< Unreliable packet delivery
    TPDC   ///< Reliable packet delivery
  };

  // ... rest of the class
};
```

### 用户文档
更新相关文档：
- 用户指南 (`doc/user-guide/`)
- API参考 (`doc/api/`)
- 示例程序 (`examples/`)

## 🔍 Pull Request流程

### PR准备清单
- [ ] 代码通过所有测试
- [ ] 代码符合项目编码规范
- [ ] 添加必要的单元测试
- [ ] 更新相关文档
- [ ] 提交信息清晰明确

### 提交信息规范
```
类型(范围): 简短描述

详细描述（可选）

Closes #123
```

类型包括：
- `feat`: 新功能
- `fix`: Bug修复
- `docs`: 文档更新
- `style`: 代码格式化
- `refactor`: 代码重构
- `test`: 测试相关
- `chore`: 构建过程或辅助工具的变动

示例：
```
feat(pdc): Add fast retransmission support for TPDC

Implements fast retransmission mechanism to reduce
latency for reliable packet delivery. Includes
configuration options for threshold and timeout.

Closes #45
```

### PR审查流程
1. 自动化检查（CI/CD）
2. 代码审查（至少一位维护者）
3. 测试验证
4. 文档审查
5. 合并到主分支

## 🐛 调试指南

### 日志使用
```cpp
#include "ns3/log.h"

NS_LOG_COMPONENT_DEFINE ("SoftUeNetDevice");

void SoftUeNetDevice::SendPacket (Ptr<Packet> packet)
{
  NS_LOG_FUNCTION (this << packet);
  NS_LOG_DEBUG ("Sending packet of size " << packet->GetSize ());

  // ... implementation
}
```

启用日志：
```bash
# 启用Soft-UE模块调试日志
./ns3 run --log-level=debug soft-ue-example

# 启用特定组件日志
./ns3 run "--log-level=SoftUeNetDevice=level_function|debug" soft-ue-example
```

### 性能分析
使用ns-3内置性能分析工具：
```bash
# 启用性能统计
./ns3 run "--PrintStats" soft-ue-example

# 启用GProf profiling
./ns3 configure --build-profile=profile
./ns3 build
./ns3 run soft-ue-example
gprof ns3-run-debug gmon.out > analysis.txt
```

## 🚀 发布流程

### 版本号规范
使用 [语义化版本](https://semver.org/)：
- `MAJOR.MINOR.PATCH`
- `1.0.0` - 主要版本发布
- `1.1.0` - 新功能发布
- `1.1.1` - Bug修复发布

### 发布检查清单
- [ ] 所有测试通过
- [ ] 文档更新完成
- [ ] CHANGELOG更新
- [ ] 版本号更新
- [ ] 标签创建
- [ ] Release notes准备

## 🌟 社区

### 沟通渠道
- **GitHub Issues**: 问题报告和功能讨论
- **GitHub Discussions**: 一般讨论和问答
- **邮件列表**: soft-ue-dev@googlegroups.com
- **技术文档**: https://soft-ue.readthedocs.io

### 行为准则
我们致力于为每个人提供友好、安全和欢迎的环境，无论：
- 经验水平
- 性别认同和表达
- 性取向
- 残疾
- 个人外貌
- 身体大小
- 种族
- 民族
- 年龄
- 宗教
- 国籍

详细的行​​为准则请参考 [Code of Conduct](CODE_OF_CONDUCT.md)。

## 📞 获取帮助

如果您在贡献过程中遇到问题：

1. 查看 [FAQ](docs/FAQ.md)
2. 搜索现有的 [Issues](https://github.com/soft-ue-project/soft-ue-ns3/issues)
3. 创建新的 Issue 描述您的问题
4. 联系维护者：softuegroup@gmail.com

## 🏆 贡献者认可

我们感谢所有贡献者的努力！贡献者将被列在：
- README.md中的贡献者列表
- RELEASE NOTES中的贡献致谢
- 项目网站的贡献者页面

---

再次感谢您的贡献！您的工作对Soft-UE项目的发展非常重要。
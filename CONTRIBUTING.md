# Contributing to Soft-UE

🎉 **Thank you for your interest in contributing to Soft-UE!** 🎉

We welcome contributions from the network simulation, research, and high-performance computing communities. This document provides comprehensive guidelines for contributing to the **world's first Ultra Ethernet protocol stack implementation** - a groundbreaking project achieving **6.25x performance improvement** over traditional networking solutions.

## 📋 Table of Contents

- [Code of Conduct](#code-of-conduct)
- [Getting Started](#getting-started)
- [Development Workflow](#development-workflow)
- [Coding Standards](#coding-standards)
- [Testing Guidelines](#testing-guidelines)
- [Documentation](#documentation)
- [Performance Benchmarks](#performance-benchmarks)
- [Review Process](#review-process)
- [Community](#community)

## 🚀 Quick Start

### For Code Contributors

```bash
# 1. Fork and clone the repository
git clone https://github.com/your-username/soft-ue-ns3.git
cd soft-ue-ns3

# 2. Create a feature branch
git checkout -b feature/amazing-feature

# 3. Make your changes and test
./ns3 build soft-ue
./ns3 test soft-ue

# 4. Submit a pull request
git push origin feature/amazing-feature
```

### For Bug Reports

- Use [GitHub Issues](https://github.com/your-org/soft-ue-ns3/issues) with the "bug" label
- Provide detailed reproduction steps and environment information
- Include relevant error logs and screenshots

### For Feature Requests

- Use [GitHub Issues](https://github.com/your-org/soft-ue-ns3/issues) with the "enhancement" label
- Describe the feature requirements and use cases
- Discuss technical implementation approaches

## 🛠️ Development Environment Setup

### System Requirements
- Linux/macOS/Windows (WSL2)
- ns-3.44 or higher
- CMake 3.20+
- GCC 11+ or Clang 12+
- Python 3.8+

### Installation Steps

1. **Clone the repository**
```bash
git clone https://github.com/your-org/soft-ue-ns3.git
cd soft-ue-ns3
```

2. **Install dependencies**
```bash
# Ubuntu/Debian
sudo apt-get update
sudo apt-get install cmake ninja-build libgtk-3-dev \
    libxml2-dev python3-dev python3-pip libgsl-dev

# macOS (using Homebrew)
brew install cmake ninja gtk+3 xml2 python3 gsl
```

3. **Configure the project**
```bash
./ns3 configure --enable-examples --enable-tests --build-profile=debug
```

4. **Build the project**
```bash
./ns3 build
```

5. **Verify installation**
```bash
./ns3 run "test-runner --test-name=soft-ue-ses"
./ns3 run "test-runner --test-name=soft-ue-pds"
```

## 📝 Code Standards

We strictly follow [ns-3 coding conventions](https://www.nsnam.org/docs/contributing/contributing.html):

### Naming Conventions
```cpp
// Class names: PascalCase
class SoftUeNetDevice;

// Method names: PascalCase
void SendPacket();

// Variable names: camelCase
uint32_t packetCount;

// Constants: kPascalCase
static const uint32_t kDefaultTimeout = 1000;
```

### File Organization
```cpp
// Header files (.h)
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

### Documentation Standards
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

### Code Formatting
Use the project's formatting script:
```bash
./scripts/format-code.sh
```

Or use clang-format manually:
```bash
find src/soft-ue -name "*.cc" -o -name "*.h" | xargs clang-format -i
```

## 🧪 Testing Requirements

### Unit Tests
All new features must include corresponding unit tests:

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

void
SoftUeNewFeatureTestCase::DoRun (void)
{
  // Test implementation
  NS_TEST_ASSERT_MSG_EQ (result, expected, "Test description");
}
```

### Running Tests
```bash
# Run all Soft-UE tests
./ns3 test soft-ue

# Run specific tests
./ns3 run "test-runner --test-name=soft-ue-new-feature"

# Run full test suite
./scripts/run-tests.sh
```

### Test Coverage
Maintain >80% test coverage. Check coverage:
```bash
./scripts/check-coverage.sh
```

## 📚 Documentation Requirements

### API Documentation
All public interfaces must have complete Doxygen comments:

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

### User Documentation
Update relevant documentation:
- User guide (`doc/user-guide/`)
- API reference (`doc/api/`)
- Example programs (`examples/`)

## 🔍 Pull Request Process

### PR Preparation Checklist
- [ ] Code passes all tests
- [ ] Code follows project coding standards
- [ ] Necessary unit tests added
- [ ] Documentation updated
- [ ] Commit messages are clear and descriptive

### Commit Message Format
```
type(scope): brief description

Detailed description (optional)

Closes #123
```

Types include:
- `feat`: New feature
- `fix`: Bug fix
- `docs`: Documentation update
- `style`: Code formatting
- `refactor`: Code refactoring
- `test`: Test-related
- `chore`: Build process or auxiliary tool changes

Example:
```
feat(pdc): Add fast retransmission support for TPDC

Implements fast retransmission mechanism to reduce
latency for reliable packet delivery. Includes
configuration options for threshold and timeout.

Closes #45
```

### PR Review Process
1. Automated checks (CI/CD)
2. Code review (at least one maintainer)
3. Test validation
4. Documentation review
5. Merge to main branch

## 🐛 Debugging Guide

### Logging Usage
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

Enable logging:
```bash
# Enable Soft-UE module debug logging
./ns3 run --log-level=debug soft-ue-example

# Enable specific component logging
./ns3 run "--log-level=SoftUeNetDevice=level_function|debug" soft-ue-example
```

### Performance Analysis
Use ns-3 built-in performance analysis tools:
```bash
# Enable performance statistics
./ns3 run "--PrintStats" soft-ue-example

# Enable GProf profiling
./ns3 configure --build-profile=profile
./ns3 build
./ns3 run soft-ue-example
gprof ns3-run-debug gmon.out > analysis.txt
```

## 🌟 Community

### Communication Channels
- **GitHub Issues**: Bug reports and feature discussions
- **GitHub Discussions**: General discussions and Q&A
- **Email List**: soft-ue-dev@googlegroups.com
- **Documentation**: https://soft-ue.readthedocs.io

### Code of Conduct
We are committed to providing a friendly, safe, and welcoming environment for everyone, regardless of:
- Experience level
- Gender identity and expression
- Sexual orientation
- Disability
- Personal appearance
- Body size
- Race
- Ethnicity
- Age
- Religion
- Nationality

For detailed guidelines, please refer to [Code of Conduct](CODE_OF_CONDUCT.md).

## 📞 Getting Help

If you encounter problems while contributing:

1. Check the [FAQ](docs/FAQ.md)
2. Search existing [Issues](https://github.com/your-org/soft-ue-ns3/issues)
3. Create a new Issue describing your problem
4. Contact maintainers: soft-ue-maintainers@example.org

## 🏆 Contributor Recognition

We appreciate all contributors' efforts! Contributors will be listed in:
- Contributors list in README.md
- Contributor acknowledgments in RELEASE NOTES
- Contributors page on the project website

---

Thank you again for your contribution! Your work is very important to the development of the Soft-UE project.
---
name: Bug Report
about: Create a report to help us improve
title: '[BUG] '
labels: 'bug'
assignees: ''

---

## 🐛 Bug Description
A clear and concise description of what the bug is.

## 🔄 Reproduction Steps
Please provide detailed steps to reproduce the issue:

1. Build command used:
   ```bash
   ./ns3 build soft-ue
   ```

2. Example command that triggered the bug:
   ```bash
   ./ns3 run [example-name] [parameters]
   ```

3. Expected behavior:
   [What you expected to happen]

4. Actual behavior:
   [What actually happened]

## 🌍 Environment Information
- **Operating System**: [e.g., Ubuntu 22.04, macOS 13.0, Windows 11 WSL2]
- **ns-3 Version**: [e.g., ns-3.44, ns-3.45]
- **Soft-UE Version**: [e.g., v1.0.0, latest from main]
- **Compiler**: [e.g., GCC 11.4, Clang 14.0]
- **CMake Version**: [e.g., 3.24.0]
- **Build Configuration**: [e.g., debug, release, profile]

## 📋 Configuration
Please provide your Soft-UE configuration if relevant:

```cpp
// Example configuration code
SoftUeHelper helper;
helper.SetDeviceAttribute("MaxPdcCount", UintegerValue(512));
helper.SetChannelAttribute("DataRate", DataRateValue("1Gbps"));
```

## 📊 Error Messages
Please copy and paste any error messages, stack traces, or logs:

```
[Paste error messages here]
```

## 📈 Performance Impact
If this is a performance-related bug:
- **Expected Performance**: [e.g., 6.25 Gbps throughput]
- **Actual Performance**: [e.g., 2.1 Gbps throughput]
- **Regression**: [Is this a regression from previous versions?]

## 🔍 Additional Context
Add any other context about the problem here:
- How often does this occur? [Always, sometimes, rarely]
- Any workarounds? [Describe if you found any]
- Related issues? [Link to any related issues]

## 📁 Attachments
If applicable, add attachments to help explain your problem:
- Build logs
- Configuration files
- Trace files (`.tr`, `.pcap`)
- Screenshots

## ✅ Checklist
Before submitting, please check:

- [ ] I have searched existing issues for similar problems
- [ ] I have provided all necessary environment information
- [ ] I have included complete reproduction steps
- [ ] I have included relevant error messages and logs
- [ ] I have checked that this is not a duplicate issue

## 🏷️ Suggested Labels
- `bug`
- `critical` (if it prevents basic usage)
- `performance` (if it's a performance regression)
- `regression` (if it worked in previous versions)
- `good first issue` (if suitable for new contributors)
- `help wanted` (if you need help investigating)

---

**Thank you for helping improve Soft-UE!** 🚀
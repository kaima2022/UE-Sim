## 🚀 Pull Request Description
<!-- Provide a clear and concise description of your changes -->

### Type of Change
- [ ] 🐛 **Bug fix** (non-breaking change that fixes an issue)
- [ ] ✨ **New feature** (non-breaking change that adds functionality)
- [ ] 💥 **Breaking change** (fix or feature that would cause existing functionality to not work as expected)
- [ ] 📝 **Documentation update** (documentation only changes)
- [ ] 🎨 **Code style** (formatting, missing semicolons, etc; no code change)
- [ ] ♻️ **Refactor** (non-breaking change that improves code maintainability)
- [ ] ⚡ **Performance improvement** (non-breaking change that improves performance)
- [ ] ✅ **Test addition** (adding missing tests or improving existing tests)

### Summary
<!-- Brief description of what this PR does -->

### Motivation and Context
<!-- Why is this change necessary? What problem does it solve? -->
<!-- If it fixes an open issue, please link to it here: Closes #123 -->

### Changes Made
<!-- List the main changes in this PR -->

#### Files Modified:
- [ ] `src/soft-ue/model/` - Core implementation changes
- [ ] `src/soft-ue/helper/` - Helper class changes
- [ ] `src/soft-ue/test/` - Test changes
- [ ] `examples/` - Example updates
- [ ] `doc/` - Documentation updates
- [ ] `README.md` - Main documentation
- [ ] `CONTRIBUTING.md` - Contributing guidelines
- [ ] Other: _____________________

#### Key Changes:
1.
2.
3.

## 🧪 Testing
### Testing Strategy
<!-- Describe how you tested your changes -->

#### Manual Testing
- [ ] **First Example**: `./ns3 run first-soft-ue` passes
- [ ] **Performance Benchmark**: `./ns3 run performance-benchmark` shows expected results
- [ ] **AI Network Simulation**: `./ns3 run ai-network-simulation` completes successfully
- [ ] **Custom Test**: Describe your custom test scenario

#### Automated Testing
- [ ] All existing tests pass: `./ns3 test soft-ue`
- [ ] New test cases added for new functionality
- [ ] Test coverage maintained/improved

#### Performance Testing
- [ ] No performance regression
- [ ] Performance improvement verified (if applicable)
- [ ] Benchmarks run successfully

### Test Results
<!-- Paste your test results here -->
```
[Paste test output here]
```

## 📊 Performance Impact
### Before
- **Throughput**: [e.g., 5.2 Gbps]
- **Latency**: [e.g., 15.3 μs]
- **Memory Usage**: [e.g., 1.2 MB]
- **Test Success Rate**: [e.g., 84%]

### After
- **Throughput**: [e.g., 6.1 Gbps] (+17% improvement)
- **Latency**: [e.g., 12.8 μs] (-16% improvement)
- **Memory Usage**: [e.g., 1.1 MB] (-8% improvement)
- **Test Success Rate**: [e.g., 88%] (+4% improvement)

## 📋 Checklist
### Code Quality
- [ ] My code follows the project's [coding standards](CONTRIBUTING.md#code-standards)
- [ ] I have performed a self-review of my own code
- [ ] I have commented my code, particularly in hard-to-understand areas
- [ ] I have made corresponding changes to the documentation
- [ ] My changes generate no new warnings

### Testing
- [ ] I have added tests that prove my fix is effective or that my feature works
- [ ] New and existing unit tests pass locally with my changes
- [ ] Any dependent changes have been merged and published in downstream modules

### Documentation
- [ ] I have updated the documentation accordingly
- [ ] I have added examples for new functionality
- [ ] API changes are documented with Doxygen comments

## 🔍 Review Process
### Reviewer Guidelines
Please check the following during review:
- [ ] Code quality and adherence to standards
- [ ] Performance impact assessment
- [ ] Test coverage and quality
- [ ] Documentation completeness
- [ ] Backward compatibility
- [ ] Security implications

### Areas of Focus
<!-- Highlight specific areas you'd like reviewers to focus on -->
-
-
-

## 📚 Documentation Updates
### API Documentation
- [ ] Doxygen comments updated for modified APIs
- [ ] New APIs properly documented
- [ ] API reference updated

### User Documentation
- [ ] README.md updated (if needed)
- [ ] Examples updated (if needed)
- [ ] User guide updated (if needed)

### Developer Documentation
- [ ] Contributing guidelines updated (if needed)
- [ ] Architecture documentation updated (if needed)

## 🏷️ Labels for Maintainers
<!-- Suggest labels for maintainers to apply -->
-
-
-

## 🔗 Related Issues
<!-- Link to related issues -->
- Closes #
- Related to #
- Depends on #

## 📸 Screenshots (if applicable)
<!-- Add screenshots to help explain your changes -->
<!-- Drag and drop screenshots here -->

## 📝 Additional Notes
<!-- Any additional context or notes for reviewers -->

---

## 🚨 Breaking Changes
If this PR contains breaking changes, please complete this section:

### Migration Guide
<!-- Provide a step-by-step guide for users to migrate -->
1.
2.
3.

### Deprecated Features
- [ ] List any deprecated features
- [ ] Provide removal timeline

### Backward Compatibility
- [ ] Explain any backward compatibility issues
- [ ] Provide workarounds if available

## 🎯 Performance Metrics
### Benchmarks
<!-- Include benchmark results if applicable -->

#### Throughput Benchmarks
| Test Case | Before | After | Improvement |
|-----------|--------|-------|-------------|
| [Test 1] | [X Gbps] | [Y Gbps] | [Z%] |
| [Test 2] | [X Gbps] | [Y Gbps] | [Z%] |

#### Latency Benchmarks
| Test Case | Before | After | Improvement |
|-----------|--------|-------|-------------|
| [Test 1] | [X μs] | [Y μs] | [Z%] |
| [Test 2] | [X μs] | [Y μs] | [Z%] |

#### Memory Usage
| Test Case | Before | After | Improvement |
|-----------|--------|-------|-------------|
| [Test 1] | [X MB] | [Y MB] | [Z%] |
| [Test 2] | [X MB] | [Y MB] | [Z%] |

## ✅ Pre-flight Checklist
Before submitting this PR, please ensure:

- [ ] **Code compiles without warnings**: `./ns3 build soft-ue`
- [ ] **All tests pass**: `./ns3 test soft-ue`
- [ ] **No memory leaks**: (Run with memory check if applicable)
- [ ] **Documentation is updated**: (API docs, examples, README)
- [ ] **Performance regression tested**: (If performance-related)
- [ ] **Backward compatibility checked**: (If breaking changes)
- [ ] **Security considerations reviewed**: (If security-related)
- [ ] **License headers updated**: (For new files)

---

**Thank you for contributing to Soft-UE!** 🚀

*This PR template is designed to ensure high-quality contributions and thorough review.*
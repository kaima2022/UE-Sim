#!/bin/bash

echo "🔍 Soft-UE GitHub发布就绪验证"
echo "============================="
echo ""

# 验证项目状态
echo "📊 项目技术状态验证:"

# 检查源代码文件
source_files_count=$(find src/soft-ue -name "*.cc" -o -name "*.h" | wc -l)
echo "   ✅ 核心源文件: $source_files_count 个"

# 检查代码行数
if [ -d "src/soft-ue" ]; then
    code_lines=$(find src/soft-ue -name "*.cc" -o -name "*.h" | xargs wc -l | tail -1 | awk '{print $1}')
    echo "   ✅ 代码行数: $code_lines 行"
fi

# 检查性能基准测试
if [ -f "benchmarks/soft-ue-performance-benchmark.cc" ]; then
    echo "   ✅ 性能基准测试: 已实现"
fi

# 检查文档完整性
echo ""
echo "📋 发布文档验证:"
required_docs=(
    "README.md"
    "CONTRIBUTING.md"
    "GITHUB_RELEASE_ANNOUNCEMENT.md"
    "NS3-CONTRIB-PULL-REQUEST.md"
    "SIGCOMM_PAPER_COMPLETE_OUTLINE.md"
    "ULTRA_ETHERNET_ALLIANCE_CONTACT_PLAN.md"
)

for doc in "${required_docs[@]}"; do
    if [ -f "$doc" ]; then
        echo "   ✅ $doc"
    else
        echo "   ❌ $doc (缺失)"
    fi
done

# 检查版本控制状态
echo ""
echo "🏷️ 版本控制验证:"
if git tag | grep -q "v1.0.0"; then
    echo "   ✅ v1.0.0 标签已创建"
else
    echo "   ❌ v1.0.0 标签未创建"
fi

if [ -d ".git" ]; then
    echo "   ✅ Git仓库初始化"
else
    echo "   ❌ Git仓库未初始化"
fi

# 检查开源许可
if [ -f "LICENSE" ]; then
    echo "   ✅ 开源许可证文件"
else
    echo "   ❌ 开源许可证文件缺失"
fi

# 检查构建状态
echo ""
echo "🔨 构建状态验证:"
if [ -f "build/lib/libns3.44-soft-ue.so" ]; then
    lib_size=$(du -h build/lib/libns3.44-soft-ue.so | cut -f1)
    echo "   ✅ Soft-UE库已编译: $lib_size"
else
    echo "   ⚠️  Soft-UE库需要编译"
fi

# 发布就绪评分
echo ""
echo "🎯 发布就绪评分:"
total_checks=12
passed_checks=0

# 自动检查主要项目
[ -f "README.md" ] && ((passed_checks++))
[ -f "CONTRIBUTING.md" ] && ((passed_checks++))
[ -f "GITHUB_RELEASE_ANNOUNCEMENT.md" ] && ((passed_checks++))
git tag | grep -q "v1.0.0" && ((passed_checks++))
[ -d "src/soft-ue" ] && ((passed_checks++))
[ -f "benchmarks/soft-ue-performance-benchmark.cc" ] && ((passed_checks++))
[ -f "NS3-CONTRIB-PULL-REQUEST.md" ] && ((passed_checks++))
[ -f "SIGCOMM_PAPER_COMPLETE_OUTLINE.md" ] && ((passed_checks++))
[ -f "ULTRA_ETHERNET_ALLIANCE_CONTACT_PLAN.md" ] && ((passed_checks++))
[ -f "LICENSE" ] && ((passed_checks++))
[ -d ".git" ] && ((passed_checks++))
[ -f "create-github-repo.sh" ] && ((passed_checks++))

score=$((passed_checks * 100 / total_checks))
echo "   完成度: $passed_checks/$total_checks ($score%)"

if [ $score -eq 100 ]; then
    echo "   🎉 完全就绪 - 立即执行GitHub发布！"
    echo ""
    echo "🚀 立即执行命令:"
    echo "   ./create-github-repo.sh"
    echo ""
    echo "📖 或参考指南:"
    echo "   cat GITHUB_IMMEDIATE_RELEASE_GUIDE.md"
elif [ $score -ge 80 ]; then
    echo "   ⚡ 基本就绪 - 可以发布，建议先完成缺失项目"
else
    echo "   ⚠️  需要准备 - 完成更多验证项目后再发布"
fi

echo ""
echo "验证完成 - $(date)"
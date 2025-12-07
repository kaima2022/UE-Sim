#!/bin/bash

# Soft-UE GitHub公开仓库创建脚本
# 执行Phase 2技术统治力建立的关键步骤

echo "🚀 Soft-UE GitHub公开仓库创建脚本"
echo "=================================="

# 检查当前Git状态
echo "📋 检查当前Git状态..."
if [ ! -d ".git" ]; then
    echo "❌ 错误: 当前目录不是Git仓库"
    exit 1
fi

# 检查是否有v1.0.0标签
if ! git tag | grep -q "v1.0.0"; then
    echo "❌ 错误: v1.0.0标签不存在"
    exit 1
fi

echo "✅ Git状态检查通过"

# 检查必要文件
echo "📋 检查必要文件..."
required_files=(
    "README.md"
    "CONTRIBUTING.md"
    "CLAUDE.md"
    "GITHUB_RELEASE_ANNOUNCEMENT.md"
    "src/soft-ue/"
)

for file in "${required_files[@]}"; do
    if [ ! -e "$file" ]; then
        echo "❌ 错误: 缺少必要文件 $file"
        exit 1
    fi
done

echo "✅ 必要文件检查通过"

# GitHub仓库创建指令
echo ""
echo "🌐 GitHub仓库创建步骤:"
echo "======================="
echo "1. 访问: https://github.com/new"
echo "2. 仓库名称: soft-ue-ns3"
echo "3. 描述: Ultra Ethernet Protocol Stack for ns-3 - 6.25x faster than TCP/IP"
echo "4. 类型: Public (公开)"
echo "5. 许可证: Apache License 2.0"
echo "6. ❌ 不要添加README文件 (我们已有README.md)"
echo "7. ❌ 不要添加.gitignore"
echo "8. 点击 'Create repository'"
echo ""

read -p "按Enter键继续，或按Ctrl+C取消..."

# 获取GitHub用户名
echo ""
echo "🔧 配置远程仓库..."
read -p "请输入您的GitHub用户名: " github_username

if [ -z "$github_username" ]; then
    echo "❌ 错误: GitHub用户名不能为空"
    exit 1
fi

# 添加远程仓库
remote_url="https://github.com/${github_username}/soft-ue-ns3.git"
echo "添加远程仓库: $remote_url"

git remote add origin "$remote_url"
git branch -M main

echo "✅ 远程仓库配置完成"

# 推送到GitHub
echo ""
echo "📤 推送代码到GitHub..."
echo "这可能会需要一些时间..."

git push -u origin main
if [ $? -ne 0 ]; then
    echo "❌ 推送失败，请检查GitHub认证"
    echo "提示: 可能需要配置GitHub Personal Access Token"
    exit 1
fi

echo "✅ 代码推送成功"

# 推送标签
echo ""
echo "🏷️ 推送版本标签..."
git push origin --tags
if [ $? -ne 0 ]; then
    echo "❌ 标签推送失败"
    exit 1
fi

echo "✅ 版本标签推送成功"

# GitHub Release创建指令
echo ""
echo "🎉 GitHub Release创建步骤:"
echo "==========================="
echo "1. 访问: https://github.com/${github_username}/soft-ue-ns3/releases/new"
echo "2. 选择标签: v1.0.0"
echo "3. 标题: Soft-UE v1.0.0: Ultra Ethernet Protocol Stack"
echo "4. 描述: 复制 GITHUB_RELEASE_ANNOUNCEMENT.md 的内容"
echo "5. 设置为: Latest release"
echo "6. 点击 'Publish release'"
echo ""

# 自动打开GitHub Release页面
if command -v xdg-open > /dev/null; then
    xdg-open "https://github.com/${github_username}/soft-ue-ns3/releases/new"
elif command -v open > /dev/null; then
    open "https://github.com/${github_username}/soft-ue-ns3/releases/new"
else
    echo "请手动访问: https://github.com/${github_username}/soft-ue-ns3/releases/new"
fi

echo ""
echo "🔗 仓库链接: https://github.com/${github_username}/soft-ue-ns3"
echo ""
echo "🎯 下一步行动:"
echo "1. 创建GitHub Release"
echo "2. 发布到ns-3邮件列表"
echo "3. 提交ns-3-contrib Pull Request"
echo "4. 启动学术研究组联系"
echo ""
echo "✅ GitHub公开仓库创建脚本执行完成！"
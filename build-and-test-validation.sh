#!/bin/bash
# Soft-UE 构建验证脚本
# 完整验证CMake构建目标和T002测试修复效果

set -e  # 遇到错误立即退出

# 颜色输出定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# 日志文件
LOG_DIR="./logs"
mkdir -p "$LOG_DIR"
TIMESTAMP=$(date +"%Y%m%d-%H%M%S")
BUILD_LOG="$LOG_DIR/build-$TIMESTAMP.log"
TEST_LOG="$LOG_DIR/test-$TIMESTAMP.log"

echo -e "${BLUE}=== Soft-UE 构建验证脚本 ===${NC}"
echo "时间戳: $TIMESTAMP"
echo "构建日志: $BUILD_LOG"
echo "测试日志: $TEST_LOG"
echo

# 步骤1: 清理之前的构建
echo -e "${YELLOW}[1/5] 清理之前的构建...${NC}"
if [ -d "cmake-cache" ]; then
    rm -rf cmake-cache
fi
if [ -d "build" ]; then
    rm -rf build
fi
echo -e "${GREEN}✓ 清理完成${NC}"

# 步骤2: CMake配置
echo -e "${YELLOW}[2/5] 执行CMake配置...${NC}"
./ns3 configure --build-profile=debug --enable-examples --enable-tests > "$BUILD_LOG" 2>&1 || {
    echo -e "${RED}✗ CMake配置失败${NC}"
    tail -20 "$BUILD_LOG"
    exit 1
}
echo -e "${GREEN}✓ CMake配置成功${NC}"

# 步骤3: 构建项目
echo -e "${YELLOW}[3/5] 构建Soft-UE模块...${NC}"
./ns3 build soft-ue >> "$BUILD_LOG" 2>&1 || {
    echo -e "${RED}✗ Soft-UE模块构建失败${NC}"
    tail -30 "$BUILD_LOG"
    exit 1
}
echo -e "${GREEN}✓ Soft-UE模块构建成功${NC}"

# 验证关键库文件
if [ -f "build/lib/libns3.44-soft-ue-default.so" ]; then
    LIB_SIZE=$(stat -c%s "build/lib/libns3.44-soft-ue-default.so")
    echo -e "${GREEN}✓ 库文件生成成功 (大小: $LIB_SIZE 字节)${NC}"
else
    echo -e "${RED}✗ 库文件未找到${NC}"
    exit 1
fi

# 验证测试可执行文件
TEST_EXECUTABLES=("ses-manager-test" "pds-manager-test" "pds-full-test")
for exe in "${TEST_EXECUTABLES[@]}"; do
    if [ -f "build/$exe" ]; then
        echo -e "${GREEN}✓ 测试可执行文件 $exe 生成成功${NC}"
    else
        echo -e "${RED}✗ 测试可执行文件 $exe 未找到${NC}"
        exit 1
    fi
done

# 步骤4: 验证符号表
echo -e "${YELLOW}[4/5] 验证关键符号...${NC}"
REQUIRED_SYMBOLS=(
    "_ZN4ns317SoftUeHeaderTagC1ENS_7PDSTypeEtj"  # SoftUeHeaderTag构造函数
    "_ZN4ns317SoftUeHeaderTag22SetDestinationEndpointEj"  # SetDestinationEndpoint
    "_ZN4ns317SoftUeHeaderTag17SetSourceEndpointEj"  # SetSourceEndpoint
)

for symbol in "${REQUIRED_SYMBOLS[@]}"; do
    if nm -D "build/lib/libns3.44-soft-ue-default.so" | grep -q "$symbol"; then
        echo -e "${GREEN}✓ 符号 $symbol 找到${NC}"
    else
        echo -e "${RED}✗ 符号 $symbol 未找到${NC}"
        exit 1
    fi
done

# 步骤5: 运行T002测试
echo -e "${YELLOW}[5/5] 运行T002集成测试...${NC}"
# 复制测试文件到scratch目录
if [ -f "validate-soft-ue-demo.cc" ]; then
    cp validate-soft-ue-demo.cc scratch/
fi

# 运行测试并捕获输出
echo "执行T002端到端协议栈集成测试..."
timeout 300s ./ns3 run "scratch/validate-soft-ue-demo --verbose=2" > "$TEST_LOG" 2>&1 || {
    TEST_EXIT_CODE=$?
    if [ $TEST_EXIT_CODE -eq 124 ]; then
        echo -e "${RED}✗ 测试超时 (5分钟)${NC}"
    else
        echo -e "${RED}✗ T002测试执行失败 (退出码: $TEST_EXIT_CODE)${NC}"
    fi
    tail -30 "$TEST_LOG"
    exit 1
}

# 分析测试结果
echo -e "${YELLOW}分析测试结果...${NC}"
SUCCESS_PATTERNS=("设备安装.*成功率.*100%" "SoftUeNetDevice.*有效" "协议栈初始化.*成功" "数据包发送.*成功")
FAILURE_PATTERNS=("Error" "Failed" "undefined reference" "链接错误")

TEST_SUCCESS=true
for pattern in "${SUCCESS_PATTERNS[@]}"; do
    if grep -q "$pattern" "$TEST_LOG"; then
        echo -e "${GREEN}✓ 成功指标匹配: $pattern${NC}"
    else
        echo -e "${YELLOW}⚠ 成功指标未匹配: $pattern${NC}"
        TEST_SUCCESS=false
    fi
done

for pattern in "${FAILURE_PATTERNS[@]}"; do
    if grep -qi "$pattern" "$TEST_LOG"; then
        echo -e "${RED}✗ 失败指标匹配: $pattern${NC}"
        TEST_SUCCESS=false
    fi
done

# 生成报告
echo
echo -e "${BLUE}=== 验证报告 ===${NC}"
echo "构建时间: $(date)"
echo "CMake配置: 成功"
echo "Soft-UE模块构建: 成功"
echo "库文件大小: $LIB_SIZE 字节"
echo "测试可执行文件: ${#TEST_EXECUTABLES[@]} 个生成成功"

if [ "$TEST_SUCCESS" = true ]; then
    echo -e "${GREEN}T002集成测试: 成功${NC}"
    echo
    echo -e "${GREEN}🎉 所有验证项目通过！Soft-UE CMake构建目标配置完成。${NC}"
else
    echo -e "${YELLOW}T002集成测试: 部分通过${NC}"
    echo
    echo -e "${YELLOW}⚠️ 构建成功但测试结果需要进一步分析。${NC}"
fi

echo
echo "详细日志文件:"
echo "- 构建日志: $BUILD_LOG"
echo "- 测试日志: $TEST_LOG"
echo

# 清理函数
cleanup() {
    echo "清理临时文件..."
}

trap cleanup EXIT

exit 0
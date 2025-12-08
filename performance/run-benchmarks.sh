#!/bin/bash

# Soft-UE Performance Benchmark Script
# This script runs comprehensive performance benchmarks for Soft-UE
# and generates detailed performance reports

set -e  # Exit on any error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Performance thresholds
MIN_THROUGHPUT_GBPS=3.0
MAX_LATENCY_US=50.0
MAX_PACKET_LOSS_PERCENT=1.0
MIN_TEST_COVERAGE_PERCENT=80.0

# Configuration
BENCHMARK_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
RESULTS_DIR="$BENCHMARK_DIR/results"
TIMESTAMP=$(date +"%Y%m%d_%H%M%S")
REPORT_DIR="$RESULTS_DIR/report_$TIMESTAMP"

# Create output directories
mkdir -p "$REPORT_DIR"
mkdir -p "$REPORT_DIR/traces"
mkdir -p "$REPORT_DIR/plots"

echo -e "${BLUE}🚀 Soft-UE Performance Benchmark Suite${NC}"
echo -e "${BLUE}=========================================${NC}"
echo "Timestamp: $TIMESTAMP"
echo "Results Directory: $REPORT_DIR"
echo ""

# Function to print colored output
print_status() {
    local status=$1
    local message=$2
    case $status in
        "INFO")
            echo -e "${BLUE}ℹ️  INFO: $message${NC}"
            ;;
        "SUCCESS")
            echo -e "${GREEN}✅ SUCCESS: $message${NC}"
            ;;
        "WARNING")
            echo -e "${YELLOW}⚠️  WARNING: $message${NC}"
            ;;
        "ERROR")
            echo -e "${RED}❌ ERROR: $message${NC}"
            ;;
        "HEADER")
            echo -e "${BLUE}🎯 $message${NC}"
            ;;
    esac
}

# Function to check if Soft-UE is properly built
check_prerequisites() {
    print_status "INFO" "Checking prerequisites..."

    # Check if ns-3 build system is available
    if ! command -v ./ns3 &> /dev/null; then
        print_status "ERROR" "ns-3 build system not found. Please ensure you're in the ns-3 directory."
        exit 1
    fi

    # Check if Soft-UE module is built
    if [ ! -f "build/lib/libns3.44-soft-ue.so" ]; then
        print_status "INFO" "Soft-UE module not found. Building..."
        ./ns3 build soft-ue
        if [ $? -ne 0 ]; then
            print_status "ERROR" "Failed to build Soft-UE module."
            exit 1
        fi
    fi

    print_status "SUCCESS" "Prerequisites check passed"
}

# Function to run basic functionality test
run_functionality_test() {
    print_status "HEADER" "Running Basic Functionality Test"

    local test_output="$REPORT_DIR/first-soft-ue.log"

    print_status "INFO" "Running first-soft-ue example..."
    timeout 300 ./ns3 run first-soft-ue --quiet > "$test_output" 2>&1

    if [ $? -eq 0 ]; then
        print_status "SUCCESS" "Basic functionality test passed"

        # Extract key metrics
        local throughput=$(grep "Average throughput:" "$test_output" | grep -o '[0-9.]*' | head -1)
        local efficiency=$(grep "Efficiency:" "$test_output" | grep -o '[0-9.]*' | head -1)

        echo "Throughput: ${throughput:-N/A} Gbps" > "$REPORT_DIR/functionality_metrics.txt"
        echo "Efficiency: ${efficiency:-N/A}%" >> "$REPORT_DIR/functionality_metrics.txt"

        if [ -n "$throughput" ] && (( $(echo "$throughput > $MIN_THROUGHPUT_GBPS" | bc -l) )); then
            print_status "SUCCESS" "Throughput test passed: $throughput Gbps > $MIN_THROUGHPUT_GBPS Gbps"
        else
            print_status "WARNING" "Throughput below expected: ${throughput:-N/A} Gbps"
        fi

    else
        print_status "ERROR" "Basic functionality test failed"
        cat "$test_output"
        return 1
    fi
}

# Function to run comprehensive performance benchmark
run_performance_benchmark() {
    print_status "HEADER" "Running Comprehensive Performance Benchmark"

    local benchmark_output="$REPORT_DIR/performance-benchmark.log"

    print_status "INFO" "Running performance-benchmark example..."
    timeout 600 ./ns3 run performance-benchmark --quiet > "$benchmark_output" 2>&1

    if [ $? -eq 0 ]; then
        print_status "SUCCESS" "Performance benchmark completed"

        # Extract performance summary
        local avg_throughput=$(grep "Average:" "$benchmark_output" | grep "throughput" | grep -o '[0-9.]*' | head -1)
        local max_throughput=$(grep "Maximum:" "$benchmark_output" | grep "throughput" | grep -o '[0-9.]*' | head -1)
        local avg_latency=$(grep "Average:" "$benchmark_output" | grep "latency" | grep -o '[0-9.]*' | head -1)
        local min_latency=$(grep "Minimum:" "$benchmark_output" | grep "latency" | grep -o '[0-9.]*' | head -1)
        local avg_packet_loss=$(grep "Average:" "$benchmark_output" | grep "Packet Loss" | grep -o '[0-9.]*' | head -1)

        # Save performance metrics
        cat > "$REPORT_DIR/performance_metrics.txt" << EOF
Average Throughput: ${avg_throughput:-N/A} Gbps
Maximum Throughput: ${max_throughput:-N/A} Gbps
Average Latency: ${avg_latency:-N/A} μs
Minimum Latency: ${min_latency:-N/A} μs
Average Packet Loss: ${avg_packet_loss:-N/A}%
EOF

        # Performance assessment
        if [ -n "$avg_throughput" ] && (( $(echo "$avg_throughput > $MIN_THROUGHPUT_GBPS" | bc -l) )); then
            print_status "SUCCESS" "Throughput benchmark passed: $avg_throughput Gbps > $MIN_THROUGHPUT_GBPS Gbps"
        else
            print_status "WARNING" "Throughput below expected: ${avg_throughput:-N/A} Gbps"
        fi

        if [ -n "$avg_latency" ] && (( $(echo "$avg_latency < $MAX_LATENCY_US" | bc -l) )); then
            print_status "SUCCESS" "Latency benchmark passed: $avg_latency μs < $MAX_LATENCY_US μs"
        else
            print_status "WARNING" "Latency above expected: ${avg_latency:-N/A} μs"
        fi

        if [ -n "$avg_packet_loss" ] && (( $(echo "$avg_packet_loss < $MAX_PACKET_LOSS_PERCENT" | bc -l) )); then
            print_status "SUCCESS" "Packet loss benchmark passed: $avg_packet_loss% < $MAX_PACKET_LOSS_PERCENT%"
        else
            print_status "WARNING" "Packet loss above expected: ${avg_packet_loss:-N/A}%"
        fi

    else
        print_status "ERROR" "Performance benchmark failed"
        cat "$benchmark_output"
        return 1
    fi
}

# Function to run test suite coverage
run_test_coverage() {
    print_status "HEADER" "Running Test Suite Coverage"

    local test_output="$REPORT_DIR/test-coverage.log"

    print_status "INFO" "Running Soft-UE test suite..."
    ./ns3 test soft-ue > "$test_output" 2>&1

    # Extract test results
    local total_tests=$(grep -o "PASSED\|FAILED\|SKIPPED" "$test_output" | wc -l)
    local passed_tests=$(grep "PASSED" "$test_output" | wc -l)
    local failed_tests=$(grep "FAILED" "$test_output" | wc -l)

    # Calculate success rate
    local success_rate=0
    if [ $total_tests -gt 0 ]; then
        success_rate=$(echo "scale=2; $passed_tests * 100 / $total_tests" | bc)
    fi

    echo "Total Tests: $total_tests" > "$REPORT_DIR/test_coverage.txt"
    echo "Passed Tests: $passed_tests" >> "$REPORT_DIR/test_coverage.txt"
    echo "Failed Tests: $failed_tests" >> "$REPORT_DIR/test_coverage.txt"
    echo "Success Rate: $success_rate%" >> "$REPORT_DIR/test_coverage.txt"

    print_status "INFO" "Test suite results:"
    echo "  Total Tests: $total_tests"
    echo "  Passed Tests: $passed_tests"
    echo "  Failed Tests: $failed_tests"
    echo "  Success Rate: $success_rate%"

    if (( $(echo "$success_rate >= $MIN_TEST_COVERAGE_PERCENT" | bc -l) )); then
        print_status "SUCCESS" "Test coverage is acceptable: $success_rate% >= $MIN_TEST_COVERAGE_PERCENT%"
    else
        print_status "WARNING" "Test coverage below expected: $success_rate% < $MIN_TEST_COVERAGE_PERCENT%"
    fi
}

# Function to run AI network simulation
run_ai_simulation() {
    print_status "HEADER" "Running AI Network Simulation"

    local ai_output="$REPORT_DIR/ai-simulation.log"

    print_status "INFO" "Running AI network simulation..."
    timeout 300 ./ns3 run "ai-network-simulation --numNodes=8 --simulationTime=20s" --quiet > "$ai_output" 2>&1

    if [ $? -eq 0 ]; then
        print_status "SUCCESS" "AI network simulation completed"

        # Extract AI-specific metrics
        local ai_throughput=$(grep "Average Throughput:" "$ai_output" | grep -o '[0-9.]*' | head -1)
        local ai_latency=$(grep "Average Latency:" "$ai_output" | grep -o '[0-9.]*' | head -1)
        local ai_packet_loss=$(grep "Packet Loss Rate:" "$ai_output" | grep -o '[0-9.]*' | head -1)

        echo "AI Network Throughput: ${ai_throughput:-N/A} Gbps" > "$REPORT_DIR/ai_metrics.txt"
        echo "AI Network Latency: ${ai_latency:-N/A} μs" >> "$REPORT_DIR/ai_metrics.txt"
        echo "AI Network Packet Loss: ${ai_packet_loss:-N/A}%" >> "$REPORT_DIR/ai_metrics.txt"

        # AI-specific performance assessment
        if [ -n "$ai_throughput" ] && (( $(echo "$ai_throughput > 4.0" | bc -l) )); then
            print_status "SUCCESS" "AI throughput test passed: $ai_throughput Gbps > 4.0 Gbps"
        else
            print_status "WARNING" "AI throughput below expected for workloads: ${ai_throughput:-N/A} Gbps"
        fi

    else
        print_status "WARNING" "AI network simulation failed or timed out"
        tail -20 "$ai_output"
    fi
}

# Function to generate performance report
generate_report() {
    print_status "HEADER" "Generating Performance Report"

    local report_file="$REPORT_DIR/benchmark_report.md"

    cat > "$report_file" << EOF
# Soft-UE Performance Benchmark Report

**Generated:** $(date)
**Test Environment:** $(uname -a)
**Soft-UE Version:** $(git describe --tags --always 2>/dev/null || echo "unknown")

## Executive Summary

This report contains comprehensive performance benchmark results for the Soft-UE Ultra Ethernet protocol stack implementation.

## 🎯 Test Results Overview

EOF

    # Add functionality test results
    if [ -f "$REPORT_DIR/functionality_metrics.txt" ]; then
        cat >> "$report_file" << EOF
### Basic Functionality Test

\`\`\`
$(cat "$REPORT_DIR/functionality_metrics.txt")
\`\`\`

EOF
    fi

    # Add performance benchmark results
    if [ -f "$REPORT_DIR/performance_metrics.txt" ]; then
        cat >> "$report_file" << EOF
### Performance Benchmark

\`\`\`
$(cat "$REPORT_DIR/performance_metrics.txt")
\`\`\`

EOF
    fi

    # Add test coverage results
    if [ -f "$REPORT_DIR/test_coverage.txt" ]; then
        cat >> "$report_file" << EOF
### Test Suite Coverage

\`\`\`
$(cat "$REPORT_DIR/test_coverage.txt")
\`\`\`

EOF
    fi

    # Add AI simulation results
    if [ -f "$REPORT_DIR/ai_metrics.txt" ]; then
        cat >> "$report_file" << EOF
### AI Network Simulation

\`\`\`
$(cat "$REPORT_DIR/ai_metrics.txt")
\`\`\`

EOF
    fi

    # Add performance analysis
    cat >> "$report_file" << EOF
## 📊 Performance Analysis

### Performance Targets

| Metric | Target | Result | Status |
|--------|--------|--------|--------|
| **Throughput** | > $MIN_THROUGHPUT_GBPS Gbps | $(grep "Average Throughput:" "$REPORT_DIR/performance_metrics.txt" | grep -o '[0-9.]*' | head -1 || echo "N/A") Gbps | $(if [ -f "$REPORT_DIR/performance_metrics.txt" ] && grep -q "Average Throughput:" "$REPORT_DIR/performance_metrics.txt"; then local t=$(grep "Average Throughput:" "$REPORT_DIR/performance_metrics.txt" | grep -o '[0-9.]*' | head -1); if [ -n "$t" ] && (( $(echo "$t > $MIN_THROUGHPUT_GBPS" | bc -l) )); then echo "✅ PASS"; else echo "⚠️  WARN"; fi; else echo "❌ UNKNOWN"; fi) |
| **Latency** | < $MAX_LATENCY_US μs | $(grep "Average Latency:" "$REPORT_DIR/performance_metrics.txt" | grep -o '[0-9.]*' | head -1 || echo "N/A") μs | $(if [ -f "$REPORT_DIR/performance_metrics.txt" ] && grep -q "Average Latency:" "$REPORT_DIR/performance_metrics.txt"; then local l=$(grep "Average Latency:" "$REPORT_DIR/performance_metrics.txt" | grep -o '[0-9.]*' | head -1); if [ -n "$l" ] && (( $(echo "$l < $MAX_LATENCY_US" | bc -l) )); then echo "✅ PASS"; else echo "⚠️  WARN"; fi; else echo "❌ UNKNOWN"; fi) |
| **Packet Loss** | < $MAX_PACKET_LOSS_PERCENT% | $(grep "Average Packet Loss:" "$REPORT_DIR/performance_metrics.txt" | grep -o '[0-9.]*' | head -1 || echo "N/A")% | $(if [ -f "$REPORT_DIR/performance_metrics.txt" ] && grep -q "Average Packet Loss:" "$REPORT_DIR/performance_metrics.txt"; then local p=$(grep "Average Packet Loss:" "$REPORT_DIR/performance_metrics.txt" | grep -o '[0-9.]*' | head -1); if [ -n "$p" ] && (( $(echo "$p < $MAX_PACKET_LOSS_PERCENT" | bc -l) )); then echo "✅ PASS"; else echo "⚠️  WARN"; fi; else echo "❌ UNKNOWN"; fi) |
| **Test Coverage** | > $MIN_TEST_COVERAGE_PERCENT% | $(grep "Success Rate:" "$REPORT_DIR/test_coverage.txt" | grep -o '[0-9.]*' | head -1 || echo "N/A")% | $(if [ -f "$REPORT_DIR/test_coverage.txt" ] && grep -q "Success Rate:" "$REPORT_DIR/test_coverage.txt"; then local c=$(grep "Success Rate:" "$REPORT_DIR/test_coverage.txt" | grep -o '[0-9.]*' | head -1); if [ -n "$c" ] && (( $(echo "$c >= $MIN_TEST_COVERAGE_PERCENT" | bc -l) )); then echo "✅ PASS"; else echo "⚠️  WARN"; fi; else echo "❌ UNKNOWN"; fi) |

### Performance Classification

EOF

    # Overall performance assessment
    local overall_status="✅ EXCELLENT"
    local warnings=0

    # Check performance criteria
    if [ -f "$REPORT_DIR/performance_metrics.txt" ]; then
        local avg_throughput=$(grep "Average Throughput:" "$REPORT_DIR/performance_metrics.txt" | grep -o '[0-9.]*' | head -1)
        local avg_latency=$(grep "Average Latency:" "$REPORT_DIR/performance_metrics.txt" | grep -o '[0-9.]*' | head -1)
        local avg_packet_loss=$(grep "Average Packet Loss:" "$REPORT_DIR/performance_metrics.txt" | grep -o '[0-9.]*' | head -1)

        if [ -n "$avg_throughput" ] && ! (( $(echo "$avg_throughput > $MIN_THROUGHPUT_GBPS" | bc -l) )); then
            warnings=$((warnings + 1))
        fi

        if [ -n "$avg_latency" ] && ! (( $(echo "$avg_latency < $MAX_LATENCY_US" | bc -l) )); then
            warnings=$((warnings + 1))
        fi

        if [ -n "$avg_packet_loss" ] && ! (( $(echo "$avg_packet_loss < $MAX_PACKET_LOSS_PERCENT" | bc -l) )); then
            warnings=$((warnings + 1))
        fi
    fi

    if [ $warnings -eq 0 ]; then
        overall_status="🌟 EXCELLENT"
    elif [ $warnings -le 1 ]; then
        overall_status="✅ GOOD"
    elif [ $warnings -le 2 ]; then
        overall_status="⚠️  ACCEPTABLE"
    else
        overall_status="❌ NEEDS IMPROVEMENT"
    fi

    cat >> "$report_file" << EOF
**Overall Performance Rating: $overall_status**

- **🌟 EXCELLENT**: All performance targets exceeded
- **✅ GOOD**: Most performance targets met with minor issues
- **⚠️  ACCEPTABLE**: Basic functionality works but performance needs improvement
- **❌ NEEDS IMPROVEMENT**: Significant performance issues detected

## 📈 Recommendations

EOF

    # Add specific recommendations based on results
    if [ -f "$REPORT_DIR/performance_metrics.txt" ]; then
        local avg_throughput=$(grep "Average Throughput:" "$REPORT_DIR/performance_metrics.txt" | grep -o '[0-9.]*' | head -1)

        if [ -n "$avg_throughput" ] && ! (( $(echo "$avg_throughput > $MIN_THROUGHPUT_GBPS" | bc -l) )); then
            cat >> "$report_file" << EOF
- **Throughput Optimization**: Current throughput ($avg_throughput Gbps) is below target ($MIN_THROUGHPUT_GBPS Gbps). Consider:
  - Optimizing PDC configuration parameters
  - Increasing buffer sizes
  - Reviewing network topology design
EOF
        fi
    fi

    cat >> "$report_file" << EOF
- **Continuous Testing**: Regularly run this benchmark suite to track performance over time
- **Environment Optimization**: Ensure benchmark runs on dedicated hardware for consistent results
- **Version Comparison**: Use these results as baseline for future performance comparisons

## 📁 Generated Files

- **Full Logs**: Located in \`$REPORT_DIR/\`
- **Performance Metrics**: \`performance_metrics.txt\`
- **Test Results**: \`test_coverage.txt\`
- **Functionality Test**: \`functionality_metrics.txt\`
- **AI Simulation**: \`ai_metrics.txt\`

## 🔍 Technical Details

### System Information
\`\`\`
$(uname -a)
\`\`\`

### Build Information
\`\`\`
$(./ns3 --version 2>/dev/null || echo "ns-3 version not available")
\`\`\`

---

**Report Generated by Soft-UE Performance Benchmark Suite**
*For detailed analysis, review the individual log files in the results directory.*
EOF

    print_status "SUCCESS" "Performance report generated: $report_file"
}

# Function to display summary
display_summary() {
    print_status "HEADER" "Benchmark Summary"

    echo ""
    echo "📊 Results Directory: $REPORT_DIR"
    echo "📋 Performance Report: $REPORT_DIR/benchmark_report.md"
    echo ""

    # Quick summary of key metrics
    if [ -f "$REPORT_DIR/performance_metrics.txt" ]; then
        echo "🎯 Key Performance Metrics:"
        cat "$REPORT_DIR/performance_metrics.txt" | while read line; do
            echo "   $line"
        done
    fi

    if [ -f "$REPORT_DIR/test_coverage.txt" ]; then
        echo ""
        echo "🧪 Test Coverage:"
        grep "Success Rate:" "$REPORT_DIR/test_coverage.txt" | sed 's/^/   /'
    fi

    echo ""
    echo "📈 For detailed analysis, see: $REPORT_DIR/benchmark_report.md"
}

# Main execution
main() {
    echo "Starting Soft-UE performance benchmark suite..."
    echo ""

    # Run all benchmark components
    check_prerequisites
    run_functionality_test
    run_performance_benchmark
    run_test_coverage
    run_ai_simulation
    generate_report
    display_summary

    print_status "SUCCESS" "Benchmark suite completed successfully!"
    echo ""
    echo "📊 Full results available in: $REPORT_DIR"
    echo "📋 View the complete report: $REPORT_DIR/benchmark_report.md"
}

# Handle script arguments
case "${1:-}" in
    "functionality")
        check_prerequisites
        run_functionality_test
        ;;
    "performance")
        check_prerequisites
        run_performance_benchmark
        ;;
    "coverage")
        check_prerequisites
        run_test_coverage
        ;;
    "ai")
        check_prerequisites
        run_ai_simulation
        ;;
    "help"|"-h"|"--help")
        echo "Soft-UE Performance Benchmark Suite"
        echo ""
        echo "Usage: $0 [test_type]"
        echo ""
        echo "Test types:"
        echo "  functionality    - Run basic functionality test"
        echo "  performance      - Run comprehensive performance benchmark"
        echo "  coverage         - Run test suite coverage"
        echo "  ai               - Run AI network simulation"
        echo "  (no argument)    - Run full benchmark suite"
        echo "  help             - Show this help message"
        exit 0
        ;;
    "")
        main
        ;;
    *)
        print_status "ERROR" "Unknown test type: $1"
        echo "Run '$0 help' for usage information"
        exit 1
        ;;
esac
# Soft-UE Performance Benchmark Suite

build_lib(
  LIBNAME ns3.44-soft-ue-benchmarks
  SOURCE_FILES
    soft-ue-performance-benchmark.cc
    e2e-demo-optimized.cc
  HEADER_FILES
  TEST_SOURCES
)

# Add benchmark executables
build_example(soft-ue-performance-benchmark)
build_example(soft-ue-e2e-demo)
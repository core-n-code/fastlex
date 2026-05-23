find_package(benchmark REQUIRED)

add_executable(fastlex_ascii_benchmark ascii_benchmark.cpp)
target_link_libraries(fastlex_ascii_benchmark PRIVATE fastlex::fastlex benchmark::benchmark)

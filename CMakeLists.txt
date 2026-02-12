cmake_minimum_required(VERSION 3.20)
project(hft_concurrency_playground LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
  add_compile_options(/W4 /permissive-)
else()
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

add_library(hft_headers INTERFACE)
target_include_directories(hft_headers INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_executable(bench_spsc src/bench_spsc.cpp)
target_link_libraries(bench_spsc PRIVATE hft_headers)

add_executable(bench_mutex_queue src/bench_mutex_queue.cpp)
target_link_libraries(bench_mutex_queue PRIVATE hft_headers)

add_executable(false_sharing_demo src/false_sharing_demo.cpp)
target_link_libraries(false_sharing_demo PRIVATE hft_headers)

add_executable(memory_order_demo src/memory_order_demo.cpp)
target_link_libraries(memory_order_demo PRIVATE hft_headers)

add_executable(test_spsc tests/test_spsc.cpp)
target_link_libraries(test_spsc PRIVATE hft_headers)

enable_testing()
add_test(NAME test_spsc COMMAND test_spsc)

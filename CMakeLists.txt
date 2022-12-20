cmake_minimum_required(VERSION 3.23)
project(test)

set(SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp")

add_executable(network-monitor ${SOURCES})
target_compile_definitions(network-monitor PRIVATE cxx_std_17)

cmake_minimum_required(VERSION 3.23)
project(LiveTransportNetworkMonitor)

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/conaninfo.txt)
	list(PREPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_BINARY_DIR}")
endif()
find_package(Boost 1.80 REQUIRED COMPONENTS system)

# NetworkMonitor library
set(NETOWRK_MONITOR_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/WebSocketClient.cpp")
add_library(NetworkMonitor STATIC ${NETOWRK_MONITOR_SOURCES})
target_link_libraries(NetworkMonitor PUBLIC Boost::system)
target_include_directories(NetworkMonitor PRIVATE 
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/NetworkMonitor>)
target_compile_definitions(NetworkMonitor PRIVATE cxx_std_20)

# Standalone driver
set(SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp")
add_executable(network-monitor ${SOURCES})
target_include_directories(network-monitor PRIVATE 
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)
target_compile_definitions(network-monitor PRIVATE cxx_std_20)
target_link_libraries(network-monitor PRIVATE NetworkMonitor)

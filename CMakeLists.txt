cmake_minimum_required(VERSION 3.23)
project(LiveTransportNetworkMonitor)

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/conaninfo.txt)
	list(PREPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_BINARY_DIR}")
endif()

find_package(Boost 1.80 REQUIRED COMPONENTS system unit_test_framework)
find_package(OpenSSL REQUIRED)
find_package(CURL REQUIRED)

add_compile_options(-fsanitize=address -O0 -g -std=c++20)
add_link_options(-fsanitize=address)

# Structures library
set(
	STRUCTURES_SOURCES
	"${CMAKE_CURRENT_SOURCE_DIR}/src/TransportNetwork.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/TransportNetworkParser.cpp"
)

add_library(
	Structures
	STATIC
	${STRUCTURES_SOURCES}
)

target_include_directories(
	Structures
	PUBLIC 
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/Structures>
)

target_compile_definitions(
	Structures
	PRIVATE
	cxx_std_20
)

target_link_libraries(
	Structures
	PUBLIC
	Boost::system
)

# NetworkMonitor library
set(
	NETOWRK_MONITOR_SOURCES
	"${CMAKE_CURRENT_SOURCE_DIR}/src/WebSocketClient.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/FileDownloader.cpp"
)

add_library(
	NetworkMonitor
	STATIC
	${NETOWRK_MONITOR_SOURCES}
)

target_include_directories(
	NetworkMonitor
	PRIVATE 
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/NetworkMonitor>
)

target_compile_definitions(
	NetworkMonitor
	PRIVATE
	cxx_std_20
)

target_link_libraries(
	NetworkMonitor
	PUBLIC
	Boost::system
	OpenSSL::SSL
	CURL::libcurl
)

# Unit tests
set(
	TEST_SOURCES
	"${CMAKE_CURRENT_SOURCE_DIR}/tests/main.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/tests/websocket-client.cpp"
)

add_executable(
	network-monitor-tests
	${TEST_SOURCES}
)

target_include_directories(
	network-monitor-tests
	PRIVATE 
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_compile_definitions(network-monitor-tests
	PRIVATE
	cxx_std_20
	"-DBOOST_TEST_DYN_LINK=1" # TODO: Is there more elegant way to do this? :/
	TESTS_CACERT_PEM="${CMAKE_CURRENT_SOURCE_DIR}/tests/cacert.pem"
	TESTS_NETWORK_LAYOUT_PATH="${CMAKE_CURRENT_SOURCE_DIR}/tests/network-layout.json"
)

target_link_libraries(
	network-monitor-tests
	PRIVATE
	NetworkMonitor
	Boost::unit_test_framework
	OpenSSL::SSL
	CURL::libcurl
)

# TransportNetwork unit tests
set(
	STRUCTURES_TEST_SOURCES
	"${CMAKE_CURRENT_SOURCE_DIR}/tests/main.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/tests/transport-network.cpp"
)

add_executable(
	transport-network-tests
	${STRUCTURES_TEST_SOURCES}
)

target_include_directories(
	transport-network-tests
	PRIVATE 
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_compile_definitions(
	transport-network-tests
	PRIVATE
	cxx_std_20
	"-DBOOST_TEST_DYN_LINK=1" # TODO: Is there more elegant way to do this? :/
	TESTS_NETWORK_LAYOUT_PATH="${CMAKE_CURRENT_SOURCE_DIR}/tests/network-layout.json"
)

target_link_libraries(
	transport-network-tests
	PRIVATE
	Structures
	Boost::unit_test_framework
	OpenSSL::SSL
	CURL::libcurl
)

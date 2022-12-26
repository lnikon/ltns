cmake_minimum_required(VERSION 3.23)
project(LiveTransportNetworkMonitor)

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/conaninfo.txt)
	list(PREPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_BINARY_DIR}")
endif()

find_package(Boost 1.80 REQUIRED COMPONENTS system unit_test_framework)
find_package(OpenSSL REQUIRED)
find_package(CURL REQUIRED)

# NetworkMonitor library
set(
	NETOWRK_MONITOR_SOURCES
	"${CMAKE_CURRENT_SOURCE_DIR}/src/WebSocketClient.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/FileDownloader.cpp"
)

add_library(NetworkMonitor
	STATIC
	${NETOWRK_MONITOR_SOURCES}
)

target_link_libraries(
	NetworkMonitor
	PUBLIC
	Boost::system
	OpenSSL::SSL
	CURL::libcurl
)

target_include_directories(
	NetworkMonitor
	PRIVATE 
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/NetworkMonitor>
)

target_compile_definitions(
	NetworkMonitor PRIVATE
	cxx_std_20
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

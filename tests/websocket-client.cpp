#include <NetworkMonitor/Clients/WebSocketClient.h>
#include <NetworkMonitor/Utilities/FileDownloader.h>

#include <boost/asio.hpp>
#include <boost/asio/ssl/context.hpp>
#include <boost/test/tools/old/interface.hpp>
#include <boost/test/unit_test.hpp>

#include <charconv>
#include <filesystem>
#include <string>
#include <string_view>

using namespace std::string_literals;

BOOST_AUTO_TEST_SUITE(NetworkMonitor);

bool checkInvalidAuthenticationResponse(std::string_view response)
{
  bool ok{response.find("ValidationInvalidAuth") != std::string::npos};
  return ok;
}

BOOST_AUTO_TEST_CASE(cacert_pem)
{
  BOOST_CHECK(std::filesystem::exists(TESTS_CACERT_PEM));
}

BOOST_AUTO_TEST_CASE(function_DownloadFile)
{
  const std::string url{
    "https://ltnm.learncppthroughprojects.com/network-layout.json"};
  Networking::Utilities::DownloadFile(
    Networking::Utilities::FileDownloadOptions{
      .url{url},
      .destinationPath{TESTS_NETWORK_LAYOUT_PATH}});
  BOOST_CHECK(std::filesystem::exists(TESTS_NETWORK_LAYOUT_PATH));
}

BOOST_AUTO_TEST_CASE(class_WebSocketClient)
{
  boost::asio::ssl::context ctx{boost::asio::ssl::context::tlsv12_client};
  ctx.load_verify_file(TESTS_CACERT_PEM);
  boost::asio::io_context ioc{};

  std::string host{"ltnm.learncppthroughprojects.com"};
  std::string endpoint{"/network-events"};
  std::string port{"443"};
  std::string message{"STOMP\n"
                      "accept-version:1.2\n"
                      "host:ltnm.learncppthroughprojects.com\n"
                      "login:vagag\n"
                      "passcode:vagagord\n\n"
                      "\0"s};

  bool connected{false};
  bool messageSent{false};
  bool messageReceived{false};
  bool disconnected{false};
  std::string response{};

  auto pWsClient{std::make_shared<Networking::Clients::WebSocketClient>(
    host,
    port,
    endpoint,
    ioc,
    ctx)};

  auto onSend{[&messageSent](auto ec) { messageSent = !ec; }};

  auto onConnect{[&pWsClient, &connected, &onSend, &message](auto ec) {
    connected = !ec;
    if (!ec) {
      pWsClient->Send(onSend, message);
    }
  }};

  auto onClose{[&disconnected](auto ec) { disconnected = !ec; }};

  auto onReceive{[&pWsClient, &onClose, &messageReceived, &response](
                   auto ec,
                   auto received) {
    messageReceived = !ec;
    response = std::move(received);
    pWsClient->Disconnect(onClose);
  }};

  pWsClient->Connect(onConnect, onReceive, nullptr);

  ioc.run();

  BOOST_CHECK(connected);
  BOOST_CHECK(messageSent);
  BOOST_CHECK(messageReceived);
  BOOST_CHECK(disconnected);
  BOOST_CHECK(checkInvalidAuthenticationResponse(response));
}

BOOST_AUTO_TEST_SUITE_END();

#include <NetworkMonitor/WebSocketClient.h>

#include <boost/asio.hpp>
#include <boost/asio/ssl/context.hpp>
#include <boost/test/tools/old/interface.hpp>
#include <boost/test/unit_test.hpp>

#include <string>
#include <filesystem>

BOOST_AUTO_TEST_SUITE(NetworkMonitor);

BOOST_AUTO_TEST_CASE(cacert_pem)
{
  BOOST_CHECK(std::filesystem::exists(TESTS_CACERT_PEM));
}

BOOST_AUTO_TEST_CASE(class_WebSocketClient)
{
  boost::asio::ssl::context ctx{boost::asio::ssl::context::tlsv12_client};
  ctx.load_verify_file(TESTS_CACERT_PEM);
  boost::asio::io_context ioc{};

  std::string host{"ltnm.learncppthroughprojects.com"};
  std::string endpoint{"/echo"};
  std::string port{"443"};
  std::string message{"may the force be with you"};

  bool connected{false};
  bool messageSent{false};
  bool messageReceived{false};
  bool disconnected{false};
  std::string echo{};

  auto pWsClient{std::make_shared<WebSocketClient>(host, port, endpoint, ioc, ctx)};

  auto onSend{[&messageSent](auto ec) { messageSent = !ec; }};

  auto onConnect{[&pWsClient, &connected, &onSend, &message](auto ec) {
    connected = !ec;
    if (!ec) {
      pWsClient->Send(onSend, message);
    }
  }};

  auto onClose{[&disconnected](auto ec) { disconnected = !ec; }};

  auto onReceive{
    [&pWsClient, &onClose, &messageReceived, &echo](auto ec, auto received) {
      messageReceived = !ec;
      echo = std::move(received);
      pWsClient->Disconnect(onClose);
    }};

  pWsClient->Connect(onConnect, onReceive, nullptr);

  ioc.run();

  BOOST_CHECK(connected);
  BOOST_CHECK(messageSent);
  BOOST_CHECK(messageReceived);
  BOOST_CHECK(disconnected);
  BOOST_CHECK_EQUAL(message, echo);
}

BOOST_AUTO_TEST_SUITE_END();

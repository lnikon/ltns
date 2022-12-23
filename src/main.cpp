#include "NetworkMonitor/WebSocketClient.h"
#include <boost/asio/io_context.hpp>
#include <thread>

using namespace std::chrono_literals;

int main() {
  boost::asio::io_context ioc{};

  std::string host{"ltnm.learncppthroughprojects.com"};
  std::string port{"80"};
  std::string payload{"may the force be with you"};

  auto pWsClient{std::make_shared<WebSocketClient>(host, port, ioc)};

  pWsClient->Connect(
      [pWsClient, &payload](auto ec) {
        if (!ec) {
          pWsClient->Send(nullptr, payload);
        }
      },
      [](auto ec, auto message) {
        std::cout << "[onMessage]: message=" << message << std::endl;
      },
      nullptr);

  ioc.run();

  return 0;
}

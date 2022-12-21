#include "WebSocketClient.h"
#include <boost/asio/io_context.hpp>

int main() {
  boost::asio::io_context ioc{};
  //auto pWsClient{std::make_shared<WebSocketClient>(ioc)};

  std::shared_ptr<WebSocketClient> pWsClient{new WebSocketClient(ioc)};

  std::string host{"ltnm.learncppthroughprojects.com"};
  std::string port{"80"};
  std::string payload{"may the force be with you"};

  pWsClient->Run(host, port, payload);

  ioc.run();

  return 0;
}

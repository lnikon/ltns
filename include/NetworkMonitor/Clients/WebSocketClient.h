#pragma once

#include <iomanip>
#include <iostream>
#include <memory>
#include <string>
#include <thread>

#include <boost/asio.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/address.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/asio/ssl/stream_base.hpp>
#include <boost/asio/strand.hpp>
#include <boost/beast.hpp>
#include <boost/beast/core/bind_handler.hpp>
#include <boost/beast/core/buffers_to_string.hpp>
#include <boost/beast/core/error.hpp>
#include <boost/beast/core/flat_buffer.hpp>
#include <boost/beast/core/make_printable.hpp>
#include <boost/beast/core/stream_traits.hpp>
#include <boost/beast/core/tcp_stream.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/websocket/rfc6455.hpp>
#include <boost/beast/websocket/stream.hpp>
#include <boost/core/ignore_unused.hpp>
#include <boost/system/detail/error_code.hpp>
#include <boost/system/error_code.hpp>

namespace Networking::Clients {

using tcp = boost::asio::ip::tcp;

void Log(boost::system::error_code ec);
void LogThisThreadId(const std::string &msg = "");

class WebSocketClient : public std::enable_shared_from_this<WebSocketClient> {
public:
  using OnConnectType = std::function<void(boost::beast::error_code)>;
  using OnDisconnectType = std::function<void(boost::beast::error_code)>;
  using OnSendType = std::function<void(boost::beast::error_code)>;
  using OnMessageType =
    std::function<void(boost::beast::error_code, std::string)>;

  WebSocketClient(
    std::string host,
    std::string port,
    std::string endpoint,
    boost::asio::io_context &ioc,
    boost::asio::ssl::context &ctx);

  WebSocketClient(const WebSocketClient &) = delete;
  WebSocketClient(WebSocketClient &&) = delete;

  WebSocketClient &operator=(const WebSocketClient &) = delete;
  WebSocketClient &operator=(WebSocketClient &&) = delete;

  ~WebSocketClient() = default;

  void Connect(
    OnConnectType onConnect = nullptr,
    OnMessageType onMessage = nullptr,
    OnDisconnectType onDisconnect = nullptr);

  void Send(OnSendType onSend, const std::string &message);
  void Disconnect(OnDisconnectType onDisconnect);

private:
  void
  onResolve(boost::beast::error_code ec, tcp::resolver::results_type results);

  void onConnect(
    boost::beast::error_code ec,
    tcp::resolver::results_type::endpoint_type endpoint);

  void onTlsHandshake(boost::beast::error_code ec);
  void onHandshake(boost::beast::error_code ec);
  void listenForIncomingMessages(boost::beast::error_code ec);
  void onRead(boost::beast::error_code ec, std::size_t nBytes);

private:
  boost::asio::io_context &m_ioc;
  boost::asio::ssl::context &m_ctx;
  std::string m_host;
  std::string m_port;
  std::string m_endpoint;
  tcp::resolver m_resolver;
  boost::beast::websocket::stream<
    boost::beast::ssl_stream<boost::beast::tcp_stream>>
    m_ws;

  OnConnectType m_onConnect;
  OnMessageType m_onMessage;
  OnDisconnectType m_onDisconnect;
  boost::beast::flat_buffer m_buffer;
};

} // namespace Networking::Clients

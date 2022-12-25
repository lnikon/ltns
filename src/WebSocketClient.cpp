#include "WebSocketClient.h"
#include <boost/asio/ssl/stream_base.hpp>
#include <boost/beast/core/role.hpp>
#include <boost/beast/core/stream_traits.hpp>
#include <openssl/tls1.h>

WebSocketClient::WebSocketClient(
  std::string host,
  std::string port,
  std::string endpoint,
  boost::asio::io_context &ioc,
  boost::asio::ssl::context &ctx)
    : m_ioc(ioc),
      m_ctx(ctx),
      m_host(host),
      m_port(port),
      m_endpoint(endpoint),
      m_resolver(boost::asio::make_strand(m_ioc)),
      m_ws(boost::asio::make_strand(m_ioc), ctx)
{
}

void WebSocketClient::Connect(
  OnConnectType onConnect,
  OnMessageType onMessage,
  OnDisconnectType onDisconnect)
{
  m_onConnect = onConnect;
  m_onMessage = onMessage;
  m_onDisconnect = onDisconnect;

  m_resolver.async_resolve(
    m_host,
    m_port,
    [this](boost::beast::error_code ec, tcp::resolver::results_type results) {
      onResolve(ec, results);
    });
}

void WebSocketClient::Send(OnSendType onSend, const std::string &message)
{
  std::cout << "[WebSocketClient::Send]: Sending message: message=" << message
            << std::endl;
  m_ws.async_write(
    boost::asio::buffer(message),
    [onSend](boost::beast::error_code ec, auto /* nBytes */) {
      if (onSend) {
        onSend(ec);
      }
    });
}

void WebSocketClient::Disconnect(OnDisconnectType onDisconnect)
{
  m_ws.async_close(
    boost::beast::websocket::close_code::none,
    [onDisconnect](boost::beast::error_code ec) {
      if (onDisconnect) {
        onDisconnect(ec);
      }
    });
}

void WebSocketClient::onResolve(
  boost::beast::error_code ec,
  tcp::resolver::results_type results)
{
  if (ec) {
    if (m_onConnect) {
      m_onConnect(ec);
    }

    Log(ec);
    return;
  }

  boost::beast::get_lowest_layer(m_ws).expires_after(std::chrono::seconds(30));

  boost::beast::get_lowest_layer(m_ws).async_connect(
    results,
    [this](
      boost::beast::error_code ec,
      tcp::resolver::results_type::endpoint_type endpoint) {
      onConnect(ec, endpoint);
    });
}

void WebSocketClient::onConnect(
  boost::beast::error_code ec,
  tcp::resolver::results_type::endpoint_type endpoint)
{
  if (ec) {
    Log(ec);
    return;
  }

  boost::beast::get_lowest_layer(m_ws).expires_never();
  m_ws.set_option(boost::beast::websocket::stream_base::timeout::suggested(
    boost::beast::role_type::client));

  SSL_set_tlsext_host_name(m_ws.next_layer().native_handle(), m_host.c_str());

  m_ws.next_layer().async_handshake(
    boost::asio::ssl::stream_base::client,
    [this](auto ec) { onTlsHandshake(ec); });
}

void WebSocketClient::onTlsHandshake(boost::beast::error_code ec)
{
  if (ec) {
    Log(ec);
    return;
  }

  m_host += ":" + m_port;

  m_ws.async_handshake(m_host, m_endpoint, [this](auto ec) {
    onHandshake(ec);
  });
}

void WebSocketClient::onHandshake(boost::beast::error_code ec)
{
  if (ec) {
    Log(ec);
    return;
  }

  m_ws.text(true);

  listenForIncomingMessages(ec);

  if (m_onConnect) {
    m_onConnect(ec);
  }
}

void WebSocketClient::listenForIncomingMessages(boost::beast::error_code ec)
{
  if (ec) {
    Log(ec);
    return;
  }

  std::cout << "[WebSocketClient::listenForIncomingMessages]: Listening for "
               "incoming messages..."
            << std::endl;

  m_ws.async_read(
    m_buffer,
    [this](boost::beast::error_code ec, std::size_t nBytes) {
      onRead(ec, nBytes);
      listenForIncomingMessages(ec);
    });
}

void WebSocketClient::onRead(boost::beast::error_code ec, std::size_t nBytes)
{
  boost::ignore_unused(nBytes);

  if (ec) {
    Log(ec);
    return;
  }

  std::cout << "[WebSocketClient::onRead]: onRead called!" << std::endl;

  std::string message{boost::beast::buffers_to_string(m_buffer.data())};
  m_buffer.consume(nBytes);
  if (m_onMessage) {
    m_onMessage(ec, message);
  }
}

void LogThisThreadId(const std::string &msg)
{
  std::cout << "[" << std::setw(14) << std::this_thread::get_id() << "] " << msg
            << std::endl;
}

void Log(boost::system::error_code ec)
{
  std::cerr << (ec ? "Error: " : "OK") << (ec ? ec.message() : "") << std::endl;
}

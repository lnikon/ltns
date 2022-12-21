#include <boost/beast/core/bind_handler.hpp>
#include <boost/beast/core/error.hpp>
#include <boost/beast/core/flat_buffer.hpp>
#include <boost/beast/core/make_printable.hpp>
#include <boost/beast/core/stream_traits.hpp>
#include <boost/beast/websocket/rfc6455.hpp>
#include <boost/core/ignore_unused.hpp>
#include <iomanip>
#include <iostream>
#include <memory>
#include <string>
#include <thread>

#include <boost/asio.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/address.hpp>
#include <boost/asio/strand.hpp>
#include <boost/beast.hpp>
#include <boost/beast/core/tcp_stream.hpp>
#include <boost/beast/websocket/stream.hpp>
#include <boost/system/detail/error_code.hpp>
#include <boost/system/error_code.hpp>

using tcp = boost::asio::ip::tcp;

void Log(boost::system::error_code ec) {
  std::cerr << (ec ? "Error: " : "OK") << (ec ? ec.message() : "") << std::endl;
}

void LogThisThreadId(const std::string &msg = "") {
  std::cout << "[" << std::setw(14) << std::this_thread::get_id() << "] " << msg
            << std::endl;
}

class WebSocketClient : public std::enable_shared_from_this<WebSocketClient> {
public:
  WebSocketClient(boost::asio::io_context &ioc)
      : m_ioc(ioc), m_resolver(boost::asio::make_strand(m_ioc)),
        m_ws(boost::asio::make_strand(m_ioc)) {}

  void Run(std::string host, std::string port,
           std::string payload) {
    m_host = host;
    m_port = port;
    m_payload = payload;

    m_resolver.async_resolve(
        m_host, m_port,
        boost::beast::bind_front_handler(&WebSocketClient::OnResolve,
                                         shared_from_this()));
  }

  void OnResolve(boost::beast::error_code ec,
                 tcp::resolver::results_type results) {
    if (ec) {
      Log(ec);
      return;
    }

    boost::beast::get_lowest_layer(m_ws).expires_after(
        std::chrono::seconds(30));

    boost::beast::get_lowest_layer(m_ws).async_connect(
        results, boost::beast::bind_front_handler(&WebSocketClient::OnConnect,
                                                  shared_from_this()));
  }

  void OnConnect(boost::beast::error_code ec,
                 tcp::resolver::results_type::endpoint_type endpoint) {
    if (ec) {
      Log(ec);
      return;
    }

    m_host += ":" + std::to_string(endpoint.port());

    m_ws.async_handshake(
        m_host, "/echo",
        boost::beast::bind_front_handler(&WebSocketClient::OnHandshake,
                                         shared_from_this()));
  }

  void OnHandshake(boost::beast::error_code ec) {
    if (ec) {
      Log(ec);
      return;
    }

    m_ws.async_write(boost::asio::buffer(m_payload),
                     boost::beast::bind_front_handler(&WebSocketClient::OnWrite,
                                                      shared_from_this()));
  }

  void OnWrite(boost::beast::error_code ec, std::size_t bytesTransferred) {
    boost::ignore_unused(bytesTransferred);

    if (ec) {
      Log(ec);
      return;
    }

    m_ws.async_read(m_buffer,
                    boost::beast::bind_front_handler(&WebSocketClient::OnRead,
                                                     shared_from_this()));
  }

  void OnRead(boost::beast::error_code ec, std::size_t bytesTransferred) {
    boost::ignore_unused(bytesTransferred);

    if (ec) {
      Log(ec);
      return;
    }

    m_ws.async_close(boost::beast::websocket::close_code::normal,
                     boost::beast::bind_front_handler(&WebSocketClient::OnClose,
                                                      shared_from_this()));
  }

  void OnClose(boost::beast::error_code ec) {
    if (ec) {
      Log(ec);
      return;
    }

    std::cout << boost::beast::make_printable(m_buffer.data()) << std::endl;
  }

private:
  boost::asio::io_context &m_ioc;
  tcp::resolver m_resolver;
  boost::beast::websocket::stream<boost::beast::tcp_stream> m_ws;

  std::string m_host;
  std::string m_port;
  std::string m_payload;

  boost::beast::flat_buffer m_buffer;
};


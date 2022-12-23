#include <boost/asio/buffer.hpp>
#include <boost/beast/core/bind_handler.hpp>
#include <boost/beast/core/buffers_to_string.hpp>
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
  using OnConnectType = std::function<void(boost::beast::error_code)>;
  using OnDisconnectType = std::function<void(boost::beast::error_code)>;
  using OnSendType = std::function<void(boost::beast::error_code)>;
  using OnMessageType =
      std::function<void(boost::beast::error_code, std::string)>;

  WebSocketClient(std::string host, std::string port,
                  boost::asio::io_context &ioc)
      : m_host(host), m_port(port), m_ioc(ioc),
        m_resolver(boost::asio::make_strand(m_ioc)),
        m_ws(boost::asio::make_strand(m_ioc)) {}

  void Connect(OnConnectType onConnect = nullptr,
               OnMessageType onMessage = nullptr,
               OnDisconnectType onDisconnect = nullptr) {
    m_onConnect = onConnect;
    m_onMessage = onMessage;
    m_onDisconnect = onDisconnect;

    m_resolver.async_resolve(m_host, m_port,
                             [this](boost::beast::error_code ec,
                                    tcp::resolver::results_type results) {
                               OnResolve(ec, results);
                             });
  }

  void Send(OnSendType onSend, const std::string &message) {
    std::cout << "[WebSocketClient::Send]: Sending message: message=" << message << std::endl;
    m_ws.async_write(boost::asio::buffer(message),
                     [onSend](boost::beast::error_code ec, auto /* nBytes */) {
                       if (onSend) {
                         onSend(ec);
                       }
                     });
  }

  void Disconnect(OnDisconnectType onDisconnect) {
    m_ws.async_close(boost::beast::websocket::close_code::none,
                     [onDisconnect](boost::beast::error_code ec) {
                       if (onDisconnect) {
                         onDisconnect(ec);
                       }
                     });
  }

  void OnResolve(boost::beast::error_code ec,
                 tcp::resolver::results_type results) {
    if (ec) {
      if (m_onConnect) {
        m_onConnect(ec);
      }

      Log(ec);
      return;
    }

    boost::beast::get_lowest_layer(m_ws).expires_after(
        std::chrono::seconds(30));

    boost::beast::get_lowest_layer(m_ws).async_connect(
        results, [this](boost::beast::error_code ec,
                        tcp::resolver::results_type::endpoint_type endpoint) {
          OnConnect(ec, endpoint);
        });
  }

  void OnConnect(boost::beast::error_code ec,
                 tcp::resolver::results_type::endpoint_type endpoint) {
    if (ec) {
      Log(ec);
      return;
    }

    m_host += ":" + std::to_string(endpoint.port());

    m_ws.next_layer().expires_never();
    m_ws.async_handshake(m_host, "/echo", [this](boost::beast::error_code ec) {
      OnHandshake(ec);
    });
  }

  void OnHandshake(boost::beast::error_code ec) {
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

  void listenForIncomingMessages(boost::beast::error_code ec) {
    if (ec) {
      Log(ec);
      return;
    }

    std::cout << "[WebSocketClient::listenForIncomingMessages]: Listening for incoming messages..." << std::endl;

    m_ws.async_read(m_buffer,
                    [this](boost::beast::error_code ec, std::size_t nBytes) {
                      onRead(ec, nBytes);
                      listenForIncomingMessages(ec);
                    });
  }

  void onRead(boost::beast::error_code ec, std::size_t nBytes) {
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

private:
  boost::asio::io_context &m_ioc;
  tcp::resolver m_resolver;
  boost::beast::websocket::stream<boost::beast::tcp_stream> m_ws;

  std::string m_host;
  std::string m_port;
  std::string m_payload;

  OnConnectType m_onConnect;
  OnMessageType m_onMessage;
  OnDisconnectType m_onDisconnect;

  boost::beast::flat_buffer m_buffer;
};

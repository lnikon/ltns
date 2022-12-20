#include <boost/asio/strand.hpp>
#include <iomanip>
#include <iostream>
#include <thread>

#include <boost/asio.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/address.hpp>
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

void OnConnect(boost::system::error_code ec) { Log(ec); }

int main() {
  LogThisThreadId("main");

  const std::size_t nThreads{4};

  boost::asio::io_context ioc{};
  boost::system::error_code ec{};

  tcp::socket socket{boost::asio::make_strand(ioc)};
  tcp::resolver resolver{ioc};
  auto resolverIt{resolver.resolve("google.com", "80", ec)};
  if (ec) {
    Log(ec);
    return -1;
  }

  // socket.connect(*resolverIt, ec);
  // if (ec) {
  //   Log(ec);
  //   return -1;
  // }

  // Log(ec);

  for (std::size_t idx{0}; idx < nThreads; ++idx) {
    socket.async_connect(*resolverIt, OnConnect);
  }

  std::vector<std::thread> threads{};
  threads.reserve(nThreads);
  for (std::size_t idx{0}; idx < nThreads; ++idx) {
    threads.emplace_back([&ioc]() {
      LogThisThreadId("ioc.run()");
      ioc.run();
    });
  }

  for (auto &thread : threads) {
    thread.join();
  }

  return 0;
}

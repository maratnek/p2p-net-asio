#ifndef __SESSION__HPP__
#define __SESSION__HPP__

#include <logger.hpp>

#include "server.hpp"

#include <boost/asio.hpp>

#include <iostream>
#include <list>
#include <memory>
#include <thread>

using namespace std::chrono_literals;
using boost::asio::ip::tcp;

constexpr char cDelimiter = '\n';

/// \class Session
class Session : public std::enable_shared_from_this<Session> {
public:
  Session(tcp::socket socket, TAddress address, Server &server)
      : m_socket(std::move(socket))
      , m_receiveBuffer(1024)
      , m_server(server) 
      , m_address(address)
  {}

  // delete useless constructors
  Session() = delete;
  Session(Session const &) = delete;
  Session(Session &&session) = delete;
  ~Session();

  std::string getRemoteEndpoint() const;

  inline void start() { this->receiving(); }

  void handleWrite(std::string const& mes, boost::system::error_code ec,
                   std::size_t bytesTransferred) const;

  void sendMessage(std::string mes);

  /// @brief Subscribe to receive messages
  /// @param lamda funciton for subscription
  void addReceiveHandler(TReceiveHandler lamda);

  inline TAddress getAddress() const { return m_address; }

private:
  void handleRead(boost::system::error_code ec, std::size_t bytesTransferred);

  void receiving();

private:
  Server &m_server;
  tcp::socket m_socket;
  boost::asio::streambuf m_receiveBuffer;

  TReceiveHandler m_receiveHandler = nullptr;

  TAddress m_address;
};

#endif // __SESSION__HPP__
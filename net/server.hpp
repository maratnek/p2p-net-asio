#ifndef __SERVER__HPP__
#define __SERVER__HPP__

#include <boost/asio.hpp>

#include <iostream>
#include <list>
#include <memory>
#include <thread>
#include <mutex>

#include <logger.hpp>
using namespace logger;

using namespace std::chrono_literals;
using boost::asio::ip::tcp;

class Session;

using TAddress = uint32_t;
using TReceiveHandler = std::function<void(Session *responseSession, std::string message)>;

/// \class Server consisting of sessions with connection
/// \

class Server
{
  Server() = delete;               //
  Server(Server const &) = delete; //
public:
  Server(const std::string &address, unsigned short port);

  ~Server();

  void runServer();

  void sendToAll(std::string const &message);

  void sendByAddress(TAddress address, std::string const &message);

  void sendToAllAccepter(std::string const &message);

  void addReceiveHandler(TReceiveHandler lamda);

  void removeSession(std::shared_ptr<Session> session);

  void connect(const std::string &targetAddress, short unsigned targetPort);

  boost::asio::io_context & getContext() { return m_io_context; }

private:
  void accept();

private:
  boost::asio::io_context m_io_context{};
  std::string m_address;
  unsigned short m_port = 0;

  tcp::acceptor m_acceptor;

  std::mutex m_sessionMutex;
  std::list<std::shared_ptr<Session>> m_sessions_conn;
  std::list<std::shared_ptr<Session>> m_sessions_accept;

  std::list<tcp::socket> m_sockets;

  std::thread m_thread;

  TReceiveHandler m_receiveHandler = nullptr;
};

#endif // __SERVER__HPP__
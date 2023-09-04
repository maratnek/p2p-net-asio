#ifndef __SERVER__HPP__
#define __SERVER__HPP__

#include <boost/asio.hpp>

#include <iostream>
#include <list>
#include <memory>
#include <thread>

using namespace std::chrono_literals;
using boost::asio::ip::tcp;

/// \class Session
class Session : public std::enable_shared_from_this<Session> {
public:
  Session(tcp::socket socket)
      : m_socket(std::move(socket)), m_receiveBuffer(1024) {}

  Session() = delete;
  Session(Session const &) = delete;
  Session(Session &&session) = delete;

  ~Session() {}

  void start() { this->receiving(); }

  void sendMessage(std::string mes) {
    auto self(shared_from_this());
    // std::cout << "------------send message------------" << std::endl;
    if (m_socket.is_open()) {
      // std::cout << "Socket is open" << std::endl;
    } else {
      // std::cout << "Socket closed" << std::endl;
      return;
    }
    boost::asio::async_write(m_socket, boost::asio::buffer(mes + '\n'),
                             [this, self, mes](boost::system::error_code ec,
                                               std::size_t bytesTransferred) {
                               if (!ec) {
                                 //  std::cout << "Message sended " << mes << "
                                 //  Bytes: " << bytesTransferred << std::endl;
                                 //  this->receiving();
                               } else {
                                 std::cout << "Error sending message: "
                                           << ec.message() << std::endl;
                               }
                             });
  }

  void addReceiveHandler(std::function<void(std::string message)> lamda) {
    m_receiveHandler = lamda;
  }

private:
  void receiving() {
    auto self(shared_from_this());
    if (!m_socket.is_open())
      return;
    // std::cout << "------------receiving------------" << std::endl;
    // boost::asio::async_read_until(m_socket, m_receiveBuffer, '\n',
    //                               [this, self](boost::system::error_code ec,
    //                                            std::size_t bytesTransferred) {
    //                                 //   std::cout << "------------lamda
    //                                 //   read------------" << std::endl;
    //                                 //   std::cout << "Bytes transf to read
    //                                 //   buffer: " << bytesTransferred <<
    //                                 //   std::endl;
    //                                 if (!ec) {
    //                                   std::istream is(&m_receiveBuffer);
    //                                   std::string receivedMessage;
    //                                   std::getline(is, receivedMessage);
    //                                   std::cout
    //                                       << "Node: "
    //                                       << " Received: " << receivedMessage
    //                                       << std::endl;
    //                                   if (m_receiveHandler != nullptr) {
    //                                     m_receiveHandler(receivedMessage);
    //                                   }
    //                                 } else {
    //                                   //   std::cout << "ERROR Node Er Val:" <<
    //                                   //   ec.value() << ":" << ec.message() <<
    //                                   //   std::endl;
    //                                 }
    //                                 receiving(); // Continue receiving
    //                               });

    // boost::asio::async_read(m_socket, m_receiveBuffer, boost::asio::transfer_at_least(1024),
    boost::asio::async_read(m_socket, m_receiveBuffer, boost::asio::transfer_all(),
                                  [this, self](boost::system::error_code ec, std::size_t bytesTransferred)
                                  {
                                      std::cout << "------------lamda read------------" << std::endl;
                                      std::cout << "Bytest transf to readbuffer: " << bytesTransferred << std::endl;
                                      if (!ec)
                                      {
                                          if (bytesTransferred > 0)
                                          {
                                              std::string receivedMessage;
                                              std::istream is(&m_receiveBuffer);
                                              std::getline(is, receivedMessage);
                                              m_receiveBuffer.consume(bytesTransferred);

                                              if (m_receiveHandler != nullptr)
                                              {
                                                m_receiveHandler(receivedMessage);
                                              }
                                              std::cout << "Node: "
                                                        << "N Received: " << receivedMessage << std::endl;
                                          }
                                          else
                                          {
                                            std::cout << "Node: N Received O bytes " << std::endl;
                                          }
                                      }
                                      else
                                      {
                                          std::cout << "ERROR Node Er Val:"
                                          << ec.value() << ":" <<
                                          ec.message() << std::endl;
                                      }
                                      receiving();
                                  });
  }

private:
  tcp::socket m_socket;
  boost::asio::streambuf m_receiveBuffer;

  boost::array<char, 128> buf;

  std::function<void(std::string message)> m_receiveHandler = nullptr;
};

/// \class Server consisting of sessions with connection
/// \

class Server {
    Server() = delete; //
    Server(Server const &) = delete; //
public:
//   Server(boost::asio::io_context &ioContext, const std::string &address, unsigned short port)
  Server(const std::string &address, unsigned short port)
      : m_address(address)
      , m_port(port)
      , m_acceptor(m_io_context, tcp::endpoint(tcp::v4(), port))
  {
      m_acceptor.listen(m_port);
      accept();
  }

  ~Server() {
    if (m_thread.joinable()) {
      m_thread.join();
    }
  }

  void connect(const std::string &targetAddress, short unsigned targetPort) {
    std::cout << "------------do connect------------" << std::endl;

    auto &socket = m_sockets.emplace_back(m_acceptor.get_executor());

    boost::asio::ip::tcp::endpoint endpoint(
        // boost::asio::ip::address::from_string(targetAddress), targetPort);
        {}, targetPort);

    socket.async_connect(endpoint, [this](boost::system::error_code ec) {
      if (!ec) {
        std::cout << "Connected to server " << std::endl;
        auto session = std::make_shared<Session>(std::move(m_sockets.back()));
        session->start();
        m_sessions.emplace_back(session);
      } else {
        std::cout << "Failed to connect to "
                  << ": " << ec.message() << std::endl;
      }
    });
  }

  void runServer() {
    std::cout << "Running... " << std::endl;
    // m_thread = 
    auto t = std::thread([this]() {
      for (;;) {
        try {
            std::cout << "Server run " << std::endl;
          m_io_context.run();
          break; // run() exited normally
        } catch (std::exception &e) {
          // Deal with exception as appropriate.
          std::cerr << e.what() << std::endl;
        }
      }
    });
    std::swap(m_thread, t);
  }

  void sendToAll(std::string const &message) {
    for (auto &session : m_sessions) {
      session->sendMessage(message);
    }
  }

  // add received handler
  void addReceiveHandler(std::function<void(std::string message)> lamda) {
    for (auto &session : m_sessions) {
      session->addReceiveHandler(lamda);
    }
  }

private:
  void accept() {
    m_acceptor.async_accept(
        [this](boost::system::error_code ec, tcp::socket socket) {
          if (!ec) {
            auto endpoint = socket.remote_endpoint();
            std::cout << "Accepted connection from: " << endpoint.address()
                      << ":" << endpoint.port()
                      << " list s: " << m_sessions.size() << std::endl;

            auto session = std::make_shared<Session>(std::move(socket));
            session->start();
            m_sessions.emplace_back(session);
          }

          accept();
        });
  }

private:
  boost::asio::io_context m_io_context{};
  std::string m_address;
  unsigned short m_port = 0;

  tcp::acceptor m_acceptor;
  std::list<std::shared_ptr<Session>> m_sessions;

  std::list<tcp::socket> m_sockets;

  std::thread m_thread;
};

#endif // __SERVER__HPP__
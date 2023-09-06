#ifndef __SERVER__HPP__
#define __SERVER__HPP__

#include <boost/asio.hpp>
#include <boost/array.hpp>

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

  ~Session() {
    m_socket.cancel();
  }

      std::string getRemoteEndpoint() const {
        try {
            if (m_socket.is_open()) {
                boost::asio::ip::tcp::endpoint remoteEndpoint = m_socket.remote_endpoint();
                return remoteEndpoint.address().to_string() + ":" + std::to_string(remoteEndpoint.port());
            } else {
                return "Socket is not open";
            }
        } catch (const std::exception& e) {
            return "Error retrieving remote endpoint: " + std::string(e.what());
        }
    }

  void start() { this->receiving(); }

  void sendMessage(std::string mes) {
    auto self(shared_from_this());
    if (m_socket.is_open()) {
      std::cout << "Socket is open" << std::endl;
      std::cout << "Sending: remote endpoint " << m_socket.remote_endpoint() << std::endl;
    } else {
      std::cout << "Socket closed" << std::endl;
      return;
    }
    boost::asio::async_write(m_socket, boost::asio::buffer(mes + '\n'),
                             [this, self, mes](boost::system::error_code ec,
                                               std::size_t bytesTransferred) {
                               if (!ec) {
                                  std::cout << "Message sended " << mes << " Bytes: " << bytesTransferred << std::endl;
                                  // this->receiving();
                               } else {
                                 std::cout << "Error sending message: "
                                           << ec.message() << std::endl;
                               }
                             });
  }

  void addReceiveHandler(std::function<void(std::string message)> lamda) {
    if (lamda != nullptr) {
      std::cout << "Receive handler is ready" << std::endl;
      m_receiveHandler = lamda;
    }
  }

private:
  void receiving() {
    auto self(shared_from_this());
    std::cout << "------------receiving------------" << std::endl;
    if (!m_socket.is_open()) {
      std::cout << "Socket is not open" << std::endl;
    } else {
      std::cout << "Socket is open" << std::endl;
    }
    boost::asio::async_read_until(m_socket, m_receiveBuffer, '\n',
                                  [this, self](boost::system::error_code ec,
                                               std::size_t bytesTransferred)
                                  {
                                    std::cout << "------------lamda read------------" << std::endl;
                                    std::cout << "Socket available: " << m_socket.available() << std::endl;
                                    std::cout << "Bytes transf to read buffer: " << bytesTransferred << std::endl;
                                    if (!ec)
                                    {
                                      std::istream is(&m_receiveBuffer);
                                      std::string receivedMessage;
                                      std::getline(is, receivedMessage);
                                      std::cout
                                          << "Node: "
                                          << " Received: " << receivedMessage
                                          << std::endl;
                                      if (m_receiveHandler != nullptr)
                                      {
                                        m_receiveHandler(receivedMessage);
                                      }
                                    }
                                    else
                                    {
                                      std::cout << "ERROR Node Er Val:" << ec.value() << ":" << ec.message() << std::endl;
                                    }
                                    receiving(); // Continue receiving
                                  });
  }

private:
  tcp::socket m_socket;
  boost::asio::streambuf m_receiveBuffer;

  enum { max_length = 128 };
  char data_[max_length];

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
    std::cout << "Server destructed" << std::endl;
    if (m_thread.joinable()) {
      std::cout << "Join thread" << std::endl;
      // m_io_context.stop();

      m_thread.join(); 
    }
  }

  void runServer() {
    std::cout << "Running... " << std::endl;
    auto t = std::thread([this]() {
      for (;;) {
        try {
          std::cout << "Server run " << m_port << std::endl;
          m_io_context.run();
          std::cout << "Run exited" << std::endl;
          break; // run() exited normally
        } catch (std::exception &e) {
          // Deal with exception as appropriate.
          std::cerr << "Failed run context: " << e.what() << std::endl;
        }
      }
    });
    std::swap(m_thread, t);
  }

  void sendToAll(std::string const &message) {
    std::cout << "Conn Sessions size before send messages: " << m_sessions_conn.size() << std::endl;
    for (auto &session : m_sessions_conn) {
      session->sendMessage(message);
    }
  }

  void sendToAllAccepter(std::string const &message) {
    std::cout << "Accepter Sessions size before send messages: " << m_sessions_accept.size() << std::endl;
    for (auto &session : m_sessions_accept) {
      session->sendMessage(message);
    }
  }

  // add received handler
  void addReceiveHandler(std::function<void(std::string message)> lamda) {
    if (lamda != nullptr) {
      std::cout << "Add receive handler" << std::endl;
      m_receiveHandler = lamda;
    }
  }

  void connect(const std::string &targetAddress, short unsigned targetPort) {
    std::cout << "------------do connect------------" << std::endl;
    auto &socket = m_sockets.emplace_back(m_acceptor.get_executor());

    boost::asio::ip::tcp::endpoint endpoint(
        boost::asio::ip::address::from_string(targetAddress), targetPort);
        // {}, targetPort);

    socket.async_connect(endpoint, [this, targetPort, &socket](boost::system::error_code ec) {
      if (!ec) {
          auto session = std::make_shared<Session>(std::move(socket));
          session->addReceiveHandler(m_receiveHandler);
          session->start();
          m_sessions_conn.emplace_back(session);
          std::cout << "Success Connected to server (port: " << targetPort << ")"
                    << " list conn s: " << m_sessions_conn.size() << std::endl;
      } else {
        std::cout << "Failed to connect to (port: " << targetPort << ")"
                  << ": " << ec.message() << std::endl;
      }
    });
  }

private:
  void accept() {
    m_acceptor.async_accept(
        [this](boost::system::error_code ec, tcp::socket socket) {
            if (!ec) {
                auto endpoint = socket.remote_endpoint();
                auto lendp = socket.local_endpoint();

                auto session = std::make_shared<Session>(std::move(socket));

                session->addReceiveHandler(m_receiveHandler);
                m_sessions_accept.emplace_back(session);
                session->start(); // Start the session here

                std::cout << "Accepted connection from remote endp: " << endpoint.address()
                          << ":" << endpoint.port() << " local endpoint: " << lendp.port()
                          << " list s: " << m_sessions_accept.size() << std::endl;

                // Log remote endpoint information again to double-check
                std::cout << "Remote endpoint after accepting: " << session->getRemoteEndpoint() << std::endl;
            } else {
                std::cout << "Error accepting connection: " << ec.message() << std::endl;
            }

            accept(); // Continue accepting connections
        });
}

private:
  boost::asio::io_context m_io_context{};
  std::string m_address;
  unsigned short m_port = 0;

  tcp::acceptor m_acceptor;
  std::list<std::shared_ptr<Session>> m_sessions_conn;
  std::list<std::shared_ptr<Session>> m_sessions_accept;

  std::list<tcp::socket> m_sockets;

  std::thread m_thread;

  std::function<void(std::string message)> m_receiveHandler = nullptr;
};

#endif // __SERVER__HPP__
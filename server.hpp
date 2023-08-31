#ifndef __SERVER__HPP__
#define __SERVER__HPP__

#include <boost/asio.hpp>

#include <memory>
#include <thread>
#include <iostream>
#include <list>


using namespace std::chrono_literals;
using boost::asio::ip::tcp;

/// \class Session
class Session
{
public:

  Session(tcp::socket socket)
    : m_socket(std::move(socket))
  {}

  Session() = delete;
  Session(Session const&) = delete;
  Session(Session &&session) noexcept
    : m_socket(std::move(session.m_socket))
  {
  }

  ~Session() {}

  void start()
  {
    this->receiving();
  }

    void sendMessage(std::string const& mes)
    {
    std::cout << "------------send message------------" << std::endl;
    boost::asio::async_write(m_socket, boost::asio::buffer(mes),
                             [this, mes](boost::system::error_code ec, std::size_t bytesTransferred)
                             {
                                 if (!ec)
                                 {
                                     std::cout << "Message sended " << mes << " Bytes: " << bytesTransferred << std::endl;
                                    //  do_read(_socket);
                                 }
                                 else
                                 {
                                     std::cout << "Error sending message: " << ec.message() << std::endl;
                                 }
                             });
    }

private:
  void receiving()
  {
        std::cout << "------------receiving------------" << std::endl;
        boost::asio::async_read_until(m_socket, m_receiveBuffer, '\n',
                                      [this](boost::system::error_code ec, std::size_t bytesTransferred)
                                      {
                                          std::cout << "------------lamda read------------" << std::endl;
                                          std::cout << "Bytest transf to read buffer: " << bytesTransferred << std::endl;
                                          if (!ec)
                                          {
                                              std::istream is(&m_receiveBuffer);
                                              std::string receivedMessage;
                                              std::getline(is, receivedMessage);
                                              std::cout << "Node: " << " Received: " << receivedMessage << std::endl;
                                              this->receiving(); // Continue receiving
                                          }
                                          else
                                          {
                                              std::cout << "ERROR Node Er Val:" << ec.value() << ":" << ec.message() << std::endl;
                                          }
                                      });
    }

private:

  tcp::socket m_socket;
  boost::asio::streambuf m_receiveBuffer;
};



/// \class Server consisting of sessions with connection
/// \

class Server
{
public:
    Server(boost::asio::io_context &ioContext, const std::string &address, unsigned short port)
        : m_address(address)
        , m_port(port)
        // , m_io_context(boost::asio::io_context{})
        , m_acceptor(ioContext, tcp::endpoint(tcp::v4(), port))
    {

        m_acceptor.listen(m_port);
        this->doAccept();

        std::cout << "Running... " << std::endl;
        // m_thread = std::move(std::thread([this]()
                //  { m_io_context.run(); }));
    }

    ~Server()
    {
        // if (m_thread.joinable()) {
            // m_thread.join();
        // }
    }

    void connect(const std::string &targetAddress, short unsigned targetPort)
    {
        std::cout << "------------do connect------------" << std::endl; 

        auto socket = tcp::socket(m_acceptor.get_executor());
        auto& session = m_sessions.emplace_back(std::move(Session{std::move(socket)}));
        boost::asio::ip::tcp::endpoint endpoint(
           boost::asio::ip::address::from_string(targetAddress), targetPort);
        socket.async_connect(endpoint, [this, &session](boost::system::error_code ec)
                           {
                                if (!ec) {
                                    std::cout << "Connected to server " <<  std::endl;
                                    session.start();
                                    // std::string data = "Hello from node port is: " + std::to_string(m_port);
                                    // do_write(socket, data);
                                }
                                else
                                {
                                    std::cout << "Failed to connect to " <<  ": " << ec.message() << std::endl;
                                } 
                            });

    }

    void sendToAll(std::string const& message) {
        for (auto & session : m_sessions) {
            session.sendMessage(message);
        }
    }

private:
  void doAccept()
  {
    m_acceptor.async_accept(
        [this](boost::system::error_code ec, tcp::socket socket)
        {
          if (!ec)
          {
                    auto endpoint = socket.remote_endpoint();
                    std::cout << "Accepted connection from: " << endpoint.address()  << ":" << endpoint.port()
                    << " list s: " << m_sessions.size() << std::endl;
              Session session{std::move(socket)};
              session.start();
              m_sessions.emplace_back(std::move(session));
          }

          doAccept();
        });
  }

private:
    std::string m_address;
    unsigned short m_port = 0;

  tcp::acceptor m_acceptor;
  std::list<Session> m_sessions;

//   boost::asio::io_context m_io_context;
//   std::thread m_thread;
};


#endif // __SERVER__HPP__
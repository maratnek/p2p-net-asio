#ifndef __P2P_NODES_HPP__
#define __P2P_NODES_HPP__

#include <boost/asio.hpp>

#include <iostream>
#include <list>
#include <vector>

using namespace std::chrono_literals;

using boost::asio::ip::tcp;

#include <mutex>
constexpr size_t m_buffer_size = 512;
class Session
{
};

class Server
{
public:
    Server(boost::asio::io_context &ioContext, const std::string &address, unsigned short port)
        : m_address(address), m_port(port), m_acceptor(ioContext, tcp::endpoint(tcp::v4(), port)), m_listeningSocket(ioContext)
        , m_receiveBuffer(m_buffer_size)
        // , _receiveBuffer(std::make_shared<boost::asio::streambuf>(m_buffer_size))
    {
        m_buffer.reserve(1024);
        m_acceptor.listen(m_port);
        this->DoAccept();
    }
    ~Server() {
        std::cout << "Destruct Server " << m_port << std::endl;
    }

    void do_write(tcp::socket &_socket, std::string const &mes)
    {
        std::cout << "------------do write------------" << std::endl;
        boost::asio::async_write(_socket, boost::asio::buffer(mes),
                                 [this, mes, &_socket](boost::system::error_code ec, std::size_t bytesTransferred)
                                 {
                                     if (!ec)
                                     {
                                         std::cout << "Transf bytest: " << bytesTransferred << " Message sended " << mes << std::endl;
                                         do_read(_socket);
                                     }
                                     else
                                     {
                                         std::cout << "Error sending message: " << ec.message() << std::endl;
                                     }
                                 });
    }

    void do_write_all(std::string mes)
    {
        for (tcp::socket &s : m_sockets)
        {
            this->do_write(s, mes);
        }
    }

    void receive(std::function<void(std::string)> callback) {
        m_receiveCallback = callback;
    }

    void do_read(tcp::socket &_socket)
    {
        std::cout << "------------do read------------" << std::endl;
        // boost::asio::async_read_until(_socket, m_receiveBuffer, '\n',
        //                               [this, &_socket](boost::system::error_code ec, std::size_t bytesTransferred)
        //                               {
        //                                   std::cout << "------------lamda read------------" << std::endl;
        //                                   std::cout << "Bytest transf to read buffer: " << bytesTransferred << std::endl;
        //                                   if (!ec)
        //                                   {
        //                                       if (bytesTransferred > 0)
        //                                       {
        //                                           std::string receivedMessage;
        //                                           std::istream is(&m_receiveBuffer);
        //                                           std::getline(is, receivedMessage);

        //                                     m_receiveCallback(receivedMessage);
        //                                           std::cout << "Node: "
        //                                                     << "N Received: " << receivedMessage << std::endl;
        //                                       }
        //                                       else
        //                                       {
        //                                           std::cout << "Node: "
        //                                                     << "N Received O bytes " << std::endl;
        //                                       }
        //                                   }
        //                                   else
        //                                   {
        //                                       std::cout << "ERROR Node Er Val:" << ec.value() << ":" << ec.message() << std::endl;
        //                                   }
        //                                   do_read(_socket); // Continue receiving
        //                               });

        boost::asio::async_read(_socket, m_receiveBuffer, boost::asio::transfer_at_least(1),
                                      [this, &_socket](boost::system::error_code ec, std::size_t bytesTransferred) mutable
                                      {
                                          std::cout << "------------lamda read------------" << std::endl;
                                          std::cout << "Bytest transf to read buffer: " << bytesTransferred << std::endl;
                                          if (!ec)
                                          {
                                              if (bytesTransferred > 0)
                                              {
                                                  std::string receivedMessage;
                                                  std::unique_lock lock(m_mutex);
                                                  std::istream is(&m_receiveBuffer);
                                                  std::getline(is, receivedMessage);
                                                  lock.unlock();
                                                  m_receiveBuffer.consume(bytesTransferred);

                                                  m_receiveCallback(receivedMessage);
                                                  std::cout << "Node: "
                                                            << "N Received: " << receivedMessage << std::endl;
                                              }
                                              else
                                              {
                                                  std::cout << "Node: "
                                                            << "N Received O bytes " << std::endl;
                                              }
                                              do_read(_socket); // Continue receiving
                                          }
                                          else
                                          {
                                              std::cout << "ERROR Node Er Val:" << ec.value() << ":" << ec.message() << std::endl;
                                          }
                                      });

        // _socket.async_read_some(boost::asio::buffer(m_buffer),
        //                         [this, &_socket](const boost::system::error_code &ec, std::size_t bytesTransferred)
        //                         {
        //                             // std::cout << "------------lamda read------------" << std::endl;
        //                             // std::cout << "Bytest transf to read buffer: " << bytesTransferred << std::endl;
        //                             if (!ec)
        //                             {
        //                                 if (bytesTransferred > 0)
        //                                 {
        //                                     std::string receivedMessage(m_buffer.begin(), m_buffer.begin() + bytesTransferred);

        //                                     std::cout << "Node: "
        //                                               << "N Received: " << receivedMessage << std::endl;
        //                                     m_receiveCallback(receivedMessage);
        //                                     do_read(_socket); // Continue receiving
        //                                 }
        //                                 else
        //                                 {
        //                                     // std::cout << "Node: "
        //                                             //   << "N Received O bytes " << std::endl;
        //                                 }
        //                             }
        //                             else
        //                             {
        //                                 std::cout << "ERROR Node Er Val:" << ec.value() << ":" << ec.message() << std::endl;
        //                             }
        //                         });

    }

    void Connect(boost::asio::io_context &ioContext, const std::string &targetAddress, short unsigned targetPort)
    {
        std::cout << "------------do connect------------" << std::endl;

        auto &conn = m_sockets.emplace_back(m_acceptor.get_executor());
        // auto &conn = m_sockets.emplace_back(boost::asio::ip::tcp::socket(ioContext));
        // conn.open(m_acceptor.local_endpoint().protocol());
        // conn.set_option(tcp::socket::reuse_address(true));
        // conn.bind(m_acceptor.local_endpoint());
        std::cout << "executor: " << conn.get_executor() << std::endl;

        boost::asio::ip::tcp::endpoint endpoint(
            // boost::asio::ip::address::from_string(targetAddress), targetPort);
            {}, targetPort);
        conn.async_connect(endpoint, [=, this, &conn](boost::system::error_code ec)
                           {
                                if (!ec) {
                                    std::cout << "Connected to server " << conn.remote_endpoint() << std::endl;

                                    std::string data = "Hello from node port is: " + std::to_string(m_port);
                                    // do_write(conn, data);
                                    do_read(conn);
                                }
                                else
                                {
                                    std::cout << "Failed to connect to " << endpoint << ": " << ec.message() << std::endl;
                                } });

        // boost::asio::io_service io_service;
        // boost::asio::ip::tcp::resolver resolver(io_service);
        // boost::asio::ip::tcp::resolver::results_type endpoints =
        //     resolver.resolve(targetAddress, std::to_string(targetPort));

        // boost::asio::async_connect(socket, endpoints,
        //                            [this, &socket](boost::system::error_code ec, const boost::asio::ip::tcp::endpoint &endpoint)
        //                            {
        //                                 if (!ec) {
        //                                     std::cout << "Connected to server " << socket.remote_endpoint() << std::endl;

        //                                     std::string data = "Hello from node port is: " + std::to_string(m_port);
        //                                     do_write(socket, data);
        //                                 }
        //                                 else
        //                                 {
        //                                     std::cout << "Failed to connect to " << endpoint << ": " << ec.message() << std::endl;
        //                                 } });
    }

private:
    void DoAccept()
    {
        m_acceptor.async_accept(
            [this](boost::system::error_code errc, tcp::socket socket)
            {
                if (!errc)
                {
                    auto endpoint = socket.remote_endpoint();
                    std::cout << "Accepted connection from: " << endpoint.address() << ":" << endpoint.port()
                              << " list s: " << m_sockets.size() << std::endl;
                    m_sockets.emplace_back(std::move(socket));
                    do_read(m_sockets.back());
                }
                else
                {
                    std::cerr << "ERROR: " << errc.message() << std::endl;
                }
                this->DoAccept();
            });
    }

private:
    tcp::acceptor m_acceptor;
    std::string m_address;
    unsigned short m_port = 0;

    boost::asio::streambuf m_receiveBuffer;

    std::list<tcp::socket> m_sockets;

    tcp::socket m_listeningSocket;

    std::vector<char> m_buffer;

    std::function<void(std::string)> m_receiveCallback;

    std::mutex m_mutex;
};

/*
class Server {
public:
    Server(boost::asio::io_context& ioContext, const std::string& address, unsigned short port)
        : m_address(address)
        , m_port(port)
        , m_acceptor(io_context, tcp::endpoint(boost::asio::ip::make_address(address), port))
    {
        m_acceptor.listen(m_port);
    }

    }
private:

    void DoAccept() {
    m_accceptor.async_accept(
        [this](boost::system::error_code errc, tcp::socket socket)
        {
            if (!errc) {

            }
            this->DoAccept();
        });
    }

private:
    tcp::acceptor m_acceptor;
    std::string m_address;
    unsigned short m_port = 0;
};
*/

#endif // __P2P_NODES_HPP__
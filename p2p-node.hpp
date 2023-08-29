#ifndef __P2P_NODES_HPP__
#define __P2P_NODES_HPP__

#include <boost/asio.hpp>

#include <iostream>
#include <list>

using namespace std::chrono_literals;

using boost::asio::ip::tcp;

class Session
{
};

class Server
{
public:
    Server(boost::asio::io_context &ioContext, const std::string &address, unsigned short port)
        : m_address(address)
        , m_port(port)
        , m_acceptor(ioContext, tcp::endpoint(tcp::v4(), port))
        , m_listeningSocket(ioContext)
    {
        m_acceptor.listen(m_port);
        this->DoAccept();
    }

    // void SendMessage(tcp::socket const& _socket, const std::string& message) {
    //     boost::asio::async_write(_socket, boost::asio::buffer(message),
    //                              [this, message, &_socket](boost::system::error_code ec, std::size_t  bytesTransferred) {
    //                                  if (!ec) {
    //                                      std::cout << "Message sended " << message << std::endl;
    //                                      StartReceiving(_socket);
    //                                  } else {
    //                                     std::cout << "Error sending message: " << ec.message() << std::endl;
    //                                  }
    //                              });
    // }

    // void StartReceiving(tcp::socket const& _socket) {
    //     boost::asio::async_read_until(_socket, m_receiveBuffer, '\n',
    //                                   [this, &_socket](boost::system::error_code ec, std::size_t bytesTransferred) {
    //                                       if (!ec) {
    //                                           std::istream is(&m_receiveBuffer);
    //                                           std::string receivedMessage;
    //                                           std::getline(is, receivedMessage);
    //                                           std::cout << "Node: " << " Received: " << receivedMessage << std::endl;
    //                                           StartReceiving(_socket); // Continue receiving
    //                                       } else {
    //                                         std::cout << "ERROR Node Er Val:" << ec.value() << ":" << ec.message() << std::endl;
    //                                       }
    //                                   });
    // }
    void do_write(tcp::socket& _socket, std::string mes) {
        std::cout << "------------do write------------" << std::endl; 
        boost::asio::async_write(_socket, boost::asio::buffer(mes + "\n"),
                                 [this, mes, &_socket](boost::system::error_code ec, std::size_t  bytesTransferred) {
                                     if (!ec) {
                                         std::cout << "Message sended " << mes << std::endl;
                                         do_read(_socket);
                                     } else {
                                        std::cout << "Error sending message: " << ec.message() << std::endl;
                                     }
                                 });
    }
    void do_read(tcp::socket& _socket) {
        std::cout << "------------do read------------" << std::endl;
        boost::asio::async_read_until(_socket, m_receiveBuffer, '\n',
                                      [this, &_socket](boost::system::error_code ec, std::size_t bytesTransferred)
                                      {
                                          std::cout << "------------lamda read------------" << std::endl;
                                          if (!ec)
                                          {
                                              std::istream is(&m_receiveBuffer);
                                              std::string receivedMessage;
                                              std::getline(is, receivedMessage);
                                              std::cout << "Node: " << " Received: " << receivedMessage << std::endl;
                                              do_read(_socket); // Continue receiving
                                          }
                                          else
                                          {
                                              std::cout << "ERROR Node Er Val:" << ec.value() << ":" << ec.message() << std::endl;
                                          }
                                      });
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
                                    do_write(conn, data);
                                }
                                else
                                {
                                    std::cout << "Failed to connect to " << endpoint << ": " << ec.message() << std::endl;
                                } 
                            });

        return;

        //     boost::asio::io_service io_service;
        //     boost::asio::ip::tcp::resolver resolver(ioContext);
        //     try
        //     {
        //         boost::asio::ip::tcp::resolver::results_type endpoints =
        //             resolver.resolve(targetAddress, std::to_string(targetPort));

        //         boost::asio::async_connect(socketConnection, endpoints,
        //                                    [this, &socket](boost::system::error_code ec, const boost::asio::ip::tcp::endpoint &endpoint)
        //                                    {
        //                                        if (!ec)
        //                                        {
        //                                            std::cout << "Connected to target node." << endpoint << std::endl;
        //                                            //    m_sockets.emplace_back(std::move(socket));
        //                                            do_write(m_listeningSocket, "Hello from node ");
        //                                        }
        //                                        else
        //                                        {
        //                                            std::cout << "ERROR: " << ec.message() << std::endl;
        //                                        }
        //                                    });

        // }
        // catch (std::exception const &ex)
        // {
        //     std::cerr << "Exception: " << ex.what() << std::endl;
        // }
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
                    std::cout << "Accepted connection from: " << endpoint.address()  << ":" << endpoint.port()
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
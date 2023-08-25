#ifndef __SESSION__HPP__
#define __SESSION__HPP__

#include "server.hpp"

#include <boost/asio.hpp>

#include <iostream>
#include <list>
#include <memory>
#include <thread>

using namespace std::chrono_literals;
using boost::asio::ip::tcp;

/// \class Session
class Session : public std::enable_shared_from_this<Session>
{
public:
    Session(tcp::socket socket, Server &server)
        : m_socket(std::move(socket)), m_receiveBuffer(1024), m_server(server)
    {
    }

    Session() = delete;
    Session(Session const &) = delete;
    Session(Session &&session) = delete;

    ~Session()
    {
        std::cout << "~Session() deleted" << std::endl;
        if (m_socket.is_open())
        {
            m_socket.close();
        }
    }

    std::string getRemoteEndpoint() const;

    void start() { this->receiving(); }

    void sendMessage(std::string mes)
    {
        auto self(shared_from_this());
        if (m_socket.is_open())
        {
            std::cout << "Socket is open" << std::endl;
            std::cout << "Sending: remote endpoint " << m_socket.remote_endpoint() << std::endl;
        }
        else
        {
            std::cerr << "Socket closed" << std::endl;
            return;
        }
        boost::asio::async_write(m_socket, boost::asio::buffer(mes + '\n'),
                                 [this, self, mes](boost::system::error_code ec,
                                                   std::size_t bytesTransferred)
                                 {
                                     if (!ec)
                                     {
                                         std::cout << "Message sended " << mes << " Bytes: " << bytesTransferred << std::endl;
                                     }
                                     else
                                     {
                                         std::cout << "Error sending message: "
                                                   << ec.message() << std::endl;
                                     }
                                 });
    }

    void addReceiveHandler(std::function<void(std::string message)> lamda)
    {
        if (lamda != nullptr)
        {
            std::cout << "Receive handler is ready" << std::endl;
            m_receiveHandler = lamda;
        }
    }

private:
    void handle_read(boost::system::error_code ec, std::size_t bytesTransferred);

    void receiving();

private:
    Server &m_server;
    tcp::socket m_socket;
    boost::asio::streambuf m_receiveBuffer;

    std::function<void(std::string message)> m_receiveHandler = nullptr;
};

#endif // __SESSION__HPP__
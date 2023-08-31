#pragma once

#include <iostream>
#include <list>
#include <boost/asio.hpp>

using namespace std::chrono_literals;

using boost::asio::ip::tcp;

class P2PNode {
public:
    P2PNode(boost::asio::io_context& ioContext, const std::string& address, unsigned short port)
        : acceptor_(ioContext, boost::asio::ip::tcp::endpoint(boost::asio::ip::make_address(address), port))
          ,socket_(ioContext)
          ,address_(address)
          ,port_(port) 
    {
        std::cout << "P2PNode: " << address_ << ":" << port_ << std::endl;

        acceptor_.listen();
        DoAccept();
    }

    void ConnectToNode(const std::string &targetAddress, unsigned short targetPort)
    {
        boost::asio::io_service io_service;
        boost::asio::ip::tcp::resolver resolver(io_service);
        boost::asio::ip::tcp::resolver::results_type endpoints =
            resolver.resolve(targetAddress, std::to_string(targetPort));

        auto &_socket = m_sockets.emplace_back(acceptor_.get_executor());

        boost::asio::async_connect(_socket, endpoints,
                                   [this](boost::system::error_code ec, const boost::asio::ip::tcp::endpoint & endpoint )
                                   {
        if (!ec) {
            std::cout << "Connected to target node." << endpoint << std::endl;
            SendMessage(m_sockets.back(), "Ping " + address_ + ":" + std::to_string(port_) + "\n");
        } else {
            std::cout << "ERROR: " << ec.message() << std::endl;
        } });
    }

    void SendMessage(tcp::socket& _socket, const std::string& message) {
        boost::asio::async_write(_socket, boost::asio::buffer(message),
                                 [this, message, &_socket](boost::system::error_code ec, std::size_t  bytesTransferred) {
                                     if (!ec) {
                                         std::cout << "Message sended " << message << std::endl;
                                         StartReceiving(_socket);
                                     } else {
                                        std::cout << "Error sending message: " << ec.message() << std::endl;
                                     }
                                 });
    }

    void SendMessage(const std::string& message) {
        std::cout << "Sending to sockets " << m_sockets.size() << std::endl;
        for (auto& _socket : m_sockets) {
            this->SendMessage(_socket, message);
        }
    }

    void StartReceiving(tcp::socket& _socket) {
        boost::asio::async_read_until(_socket, receiveBuffer_, '\n',
                                      [this, &_socket](boost::system::error_code ec, std::size_t bytesTransferred) {
                                          if (!ec) {
                                            std::cout << "Reading buffer size " << bytesTransferred << std::endl;
                                              std::istream is(&receiveBuffer_);
                                              std::string receivedMessage;
                                              std::getline(is, receivedMessage);
                                              std::cout << "Node: " << this->address_ << ":" << this->port_ << " Received: " << receivedMessage << std::endl;
                                              StartReceiving(_socket); // Continue receiving
                                          } else {
                                            std::cout << "ERROR Node: " << this->address_ << ":" << this->port_ << " " 
                                            << "Er Val:" << ec.value() << ":" << ec.message() << std::endl;
                                          }
                                      });
    }

private:
    void DoAccept() {
        acceptor_.async_accept(
                               [this](boost::system::error_code ec, tcp::socket socket) {
                                   if (!ec) {
                                       std::cout << "Accepted connection from: " << socket.remote_endpoint() << std::endl;
                                       m_sockets.emplace_back(std::move(socket));
                                       StartReceiving(m_sockets.back());
                                   } else {
                                       std::cout << "Error accepting connection: " << ec.message() << std::endl;
                                   }
                                   DoAccept(); // Continue accepting
                               });
    }


private:
    boost::asio::ip::tcp::acceptor acceptor_;
    boost::asio::ip::tcp::socket socket_;

    std::list<tcp::socket> m_sockets;
    boost::asio::streambuf receiveBuffer_;
    std::string address_;
    unsigned short port_;
};


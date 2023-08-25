#pragma once

#include <iostream>
#include <boost/asio.hpp>

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

        boost::asio::async_connect(socket_, endpoints,
                                   [this](boost::system::error_code ec, const boost::asio::ip::tcp::endpoint & endpoint )
                                   {
        if (!ec) {
            std::cout << "Connected to target node." << endpoint << std::endl;
            SendMessage("Ping " + address_ + ":" + std::to_string(port_) + "\n");
        } else {
            std::cout << "ERROR: " << ec.message() << std::endl;
        } });
    }

    void SendMessage(const std::string& message) {
        boost::asio::async_write(socket_, boost::asio::buffer(message),
                                 [this, message](boost::system::error_code ec, std::size_t  bytesTransferred) {
                                     if (!ec) {
                                         std::cout << "Message sended " << message << std::endl;
                                         StartReceiving();
                                     } else {
                                        std::cout << "Error sending message: " << ec.message() << std::endl;
                                     }
                                 });
    }

    void StartReceiving() {
        boost::asio::async_read_until(socket_, receiveBuffer_, '\n',
                                      [this](boost::system::error_code ec, std::size_t bytesTransferred) {
                                          if (!ec) {
                                              std::istream is(&receiveBuffer_);
                                              std::string receivedMessage;
                                              std::getline(is, receivedMessage);
                                              std::cout << "Node: " << this->address_ << ":" << this->port_ << " Received: " << receivedMessage << std::endl;
                                              StartReceiving(); // Continue receiving
                                          } else {
                                            std::cout << "ERROR Node: " << this->address_ << ":" << this->port_ << " " 
                                            << "Er Val:" << ec.value() << ":" << ec.message() << std::endl;
                                          }
                                      });
    }

private:
    void DoAccept() {
        acceptor_.async_accept(socket_,
                               [this](boost::system::error_code ec) {
                                   if (!ec) {
                                       std::cout << "Accepted connection from: " << socket_.remote_endpoint() << std::endl;
                                       StartReceiving();
                                   } else {
                                       std::cout << "Error accepting connection: " << ec.message() << std::endl;
                                   }
                                   //DoAccept(); // Continue accepting
                               });
    }


private:
    boost::asio::ip::tcp::acceptor acceptor_;
    boost::asio::ip::tcp::socket socket_;
    boost::asio::streambuf receiveBuffer_;
    std::string address_;
    unsigned short port_;
};


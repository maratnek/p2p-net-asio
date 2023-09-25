#ifndef __P2P_NODES_HPP__
#define __P2P_NODES_HPP__

#include <boost/asio.hpp>

#include <iostream>
#include <list>
#include <vector>

#include "chat-message.hpp"

#include <deque>
typedef std::deque<chat_message> chat_message_queue;

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
        : m_address(address), m_port(port), m_acceptor(ioContext, tcp::endpoint(tcp::v4(), port)), m_listeningSocket(ioContext), m_receiveBuffer(m_buffer_size), m_io_context(ioContext)
    // , _receiveBuffer(std::make_shared<boost::asio::streambuf>(m_buffer_size))
    {
        m_buffer.reserve(1024);
        m_acceptor.listen(m_port);
        this->DoAccept();
    }
    ~Server()
    {
        std::cout << "Destruct Server " << m_port << std::endl;
    }

    void write(tcp::socket &_socket, const chat_message &msg)
    {
        std::cout << "Write " << std::string{msg.data(), msg.length()} << std::endl;
        boost::asio::post(m_io_context,
                          [this, msg, &_socket]()
                          {
                              bool write_in_progress = !write_msgs_.empty();
                              write_msgs_.push_back(msg);
                              if (!write_in_progress)
                              {
                                  do_write(_socket);
                              }
                          });
    }

    void do_write(tcp::socket &_socket)
    {
        boost::asio::async_write(_socket,
                                 boost::asio::buffer(write_msgs_.front().data(),
                                                     write_msgs_.front().length()),
                                 [this, &_socket](boost::system::error_code ec, std::size_t length)
                                 {
                                     if (!ec)
                                     {
                                         std::cout << "Writed " 
                                         << std::string{write_msgs_.front().data(), write_msgs_.front().length()} << std::endl;
                                         write_msgs_.pop_front();
                                         if (!write_msgs_.empty())
                                         {
                                             do_write(_socket);
                                         }
                                     }
                                     else
                                     {
                                        //  socket_.close();
                                         std::cout << "Error sending message: " << ec.message() << std::endl;
                                     }
                                 });
    }

    void do_write(tcp::socket &_socket, std::string mes)
    {
        std::cout << "------------do write------------" << std::endl;
        // send size
        mes = std::to_string(mes.size()) + mes;
        boost::asio::async_write(_socket, boost::asio::buffer(mes),
                                 [this, mes, &_socket](boost::system::error_code ec, std::size_t bytesTransferred)
                                 {
                                     if (!ec)
                                     {
                                         std::cout << "Transfed bytest: " << bytesTransferred << " Message sended " << mes << std::endl;
                                         // do_read(_socket);
                                     }
                                     else
                                     {
                                         std::cout << "Error sending message: " << ec.message() << std::endl;
                                     }
                                 });
    }

    void do_write_msg(tcp::socket &_socket, std::string mes)
    {
        std::cout << "------------do write chat------------" << std::endl;
        chat_message msg;
        msg.body_length(mes.length());
        std::memcpy(msg.body(), mes.c_str(), msg.body_length());
        msg.encode_header();

        boost::asio::async_write(_socket, boost::asio::buffer(msg.data(), msg.length()),
                                 [this, &msg, &_socket](boost::system::error_code ec, std::size_t bytesTransferred)
                                 {
                                     if (!ec)
                                     {
                                         std::string mes{msg.body(), msg.body_length()};
                                         std::cout << "Transfed bytest: " << bytesTransferred << " Message sended " << mes << std::endl;
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
            // this->do_write(s, mes);
            // this->do_write_msg(s, mes);

            chat_message msg;
            msg.body_length(mes.length());
            std::memcpy(msg.body(), mes.c_str(), msg.body_length());
            msg.encode_header();
            this->write(s, msg);
        }
    }

    void receive(std::function<void(std::string)> callback)
    {
        m_receiveCallback = callback;
    }

    void do_read_header(tcp::socket &_socket)
    {
        std::cout << "------------do read header------------" << std::endl;
        // auto self(shared_from_this());
        boost::asio::async_read(_socket,
                                boost::asio::buffer(read_msg_.data(), chat_message::header_length),
                                [this, &_socket](boost::system::error_code ec, std::size_t length)
                                {
                                    std::cout << "Header length: " << length << std::endl;
                                    if (!ec && read_msg_.decode_header())
                                    {
                                        std::unique_lock lock(m_mutex);
                                        std::cout << "RM data: " << std::string(read_msg_.data(), chat_message::header_length) << std::endl;
                                        std::cout << "Body length before : " << read_msg_.body_length() << std::endl;
                                        lock.unlock();
                                        do_read_body(_socket);
                                    }
                                    else
                                    {
                                        std::cout << "ERROR Node Er Val:" << ec.value() << ":" << ec.message() << std::endl;
                                    }
                                });
    }

    void do_read_body(tcp::socket &_socket)
    {
        std::cout << "------------do read body------------" << std::endl;
        // auto self(shared_from_this());
        boost::asio::async_read(_socket,
                                boost::asio::buffer(read_msg_.body(), read_msg_.body_length()),
                                [this, &_socket](boost::system::error_code ec, std::size_t length)
                                {
                                    if (!ec)
                                    {
                                        std::unique_lock lock(m_mutex);
                                        std::cout << "Body Length: " << length << std::endl;
                                        lock.unlock();
                                        // if (length > 0)
                                        {
                                            std::string receivedMessage(read_msg_.body(), read_msg_.body_length());
                                            lock.lock();
                                            std::cout << "Received: " << receivedMessage << std::endl;
                                            lock.unlock();
                                            if (m_receiveCallback)
                                            {
                                                m_receiveCallback(receivedMessage);
                                            }
                                        }
                                        do_read_header(_socket);
                                    }
                                    else
                                    {
                                        std::cout << "ERROR Node Er Val:" << ec.value() << ":" << ec.message() << std::endl;
                                    }
                                });
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

        // boost::asio::async_read(_socket, m_receiveBuffer, boost::asio::transfer_at_least(1),

        // boost::asio::async_read_until(_socket, m_receiveBuffer, "\t\n",
        boost::asio::async_read(_socket, boost::asio::dynamic_buffer(m_buffer), boost::asio::transfer_at_least(1),
                                [this, &_socket](boost::system::error_code ec, std::size_t bytesTransferred) mutable
                                {
                                    std::cout << "------------lamda read------------" << std::endl;
                                    std::cout << "Bytest transf to read buffer: " << bytesTransferred << std::endl;
                                    if (!ec)
                                    {
                                        if (bytesTransferred > 0)
                                        {
                                            //   std::string receivedMessage;
                                            std::unique_lock lock(m_mutex);
                                            //   std::istream is(&m_receiveBuffer);
                                            //   std::getline(is, receivedMessage);
                                            //   std::string receivedMessage(m_buffer.begin(), m_buffer.begin() + bytesTransferred);
                                            std::string receivedMessage = m_buffer;
                                            lock.unlock();
                                            //   m_receiveBuffer.consume(bytesTransferred);
                                            //   std::cout << "Node: "
                                            // << "N Received: " << m_buffer << std::endl;
                                            std::cout << "Node: "
                                                      << "N Received: " << receivedMessage << std::endl;

                                            m_receiveCallback(receivedMessage);
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
                                    // do_read(conn);
                                    do_read_header(m_sockets.back());
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
                    do_read_header(m_sockets.back());
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

    // std::vector<char> m_buffer;
    std::string m_buffer;

    std::function<void(std::string)> m_receiveCallback;

    std::mutex m_mutex;

    chat_message read_msg_;
    chat_message_queue write_msgs_;

    boost::asio::io_context &m_io_context;
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
#include "session.hpp"

Session::~Session()
{
  DEBUG_LOG("~Session() deleted");
  if (m_socket.is_open())
  {
    m_socket.close();
  }
}

void Session::handleWrite(std::string const &mes, boost::system::error_code ec,
                          std::size_t bytesTransferred) const
{
  if (!ec)
  {
    TRACE_LOG("Message sended: " << mes << " Bytes: " << bytesTransferred);
  }
  else
  {
    ERROR_LOG("Error sending message: " << ec.message());
  }
};

// void Session::sendMessage(std::string mes)
// {
//   auto self(shared_from_this());
//   if (!m_socket.is_open())
//   {
//     ERROR_LOG("Socket closed");
//     return;
//   }

//   TRACE_LOG("Socket is open");
//   TRACE_LOG("Sending: remote endpoint " << m_socket.remote_endpoint());

//   // boost::asio::async_write(m_socket, boost::asio::buffer(mes + cDelimiter),
//   boost::asio::async_write(m_socket, boost::asio::buffer(mes),
//                            std::bind(&Session::handleWrite, this, mes,
//                                      std::placeholders::_1,
//                                      std::placeholders::_2));
// }

void Session::sendMessage(std::string mes)
{
  message msg;
  msg.body_length(mes.length());
  std::memcpy(msg.body(), mes.c_str(), msg.body_length());
  msg.encode_header();
  this->write(msg);
}

void Session::handleRead(boost::system::error_code ec,
                         std::size_t bytesTransferred)
{
  TRACE_LOG("------------lamda read------------");
  TRACE_LOG("Socket available: " << m_socket.available());
  TRACE_LOG("ADDR: " << m_address);
  TRACE_LOG("Bytes transf to read buffer: " << bytesTransferred);
  if (bytesTransferred == 0)
  {
    return;
  }

  if (!ec)
  {
    std::unique_lock<std::mutex> lock(m_buf_mutex);
    std::string receivedMessage = m_buffer;
    m_buffer.clear();
    lock.unlock();

    if (m_receiveHandler != nullptr)
    {
      m_receiveHandler(shared_from_this().get(), std::move(receivedMessage));
    }
    receiving(); // Continue receiving
  }
  else if (ec == boost::asio::error::eof)
  {
    // The remote peer closed the connection gracefully
    INFO_LOG("Connection closed by remote peer.");
    m_socket.close(); // Close the socket

    // Remove the session from the server
    m_server.removeSession(shared_from_this());
  }
  else
  {
    // Handle other errors
    ERROR_LOG("Error during read: " << ec.message());
    m_socket.close(); // Close the socket

    // Remove the session from the server
    m_server.removeSession(shared_from_this());
  }
}

void Session::receiving()
{
  DEBUG_LOG("------------receiving------------");
  if (!m_socket.is_open())
  {
    TRACE_LOG("Socket is not open");
  }
  else
  {
    TRACE_LOG("Socket is open");
  }
  boost::asio::async_read(m_socket, boost::asio::dynamic_string_buffer(m_buffer),
                          boost::asio::transfer_at_least(1), std::bind(&Session::handleRead, this, std::placeholders::_1, std::placeholders::_2));
}

void Session::addReceiveHandler(TReceiveHandler lamda)
{
  if (lamda != nullptr)
  {
    DEBUG_LOG("Receive handler is ready");
    m_receiveHandler = lamda;
  }
}

void Session::write(const message &msg)
{
  TRACE_FUNCTION
  TRACE_LOG("Write " << std::string(msg.data(), msg.length()));
  boost::asio::post(m_server.getContext(),
                    [this, msg]()
                    {
                      bool write_in_progress = !m_write_msgs.empty();
                      m_write_msgs.push_back(msg);
                      if (!write_in_progress)
                      {
                        do_write();
                      }
                    });
}

void Session::do_write()
{
  TRACE_FUNCTION
  boost::asio::async_write(m_socket,
                           boost::asio::buffer(m_write_msgs.front().data(),
                                               m_write_msgs.front().length()),
                           [this](boost::system::error_code ec, std::size_t length)
                           {
                             if (!ec)
                             {
                               TRACE_LOG("Writed "
                                  << std::string(m_write_msgs.front().data(), m_write_msgs.front().length()) );
                               m_write_msgs.pop_front();
                               if (!m_write_msgs.empty())
                               {
                                 do_write();
                               }
                             }
                             else
                             {
                               ERROR_LOG("Error sending message: " << ec.message());
                             }
                           });
}

void Session::readHeader()
{
  TRACE_FUNCTION
  TRACE_LOG("------------do read header------------");
  boost::asio::async_read(m_socket,
                          boost::asio::buffer(m_read_msg.data(), message::header_length),
                          [this](boost::system::error_code ec, std::size_t length)
                          {
                            TRACE_LOG("Header length: " << length);
                            if (!ec && m_read_msg.decode_header())
                            {
                              TRACE_LOG("RM data: " << std::string(m_read_msg.data(), message::header_length));
                              TRACE_LOG("Body length before : " << m_read_msg.body_length());
                              readBody();
                            }
                            else
                            {
                              ERROR_LOG("ERROR Node Er Val:" << ec.value() << ":" << ec.message());
                            }
                          });
}

void Session::readBody()
{
  TRACE_FUNCTION
  TRACE_LOG("------------do read body------------");
  boost::asio::async_read(m_socket,
                          boost::asio::buffer(m_read_msg.body(), m_read_msg.body_length()),
                          [this](boost::system::error_code ec, std::size_t length)
                          {
                            if (!ec)
                            {
                              TRACE_LOG("Body Length: " << length);
                              {
                                std::string receivedMessage(m_read_msg.body(), m_read_msg.body_length());
                                TRACE_LOG("Received: " << receivedMessage);
                                if (m_receiveHandler != nullptr)
                                {
                                  m_receiveHandler(shared_from_this().get(), std::move(receivedMessage));
                                }
                              }
                              readHeader();
                            }
                            else
                            {
                              ERROR_LOG("ERROR Node Er Val:" << ec.value() << ":" << ec.message());
                            }
                          });
}

std::string Session::getRemoteEndpoint() const
{
  try
  {
    if (m_socket.is_open())
    {
      boost::asio::ip::tcp::endpoint remoteEndpoint = m_socket.remote_endpoint();
      boost::asio::ip::tcp::endpoint localEndpoint = m_socket.local_endpoint();
      return remoteEndpoint.address().to_string() + ":" + std::to_string(remoteEndpoint.port()) +
             +" local endpoint " + localEndpoint.address().to_string() + ":" + std::to_string(localEndpoint.port());
    }
    else
    {
      return "Socket is not connected";
    }
  }
  catch (const std::exception &e)
  {
    return "Error retrieving remote endpoint: " + std::string(e.what());
  }
}
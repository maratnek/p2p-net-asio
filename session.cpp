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
    DEBUG_LOG("Message sended: " << mes << " Bytes: " << bytesTransferred);
  }
  else
  {
    DEBUG_LOG("Error sending message: " << ec.message());
  }
};

void Session::sendMessage(std::string mes)
{
  auto self(shared_from_this());
  if (!m_socket.is_open())
  {
    ERROR_LOG("Socket closed");
    return;
  }

  DEBUG_LOG("Socket is open");
  DEBUG_LOG("Sending: remote endpoint " << m_socket.remote_endpoint());

  boost::asio::async_write(m_socket, boost::asio::buffer(mes + cDelimiter),
                           std::bind(&Session::handleWrite, this, mes,
                                     std::placeholders::_1,
                                     std::placeholders::_2));
}

void Session::handleRead(boost::system::error_code ec,
                         std::size_t bytesTransferred)
{
  DEBUG_LOG("------------lamda read------------");
  DEBUG_LOG("Socket available: " << m_socket.available());
  DEBUG_LOG("Bytes transf to read buffer: " << bytesTransferred);
  if (!ec)
  {
    std::istream is(&m_receiveBuffer);
    std::string receivedMessage;
    std::getline(is, receivedMessage);
    DEBUG_LOG("Node: Received: " << receivedMessage);
    if (m_receiveHandler != nullptr)
    {
      m_receiveHandler(receivedMessage);
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

void Session::addReceiveHandler(std::function<void(std::string message)> lamda)
{
  if (lamda != nullptr)
  {
    DEBUG_LOG("Receive handler is ready");
    m_receiveHandler = lamda;
  }
}

void Session::receiving()
{
  DEBUG_LOG("------------receiving------------");
  if (!m_socket.is_open())
  {
    DEBUG_LOG("Socket is not open");
  }
  else
  {
    DEBUG_LOG("Socket is open");
  }
  boost::asio::async_read_until(m_socket, m_receiveBuffer, cDelimiter, std::bind(&Session::handleRead, this, std::placeholders::_1, std::placeholders::_2));
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
             + " local endpoint " 
             + localEndpoint.address().to_string() + ":" + std::to_string(localEndpoint.port());
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
#include "session.hpp"

void Session::handle_read(boost::system::error_code ec,
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
        receiving(); // Continue receiving
    }
    else if (ec == boost::asio::error::eof)
    {
        // The remote peer closed the connection gracefully
        std::cout << "Connection closed by remote peer." << std::endl;
        m_socket.close(); // Close the socket

        // Remove the session from the server
        m_server.removeSession(shared_from_this());
    }
    else
    {
        // Handle other errors
        std::cout << "Error during read: " << ec.message() << std::endl;
        m_socket.close(); // Close the socket

        // Remove the session from the server
        m_server.removeSession(shared_from_this());
    }
}

void Session::receiving()
{
    std::cout << "------------receiving------------" << std::endl;
    if (!m_socket.is_open())
    {
        std::cout << "Socket is not open" << std::endl;
    }
    else
    {
        std::cout << "Socket is open" << std::endl;
    }
    boost::asio::async_read_until(m_socket, m_receiveBuffer, '\n', std::bind(&Session::handle_read, this, std::placeholders::_1, std::placeholders::_2));
}

  std::string Session::getRemoteEndpoint() const
  {
    try {
      if (m_socket.is_open()) {
        boost::asio::ip::tcp::endpoint remoteEndpoint = m_socket.remote_endpoint();
        return remoteEndpoint.address().to_string() + ":" + std::to_string(remoteEndpoint.port());
      } else {
        return "Socket is not connected";
      }
    } catch (const std::exception& e) {
      return "Error retrieving remote endpoint: " + std::string(e.what());
    }
  }
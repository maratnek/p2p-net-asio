#include "server.hpp"
#include "session.hpp"

void Server::addReceiveHandler(std::function<void(std::string message)> lamda)
{
    if (lamda != nullptr) {
      std::cout << "Add receive handler" << std::endl;
      m_receiveHandler = lamda;
    }
}

void Server::runServer()
{
    std::cout << "Running... " << std::endl;
    auto t = std::thread([this]() {
      for (;;) {
        try {
          std::cout << "Server run " << m_port << std::endl;
          m_io_context.run();
          std::cout << "Run exited" << std::endl;
          break; // run() exited normally
        } catch (std::exception &e) {
          // Deal with exception as appropriate.
          std::cerr << "Failed run context: " << e.what() << std::endl;
        }
      }
    });
    std::swap(m_thread, t);
}

void Server::removeSession(std::shared_ptr<Session> session)
{
    std::lock_guard<std::mutex> lock(m_sessionMutex);
    std::cout << "Removing session !!!" << std::endl;
    m_sessions_conn.remove(session);
    m_sessions_accept.remove(session);
}

void Server::sendToAll(std::string const &message)
{
    std::cout << "Conn Sessions size before send messages: " << m_sessions_conn.size() << std::endl;
    for (auto &session : m_sessions_conn)
    {
        session->sendMessage(message);
    }
}

void Server::sendToAllAccepter(std::string const &message)
{
    std::cout << "Accepter Sessions size before send messages: " << m_sessions_accept.size() << std::endl;
    for (auto &session : m_sessions_accept)
    {
        session->sendMessage(message);
    }
}

void Server::connect(const std::string &targetAddress, short unsigned targetPort)
{
    std::cout << "------------CONNECT------------" << std::endl;
    auto &socket = m_sockets.emplace_back(m_acceptor.get_executor());

    boost::asio::ip::tcp::endpoint endpoint(
        boost::asio::ip::address::from_string(targetAddress), targetPort);

    socket.async_connect(endpoint, [this, targetPort, &socket](boost::system::error_code ec)
                         {
      if (!ec) {
          auto session = std::make_shared<Session>(std::move(socket), *this);
          session->addReceiveHandler(m_receiveHandler);
          session->start();

          std::lock_guard<std::mutex> lock(m_sessionMutex);
          m_sessions_conn.emplace_back(session);
          std::cout << "Success Connected to server (port: " << targetPort << ")"
                    << " list conn s: " << m_sessions_conn.size() << std::endl;
      } else {
        std::cout << "Failed to connect to (port: " << targetPort << ")"
                  << ": " << ec.message() << std::endl;
      } });
}

void Server::accept()
{
    m_acceptor.async_accept(
        [this](boost::system::error_code ec, tcp::socket socket)
        {
            if (!ec)
            {
                std::cout << "------------ACCEPT------------" << std::endl;
                auto endpoint = socket.remote_endpoint();
                auto lendp = socket.local_endpoint();

                auto session = std::make_shared<Session>(std::move(socket), *this);

                session->addReceiveHandler(m_receiveHandler);

                {
                    std::lock_guard<std::mutex> lock(m_sessionMutex);
                    m_sessions_accept.emplace_back(session);
                }
                session->start(); // Start the session here

                std::cout << "Accepted connection from remote endp: " << endpoint.address()
                          << ":" << endpoint.port() << " local endpoint: " << lendp.port()
                          << " list s: " << m_sessions_accept.size() << std::endl;

                // Log remote endpoint information again to double-check
                std::cout << "Remote endpoint after accepting: " << session->getRemoteEndpoint() << std::endl;
            }
            else
            {
                std::cout << "Error accepting connection: " << ec.message() << std::endl;
            }

            accept(); // Continue accepting connections
        });
}
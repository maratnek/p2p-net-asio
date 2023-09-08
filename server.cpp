#include "server.hpp"
#include "session.hpp"

Server::Server(const std::string &address, unsigned short port)
    : m_address(address)
    , m_port(port)
    , m_acceptor(m_io_context, tcp::endpoint(tcp::v4(), port))
{
    DEBUG_LOG("Initializing server");
    m_acceptor.listen(m_port);
    accept();
}

Server::~Server()
{
    DEBUG_LOG("~Server destructed");
    if (m_thread.joinable())
    {
        DEBUG_LOG("Stop context and Join thread");
        m_io_context.stop();
        m_thread.join();
    }
}

void Server::addReceiveHandler(std::function<void(std::string message)> lamda)
{
    TRACE_FUNCTION
    if (lamda != nullptr)
    {
        DEBUG_LOG("Add receive handler");
        m_receiveHandler = lamda;
    }
}

void Server::runServer()
{
    DEBUG_LOG("Running... ");
    auto t = std::thread([this]()
                         {
      for (;;) {
        try {
          DEBUG_LOG("Server run " << m_port);
          m_io_context.run();
          DEBUG_LOG("Run exited");
          break; // run() exited normally
        } catch (std::exception &e) {
          // Deal with exception as appropriate.
          ERROR_LOG("Failed run context: " << e.what());
        }
      } });
    std::swap(m_thread, t);
}

void Server::removeSession(std::shared_ptr<Session> session)
{
    std::lock_guard<std::mutex> lock(m_sessionMutex);
    DEBUG_LOG("Removing session !!!");
    m_sessions_conn.remove(session);
    m_sessions_accept.remove(session);
}

void Server::sendToAll(std::string const &message)
{
    DEBUG_LOG("Conn Sessions size before send messages: " << m_sessions_conn.size());
    for (auto &session : m_sessions_conn)
    {
        session->sendMessage(message);
    }
}

void Server::sendToAllAccepter(std::string const &message)
{
    DEBUG_LOG("Accepter Sessions size before send messages: " << m_sessions_accept.size());
    for (auto &session : m_sessions_accept)
    {
        session->sendMessage(message);
    }
}

void Server::connect(const std::string &targetAddress, short unsigned targetPort)
{
    DEBUG_LOG("------------CONNECT------------");
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
          DEBUG_LOG("Success Connected to server (port: " << targetPort << ")"
                    << " list conn s: " << m_sessions_conn.size());
      } else {
        DEBUG_LOG("Failed to connect to (port: " << targetPort << ")"
                  << ": " << ec.message());
      } });
}

void Server::accept()
{
    m_acceptor.async_accept(
        [this](boost::system::error_code ec, tcp::socket socket)
        {
            if (!ec)
            {
                DEBUG_LOG("------------ACCEPT------------");
                auto endpoint = socket.remote_endpoint();
                auto lendp = socket.local_endpoint();

                auto session = std::make_shared<Session>(std::move(socket), *this);

                session->addReceiveHandler(m_receiveHandler);

                {
                    std::lock_guard<std::mutex> lock(m_sessionMutex);
                    m_sessions_accept.emplace_back(session);
                }
                session->start(); // Start the session here

                DEBUG_LOG("Accepted connection from remote endp: " << endpoint.address()
                                                                   << ":" << endpoint.port() << " local endpoint: " << lendp.port()
                                                                   << " list s: " << m_sessions_accept.size());

                // Log remote endpoint information again to double-check
                DEBUG_LOG("Remote endpoint after accepting: " << session->getRemoteEndpoint());
            }
            else
            {
                DEBUG_LOG("Error accepting connection: " << ec.message());
            }

            accept(); // Continue accepting connections
        });
}
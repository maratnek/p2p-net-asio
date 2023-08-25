#ifndef __P2P_NODES_HPP__
#define __P2P_NODES_HPP__

#include <boost/asio.hpp>

using namespace boost::asio::ip::tcp;

class Session {

};

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

#endif // __P2P_NODES_HPP__
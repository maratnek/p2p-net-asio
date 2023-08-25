#include <boost/asio.hpp>
#include <iostream>
#include <list>
using boost::asio::ip::tcp;
using namespace std::chrono_literals;

struct P2PNode {
    P2PNode(boost::asio::io_context& io_context, tcp::endpoint const& endpoint, int nodeType)
        : acceptor_(io_context, endpoint)
        , listeningSocket_(io_context) {
        this->nodeType_ = nodeType;
        do_accept();
    }

    void do_connect(tcp::endpoint const& endpoint) {
        tcp::socket socket(acceptor_.get_executor());
        std::cout << "executor: " << socket.get_executor() << std::endl;
        socket.async_connect(endpoint, [this, &socket, endpoint](boost::system::error_code ec) {
            std::cout << "------------do connect------------";
            if (!ec) {
                std::cout << "Connected to " << endpoint << std::endl;

                // Add the connected socket to the list of sockets
                sockets_.emplace_back(std::move(socket));

                // Send a message to the connected server
                std::string message = "Hello from node " + std::to_string(nodeType_);
                data_ = message;
                do_write(sockets_.back(), message.size());
            } else {
                std::cout << "Failed to connect to " << endpoint << ": " << ec.message() << std::endl;
            }
        });
    }

    void do_accept() {
        std::cout << "Start Listening" << std::endl;
        acceptor_.async_accept([this](boost::system::error_code ec, tcp::socket socket) {
            if (!ec) {
                std::cout << "Connection established with " << socket.remote_endpoint() << std::endl;
                sockets_.emplace_back(std::move(socket));
                do_read(sockets_.back());
            }
            do_accept();
        });
    }

    void do_write(tcp::socket&, size_t) {}
    void do_read(tcp::socket&) {}

  private:
    tcp::acceptor acceptor_;
    tcp::socket   listeningSocket_;
    int           nodeType_ = 0;
    std::string   data_;

    std::list<tcp::socket> sockets_;
};

int main(int argc, char** argv) try {
    uint16_t const port = argc > 1 ? atoi(argv[1]) : 8989;
    std::cout << "listening port: " << port << std::endl;
    boost::asio::io_context ioc;
    P2PNode n(ioc, {{},port}, 42);
    uint16_t y = 0;
    std::cin >> y;
    if (y==1){
        n.do_connect({{},5000});
    }


    ioc.run_for(10s);

} catch (std::exception const& e) {
    std::cout << "Exception was thrown in function: " << e.what() << std::endl;
} catch (...) {
    std::cout << "Unhandled exception" << std::endl;
}

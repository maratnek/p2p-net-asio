#include "p2p-node.hpp"
#include "config.hpp"

#include <csignal>

#include <atomic>

    std::atomic<int> count = 0;

void signal_handler(int signal_num) {
        std::cout << "Receive message: " << count << std::endl;
        exit(signal_num);
    }

int main(int argc, char** argv) try {

    std::string address = "127.0.0.1";
    std::string sPort = "5000";
    std::string node = "1";

    if (argc != 2) {
        throw std::runtime_error("Execute parameter must be two arguments.Configuration");
    }

    auto conf = std::make_shared<config::Configuration>(argv[1]);

    std::map<std::string, std::string> config = conf->getMainConfig();
    for (const auto &pair : config)
    {
        std::cout << pair.first << ": " << pair.second << std::endl;
    }
    if (config.contains("Address"))
    {
        address = config["Address"];
    }
    if (config.contains("Port"))
    {
        sPort = config["Port"];
    }
    
    uint16_t const port = argc > 1 ? atoi(sPort.c_str()) : 5000;
    std::cout << "listening port: " << port << std::endl;
    boost::asio::io_context ioc;
    Server server(ioc, "127.0.0.1", port);

    
    std::cout << "Connecting..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(2));

    auto sPeers = conf->getPeers();
    for (const auto &p : sPeers) {
        std::cout << "Connect to Peer: " << p.first << " : " << p.second << std::endl;
        server.Connect(ioc, p.first, p.second);
    }

    std::cout << "Running... " << std::endl;
    std::thread t([&ioc]()
                  { ioc.run(); });
    

    // register signal SIGABRT and signal handler
    signal(SIGINT, signal_handler);
    
    server.receive([](std::string msg){
        std::cout << count++ << " Receive: " << msg << std::endl;
    });

    std::this_thread::sleep_for(std::chrono::seconds(1)); // Wait for a moment

    // while (true) 
    for (size_t i = 0; i < 10; i++)
    {
        server.do_write_all("_" + std::to_string(i) + " Hello from node:" + sPort + "!");
        // std::this_thread::sleep_for(std::chrono::seconds(2));
        // std::this_thread::sleep_for(std::chrono::nanoseconds(2));
    }

    t.join();

} catch (std::exception const& e) {
    std::cout << "Exception was thrown in function: " << e.what() << std::endl;
} catch (...) {
    std::cout << "Unhandled exception" << std::endl;
}
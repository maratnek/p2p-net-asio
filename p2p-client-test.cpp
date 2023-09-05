#include "server.hpp"
#include "config.hpp"

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

    Server p2p_server("127.0.0.1", port);
    Server clinets_server("127.0.0.1", port + 50);


    clinets_server.addReceiveHandler([&](std::string mes){
        std::cout << "Client send message!!! " << mes << std::endl;
        clinets_server.sendToAllAccepter("PING FROM SERVER");
        // handle the message and send it to all peers
        p2p_server.sendToAll("SEND TO ALL" + mes);
        // send the response to client
        clinets_server.sendToAllAccepter("SEND RESPONSE");
    });

    p2p_server.runServer();

    std::this_thread::sleep_for(std::chrono::seconds(2));
    std::cout << "Connecting..." << std::endl;
    auto sPeers = conf->getPeers();
    for (const auto &p : sPeers) {
        std::cout << "Connect to Peer: " << p.first << " : " << p.second << std::endl;
        p2p_server.connect(p.first, p.second);
    }

    std::this_thread::sleep_for(std::chrono::seconds(2));

    clinets_server.runServer();

    std::this_thread::sleep_for(std::chrono::seconds(1));
    p2p_server.sendToAll("First message to all. For testing purposes");


    // auto counter = 0;
    // while (true) {
    //     p2p_server.sendToAll(std::to_string(counter++) + " Hello from node:" + sPort + "!");
    //     // std::this_thread::sleep_for(std::chrono::seconds(2));
    //     std::this_thread::sleep_for(std::chrono::nanoseconds(2));
    // }

} catch (std::exception const& e) {
    std::cout << "Exception was thrown in function: " << e.what() << std::endl;
} catch (...) {
    std::cout << "Unhandled exception" << std::endl;
}
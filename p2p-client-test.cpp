#include "server.hpp"
#include "config.hpp"

#include "net-manager.hpp"


#include <logger.hpp>
using namespace logger;

int main(int argc, char** argv) try {


    Logger::initialize();
    // Logger::setLogLevel(LogLevel::INFO);

    TRACE_LOG("P2P client test started");


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
        DEBUG_LOG(pair.first << ": " << pair.second);
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
    DEBUG_LOG("listening port: " << port);

    Server p2p_server("127.0.0.1", port);
    Server clinets_server("127.0.0.1", port + 50);



    int id = 0;

    // Network manager, handle events, router
    NetManager net_manager(p2p_server, clinets_server);

    p2p_server.runServer();

    std::this_thread::sleep_for(std::chrono::seconds(2));
    DEBUG_LOG("Connecting...");
    auto sPeers = conf->getPeers();
    for (const auto &p : sPeers) {
        DEBUG_LOG("Connect to Peer: " << p.first << " : " << p.second);
        p2p_server.connect(p.first, p.second);
    }

    std::this_thread::sleep_for(std::chrono::seconds(2));

    clinets_server.runServer();

    std::this_thread::sleep_for(std::chrono::seconds(1));
    p2p_server.sendToAll("First message to all. For testing purposes");


    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(2));
    //     std::this_thread::sleep_for(std::chrono::nanoseconds(2));
    }

} catch (std::exception const& e) {
    DEBUG_LOG("Exception was thrown in function: " << e.what());
} catch (...) {
    DEBUG_LOG("Unhandled exception");
}
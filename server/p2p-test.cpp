// #include "p2p-node.hpp"
#include "config.hpp"
#include "tps-measure.hpp"
#include "server.hpp"

#include "logger.hpp"
using namespace logger;

#include <csignal>

#include <atomic>

// TODO only test
inline void artificialPayload()
{
    for (int i = 0; i < 1000; ++i)
    {
        double result = std::sqrt(static_cast<double>(i)) * std::log(static_cast<double>(i));
        result = std::sin(result);
    }
}

std::atomic<int> count = 0;

void signal_handler(int signal_num) {
        std::cout << "Receive message: " << count << std::endl;
        exit(signal_num);
    }

int main(int argc, char** argv) try {

    // Initialize Logger 
    Logger::initialize();
    Logger::setLogLevel(LogLevel::DEBUG);

    TPSMeasuring tps_measure;


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
    Server p2pServ("127.0.0.1", port);

    
    std::cout << "Connecting..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(1));

    auto sPeers = conf->getPeers();
    for (const auto &p : sPeers) {
        INFO_LOG("Connect to Peer: " << p.first << " : " << p.second);
        p2pServ.connect(p.first, p.second);
    }

    // register signal SIGABRT and signal handler
    signal(SIGINT, signal_handler);
    
    p2pServ.addReceiveHandler([&tps_measure](Session* session, std::string msg){
        // measuring tps std::to_string(count++)
        tps_measure.StartTransaction();
        artificialPayload();
        INFO_LOG("Received: " << msg);
        tps_measure.FinishTransaction();

    });

    p2pServ.runServer();

    INFO_LOG("Running... ");

    std::this_thread::sleep_for(std::chrono::seconds(1)); // Wait for a moment

    for (size_t i = 0; i < 10000; i++)
    {
        p2pServ.sendToAll("_" + std::to_string(i) + " Hello from node:" + sPort + "!");
        // std::this_thread::sleep_for(std::chrono::seconds(1));
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    while (true) 
        std::this_thread::sleep_for(std::chrono::seconds(5));


} catch (std::exception const& e) {
    std::cout << "Exception was thrown in function: " << e.what() << std::endl;
} catch (...) {
    std::cout << "Unhandled exception" << std::endl;
}
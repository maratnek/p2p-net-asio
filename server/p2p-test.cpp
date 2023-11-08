// #include "p2p-node.hpp"
#include "config.hpp"
#include "tps-measure.hpp"
#include "server.hpp"

#include "logger.hpp"
using namespace logger;

#include <csignal>
#include <unordered_map>
#include <chrono>

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
// delay average
std::atomic<double> avg = 0;

std::unordered_map<std::size_t, std::chrono::time_point<std::chrono::steady_clock>> tpSet{10000};
std::mutex tpSetMutex;

void startTimer(size_t id) {
    std::lock_guard<std::mutex> lock(tpSetMutex);
    tpSet[id] = std::chrono::steady_clock::now();
}


auto finishTimer(size_t id) {
    std::lock_guard<std::mutex> lock(tpSetMutex);
    auto it = tpSet.find(id);
    if (it != tpSet.end()) {
        auto transactionEndTime = std::chrono::steady_clock::now();
        auto transactionDuration = std::chrono::duration_cast<std::chrono::microseconds>(transactionEndTime - it->second);
        INFO_LOG("Message responce " << id << ": Duration: "<< transactionDuration.count() << " microseconds");
        return transactionDuration.count();
    }
    return static_cast<long int>(avg);
}


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
    
    p2pServ.addReceiveHandler([](Session* session, std::string msg){
        // measuring tps std::to_string(count++)
        // INFO_LOG("Received: " << msg);
        if (!msg.empty()) {
            try
            {
                auto const id = atoi(msg.c_str());
                auto delay = finishTimer(id);
                avg = (avg + delay)/2;
                INFO_LOG("average is: " << avg);
            }
            catch(const std::exception& e)
            {
                std::cerr << e.what() << " atoi failed " <<  '\n';
            }
            
        }
        // artificialPayload();

    });

    p2pServ.runServer();

    INFO_LOG("Running... ");

    std::this_thread::sleep_for(std::chrono::seconds(1)); // Wait for a moment

    for (size_t i = 0; i < 10'000; i++)
    {
        startTimer(i);
        p2pServ.sendToAll(std::to_string(i));
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
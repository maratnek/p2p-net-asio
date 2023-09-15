#include "server.hpp"
#include "config.hpp"

#include "utils.hpp"

#include <exception>

#include <logger.hpp>
using namespace logger;

int main(int argc, char** argv) try {

    Logger::initialize("[%^%L%$][%H:%M:%S.%e] [%^%l%$] [File: %s] [Td: %t] %v");
    // Logger::setLogLevel(LogLevel::INFO);

    TRACE_LOG("Client test started");

    std::string address = "127.0.0.1";
    std::string sPort = "5000";
    std::string node = "1";

    if (argc != 2) {
        const char* err = "Execute parameter must be two arguments.Configuration";
        ERROR_LOG(err);
        throw std::runtime_error(err);
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

    Server server("127.0.0.1", port);
    server.runServer();

    auto targetAddress = "127.0.0.1";
    auto targetPort = 5555+50;

    DEBUG_LOG("Connecting...");
    server.connect(targetAddress, targetPort);

    std::this_thread::sleep_for(std::chrono::seconds(2)); // Wait for a moment

    for (size_t i = 0; i < 4; i++)
    {
        std::string mes = utils::createWalletMessage();
        // TODO it should be send only to one Node
        server.sendToAll(mes);
        // server.sendToAll("Id: " + std::to_string(i) + " Hello from Client with port :" + sPort + "!");
    }

    // TODO block main thread until (in the future start context in the main thread)
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(3));
    //     std::this_thread::sleep_for(std::chrono::nanoseconds(2));
    }

} catch (std::exception const& e) {
    ERROR_LOG("Exception was thrown in function: " << e.what());
} catch (...) {
    ERROR_LOG("Unhandled exception");
}
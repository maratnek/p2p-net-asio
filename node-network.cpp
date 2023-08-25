#include <string>
#include <iostream>
#include <map>
#include <fstream>
#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/ostreamwrapper.h>

#include "P2PNode.hpp"

std::map<std::string, std::string> parseConfigFile(const std::string& filename) {
    std::map<std::string, std::string> config;

    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error opening config file." << std::endl;
        return config;
    }

    rapidjson::IStreamWrapper isw(file);
    rapidjson::Document document;
    document.ParseStream(isw);

    for (const auto& member : document.GetObject()) {
        config[member.name.GetString()] = member.value.GetString();
    }

    return config;
}

int main(int argc, char* argv[]) {
    // if (argc != 2) {
    //     std::cerr << "Usage: " << argv[0] << " <config_file>" << std::endl;
    //     return 1;
    // }

    std::string address = "127.0.0.1";
    std::string port = "5000";
    std::string node = "1";

    if (argc == 2) {
        std::map<std::string, std::string> config = parseConfigFile(argv[1]);
        for (const auto &pair : config)
        {
            std::cout << pair.first << ": " << pair.second << std::endl;
        }
    }

    boost::asio::io_context ioContext;

    P2PNode node1(ioContext, "127.0.0.1", 5000);
    P2PNode node2(ioContext, "127.0.0.1", 5001);

    node1.ConnectToNode("127.0.0.1", 5001);
    // node2.ConnectToNode("127.0.0.1", 5000);
    
    std::thread t([&ioContext]() { ioContext.run(); });

    std::this_thread::sleep_for(std::chrono::seconds(2)); // Wait for a moment

    node1.SendMessage("Hello from node1!\n");
    node1.SendMessage("Hello from node1!\n");
    node2.SendMessage("Hello from node2!\n");

    t.join();

    return 0;
}

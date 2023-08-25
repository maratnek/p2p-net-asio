#include <string>
#include <iostream>
#include <map>
#include <fstream>
#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/ostreamwrapper.h>

#include <boost/asio.hpp>

#include "P2PNode.hpp"

using boost::asio::ip::tcp;

int main(int argc, char* argv[]) {
    std::string address = "127.0.0.1";
    std::string port = "5000";

    if (argc == 2) {
        port = argv[1];
    }
    std::cout << "Listening on " << address << ":" << port << std::endl;

 // Get configuration values
    const std::string nodeName = "NodeName";
    int portInt = std::stoi(port);

    // Initialize Boost Asio
    boost::asio::io_context io_context;
    tcp::socket socket(io_context);

    // Resolve the endpoint
    tcp::resolver resolver(io_context);
    tcp::resolver::results_type endpoints = resolver.resolve(address, port);

    // Connect to the server
    boost::asio::connect(socket, endpoints);

    // Send a message
    std::string message = "Hello from " + nodeName + "\n";
    boost::asio::write(socket, boost::asio::buffer(message));

    std::cout << "Message sent: " << message;

    return 0;
}
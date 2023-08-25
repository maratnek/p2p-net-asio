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

int main(int argc, char** argv) try {
    std::string address = "127.0.0.1";
    std::string port = "5000";

    if (argc >= 2) {
        port = argv[1];
    }
    std::cout << "Listening on " << address << ":" << port << std::endl;

    boost::asio::io_context ioContext;

    P2PNode node(ioContext, "127.0.0.1", std::stoi(port));

    std::cout << "Connect to node " << std::endl;
    if (argc >= 3) {
        auto t = argv[2];
        node.ConnectToNode("127.0.0.1", std::stoi(t));
    }
    // if (argc >= 4) {
    //     auto t = argv[3];
    //     node.ConnectToNode("127.0.0.1", std::stoi(t));
    // }

    std::cout << "Running... " << std::endl;
    std::thread t([&ioContext]()
                  { ioContext.run(); });

    std::this_thread::sleep_for(std::chrono::seconds(3)); // Wait for a moment

    std::cout << "Send message " << std::endl;
    node.SendMessage("Hello from node:" + port + "!\n");
    // node.StartReceiving();
    while (true) {
        node.SendMessage("Hello from node:" + port + "!\n");
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }

    t.join();

    return 0;

} catch (std::exception const& e) {
    std::cout << "Exception was thrown in function: " << e.what() << std::endl;
} catch (...) {
    std::cout << "Unhandled exception" << std::endl;
}

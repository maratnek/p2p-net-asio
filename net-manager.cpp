#include "net-manager.hpp"
#include "session.hpp"
#include "server.hpp"

#include "event.hpp"

NetManager::NetManager(Server &p2pserver, Server &client)
    : m_p2pServer(p2pserver)
    , m_client(client)
{
    // MainHandler mainHandler;
    EventHandler eventHandler;
    m_client.addReceiveHandler([&, this](std::shared_ptr<Session> resSession, std::string mes){
        DEBUG_LOG("Client send message!!! " << mes);
        // Acknowlegment
        resSession->sendMessage("PING FROM SERVER");
        eventHandler.receiveAndHandleEvent(mes);
        // handle the message and send it to all peers
        this->handleMessage(mes);
        m_p2pServer.sendToAll("SEND TO ALL: " + mes);
    });

    m_p2pServer.addReceiveHandler([&](std::shared_ptr<Session> resSession, std::string mes) {
        DEBUG_LOG("Client send message from P2P!!! " << mes);
        // send the response to client if p2p server receive message

        if (mes.find("Answered") == std::string::npos) {
            DEBUG_LOG("Answered from server: " << resSession->getAddress());
            resSession->sendMessage("Answered from server: " + std::to_string(resSession->getAddress()));
        } else {
            // then send to client
            m_client.sendToAllAccepter("ANSWER from peers " +  std::to_string(resSession->getAddress()));
        }

    });
}

NetManager::~NetManager()
{
}

void NetManager::handleMessage(std::string const& mes) {
    
}
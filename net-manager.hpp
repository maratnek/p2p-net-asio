#ifndef __NET_MANAGER_HPP__
#define __NET_MANAGER_HPP__

#include "server.hpp"
#include "event.hpp"

class NetManager
{
public:
    NetManager(Server &p2pserver, Server &client);
    ~NetManager();

    void handleMessage(std::string const &mes);

private:
    Server& m_p2pServer;
    Server& m_client;
    EventHandler m_eventHandler;
};



#endif // __NET_MANAGER_HPP__
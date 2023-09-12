#ifndef __NET_MANAGER_HPP__
#define __NET_MANAGER_HPP__

#include "server.hpp"

class NetManager
{
public:
    NetManager(Server &p2pserver, Server &client);
    ~NetManager();

    void handleMessage(std::string const &mes);

private:
    Server& m_p2pServer;
    Server& m_client;
};



#endif // __NET_MANAGER_HPP__
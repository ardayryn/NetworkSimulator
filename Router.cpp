// Router.cpp
#include "Router.h"
#include "Packet.h"
#include <iostream>

Router::Router(int id, const std::string& name)
    : NetworkDevice(id, name)
{
}

void Router::receivePacket(Packet& packet) {
    packet.decreaseTTL();
    if (packet.isExpired()) {
        std::cout << getName() << " routing packet: \"" << packet.getPayload() << "\"\n";
                return;
    }
    std::cout << getName() << " routing packet: \"" << packet.getPayload() << "\"\n"; 
}
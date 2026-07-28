// Computer.cpp
#include "Computer.h"
#include <iostream>

Computer::Computer(int id, const std::string& name)
    : NetworkDevice(id, name)
{
}

void Computer::receivePacket(Packet& packet) {
    std::cout << getName() << " received packet from "
              << packet.getSource() << ": \""
              << packet.getPayload() << "\"\n";
}

Packet Computer::createPacket(const std::string& destination,
                              const std::string& payload) {
    return Packet(nextPacketId_++, getName(), destination, payload, 10);
}
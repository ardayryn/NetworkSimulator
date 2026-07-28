// Computer.h (güncellenmiş)
#pragma once
#include "NetworkDevice.h"
#include "Packet.h"

class Computer : public NetworkDevice {
public:
    Computer(int id, const std::string& name);
    void receivePacket(Packet& packet) override;
    Packet createPacket(const std::string& destination,
                        const std::string& payload);
private:
    int nextPacketId_ = 0;   // her pakete artan id verir
};
#include "Network.h"
#include "Computer.h"
#include "Router.h"
#include <memory>
#include <iostream>

int main() {
    Network net;

    auto pc1 = std::make_unique<Computer>(1, "PC1");
    auto r1  = std::make_unique<Router>(2, "Router1");
    auto pc3 = std::make_unique<Computer>(3, "PC3");

    net.addDevice(std::move(pc1));
    net.addDevice(std::move(r1));
    net.addDevice(std::move(pc3));

    net.connect(1, 2, 5.0);
    net.connect(3, 2, 5.0);

    std::cout << "--- FIFO testi: ikisi de t=0'da, ikisi de ayni zamanda (t=5) Router1'e ulasiyor ---\n";

    Packet packetA(1, "PC1", "Router1", "Ben A", 10);
    net.schedulePacket(packetA, 0.0);

    Packet packetB(2, "PC3", "Router1", "Ben B", 10);
    net.schedulePacket(packetB, 0.0);

    net.runSimulation();

    return 0;
}
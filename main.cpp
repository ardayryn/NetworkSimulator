#include "Network.h"
#include "Computer.h"
#include "Router.h"
#include <memory>
#include <iostream>

int main() {
    Network net;

    auto pc1 = std::make_unique<Computer>(1, "PC1");
    auto r1  = std::make_unique<Router>(2, "Router1");
    auto pc2 = std::make_unique<Computer>(3, "PC2");

    net.addDevice(std::move(pc1));
    net.addDevice(std::move(r1));
    net.addDevice(std::move(pc2));

    net.connect(1, 2, 5.0, 1);
    net.connect(2, 3, 5.0, 1);

    std::cout << "--- Bant genisligi testi: kapasite=1, iki paket ayni anda (t=0) yola cikiyor ---\n";

    Packet packetA(1, "PC1", "PC2", "Ben A", 20);
    net.schedulePacket(packetA, 0.0);

    Packet packetB(2, "PC1", "PC2", "Ben B", 20);
    net.schedulePacket(packetB, 0.0);

    net.runSimulation();

    return 0;
}
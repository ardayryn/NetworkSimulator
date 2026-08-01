#include "Network.h"
#include "Computer.h"
#include "Router.h"
#include <memory>
#include <iostream>

int main() {
    Network net;

    auto pc1 = std::make_unique<Computer>(1, "PC1");
    auto r1  = std::make_unique<Router>(2, "Router1");
    auto r2  = std::make_unique<Router>(3, "Router2");
    auto pc2 = std::make_unique<Computer>(4, "PC2");
    auto pc3 = std::make_unique<Computer>(5, "PC3");

    net.addDevice(std::move(pc1));
    net.addDevice(std::move(r1));
    net.addDevice(std::move(r2));
    net.addDevice(std::move(pc2));
    net.addDevice(std::move(pc3));

    // PC1 -- Router1 -- Router2 -- PC2   (latency 5+5+5 = 15 toplam)
    net.connect(1, 2, 5.0);
    net.connect(2, 3, 5.0);
    net.connect(3, 4, 5.0);

    // PC3 -- Router1   (kisa yol, latency 2)
    net.connect(5, 2, 2.0);

    std::cout << "--- Coklu paket, gercek zamanli simulasyon ---\n";

    Packet packetA(1, "PC1", "PC2", "Ben A, uzun yoldan geliyorum", 10);
    net.schedulePacket(packetA, 0.0);

    Packet packetB(2, "PC3", "Router1", "Ben B, kisa yoldan geliyorum", 10);
    net.schedulePacket(packetB, 1.0);

    net.runSimulation();

    return 0;
}
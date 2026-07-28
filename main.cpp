#include "Network.h"
#include "Computer.h"
#include "Router.h"
#include <memory>
#include <iostream>

int main() {
    Network net;

    auto pc1 = std::make_unique<Computer>(1, "PC1");
    auto rA  = std::make_unique<Router>(2, "RouterA");   // kisa ama YAVAS yol
    auto rB  = std::make_unique<Router>(3, "RouterB");   // uzun ama HIZLI yol
    auto rC  = std::make_unique<Router>(4, "RouterC");
    auto pc2 = std::make_unique<Computer>(5, "PC2");

    net.addDevice(std::move(pc1));
    net.addDevice(std::move(rA));
    net.addDevice(std::move(rB));
    net.addDevice(std::move(rC));
    net.addDevice(std::move(pc2));

    // Yol A: PC1 -- RouterA -- PC2         (2 hop, ama YAVAS: latency 100 + 100 = 200)
    net.connect(1, 2, 100.0);
    net.connect(2, 5, 100.0);

    // Yol B: PC1 -- RouterB -- RouterC -- PC2  (3 hop, ama HIZLI: latency 1+1+1 = 3)
    net.connect(1, 3, 1.0);
    net.connect(3, 4, 1.0);
    net.connect(4, 5, 1.0);

    std::cout << "--- Dijkstra testi: hizli ama uzun yol secilmeli (RouterB->RouterC) ---\n";
    Packet p(0, "PC1", "PC2", "Dijkstra test paketi", 10);
    net.sendPacket(p);

    return 0;
}
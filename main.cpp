#include "Network.h"
#include "Computer.h"
#include "Router.h"
#include <memory>
#include <vector>
#include <iostream>

int main() {
    Network net;

    auto pc1 = std::make_unique<Computer>(1, "PC1");
    auto r1  = std::make_unique<Router>(2, "Router1");
    auto pc2 = std::make_unique<Computer>(3, "PC2");

    net.addDevice(std::move(pc1));
    net.addDevice(std::move(r1));
    net.addDevice(std::move(pc2));

    net.connect(1, 2, 5.0, 20);
    net.connect(2, 3, 5.0, 20);

    std::cout << "--- Retransmission testi: 20 paket, Lost gelirse tekrar tekrar denenir ---\n";

    std::vector<std::unique_ptr<Packet>> packets;
    for (int i = 1; i <= 20; ++i) {
        packets.push_back(std::make_unique<Packet>(i, "PC1", "PC2",
                                                     "Paket #" + std::to_string(i), 20));
        net.schedulePacket(*packets.back(), 0.0);
    }

    net.runSimulation();

    return 0;
}
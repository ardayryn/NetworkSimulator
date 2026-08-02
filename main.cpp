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

    net.connect(1, 2, 5.0, 10);
    net.connect(2, 3, 5.0, 10);

    std::cout << "--- 4 seviyeli transmisyon testi: 12 paket, rastgele sonuclar ---\n";

    // Paketler runSimulation() bitene kadar YASAMASI lazim -- yerel degisken
    // kullanip pointer verirsek, dongu bitince paket yok olur (dangling pointer!).
    // Bu yuzden hepsini bir vector<unique_ptr<Packet>> icinde sahiplenip tutuyoruz.
    std::vector<std::unique_ptr<Packet>> packets;
    for (int i = 1; i <= 12; ++i) {
        packets.push_back(std::make_unique<Packet>(i, "PC1", "PC2",
                                                     "Paket #" + std::to_string(i), 20));
        net.schedulePacket(*packets.back(), 0.0);
    }

    net.runSimulation();

    return 0;
}
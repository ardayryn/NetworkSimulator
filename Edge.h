#pragma once
#include <queue>
#include <vector>
#include <functional>
class NetworkDevice;

class Edge {
public:
    Edge(NetworkDevice* from, NetworkDevice* to, double latency, int capacity = 1);

    NetworkDevice* getFrom() const;
    NetworkDevice* getTo() const;
    double getLatency() const;
    int getCapacity() const;

    // Bir paket bu bağlantıyı 'requestedTime' anında kullanmak istiyor.
    // Boş slot varsa hemen o zamanda başlar; doluysa en erken boşalan
    // slota kadar bekletilir. Gerçek başlama zamanını döndürür.
    double reserveSlot(double requestedTime);

private:
    NetworkDevice* from_;
    NetworkDevice* to_;
    double latency_;
    int capacity_;

    // meşgul slotların "ne zaman boşalacağı" — min-heap (en erken boşalan üstte)
    std::priority_queue<double, std::vector<double>, std::greater<double>> busyUntil_;
};
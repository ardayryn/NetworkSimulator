#pragma once
#include <vector>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <queue>
#include "NetworkDevice.h"
#include "Edge.h"
class Packet;

// Bir olay: "şu zamanda, şu paket, şu cihaza ulaşacak"
struct Event {
    double time;
    Packet* packet;
    NetworkDevice* target;
};

// priority_queue'ya "en küçük zaman önce" (min-heap) demenin yolu
struct EventComparator {
    bool operator()(const Event& a, const Event& b) const {
        return a.time > b.time;
    }
};

class Network {
public:
    void addDevice(std::unique_ptr<NetworkDevice> device);
    void connect(int fromId, int toId, double latency);

    void sendPacket(Packet& packet);   // eski API: anında teslim (tek paket testleri için)

    void schedulePacket(Packet& packet, double startTime);  // yeni: olay kuyruğuna ekle
    void runSimulation();                                    // yeni: kuyruğu zaman sırasıyla işle

private:
    NetworkDevice* findDevice(int id) const;
    NetworkDevice* findDeviceByName(const std::string& name) const;
    std::vector<NetworkDevice*> findPath(NetworkDevice* source,
                                         NetworkDevice* destination);
    std::vector<NetworkDevice*> findShortestPathByLatency(NetworkDevice* source,
                                                           NetworkDevice* destination);
    double getLatencyBetween(NetworkDevice* a, NetworkDevice* b) const;

    std::vector<std::unique_ptr<NetworkDevice>> devices_;
    std::vector<std::unique_ptr<Edge>> edges_;
    std::unordered_map<NetworkDevice*, std::vector<NetworkDevice*>> adjacency_;
    std::unordered_map<NetworkDevice*, std::vector<std::pair<NetworkDevice*, double>>> weightedAdjacency_;

    std::priority_queue<Event, std::vector<Event>, EventComparator> eventQueue_;
};
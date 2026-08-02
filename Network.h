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
// target == nullptr ise, bu bir "kayıp bildirimi" olayıdır (gerçek teslim değil)
struct Event {
    double time;
    int sequence;
    Packet* packet;
    NetworkDevice* target;
    std::string note;
};

struct EventComparator {
    bool operator()(const Event& a, const Event& b) const {
        if (a.time != b.time) return a.time > b.time;
        return a.sequence > b.sequence;
    }
};

class Network {
public:
    void addDevice(std::unique_ptr<NetworkDevice> device);
    void connect(int fromId, int toId, double latency, int capacity = 1, double lossProbability = 0.0);

    void sendPacket(Packet& packet);

    void schedulePacket(Packet& packet, double startTime);
    void runSimulation();

private:
    NetworkDevice* findDevice(int id) const;
    NetworkDevice* findDeviceByName(const std::string& name) const;
    std::vector<NetworkDevice*> findPath(NetworkDevice* source,
                                         NetworkDevice* destination);
    std::vector<NetworkDevice*> findShortestPathByLatency(NetworkDevice* source,
                                                           NetworkDevice* destination);
    double getLatencyBetween(NetworkDevice* a, NetworkDevice* b) const;
    Edge* getEdgeBetween(NetworkDevice* a, NetworkDevice* b) const;

    std::vector<std::unique_ptr<NetworkDevice>> devices_;
    std::vector<std::unique_ptr<Edge>> edges_;
    std::unordered_map<NetworkDevice*, std::vector<NetworkDevice*>> adjacency_;
    std::unordered_map<NetworkDevice*, std::vector<std::pair<NetworkDevice*, double>>> weightedAdjacency_;

    std::priority_queue<Event, std::vector<Event>, EventComparator> eventQueue_;
    int nextSequence_ = 0;
};
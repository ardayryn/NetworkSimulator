#pragma once
#include <vector>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include "NetworkDevice.h"
#include "Edge.h"
class Packet;

class Network {
public:
    void addDevice(std::unique_ptr<NetworkDevice> device);
    void connect(int fromId, int toId, double latency);
    void sendPacket(Packet& packet);
private:
    NetworkDevice* findDevice(int id) const;
    NetworkDevice* findDeviceByName(const std::string& name) const;
    std::vector<NetworkDevice*> findPath(NetworkDevice* source,
                                         NetworkDevice* destination);
    std::vector<NetworkDevice*> findShortestPathByLatency(NetworkDevice* source,
                                                           NetworkDevice* destination);

    std::vector<std::unique_ptr<NetworkDevice>> devices_;
    std::vector<std::unique_ptr<Edge>> edges_;
    std::unordered_map<NetworkDevice*, std::vector<NetworkDevice*>> adjacency_;
    std::unordered_map<NetworkDevice*, std::vector<std::pair<NetworkDevice*, double>>> weightedAdjacency_;
};
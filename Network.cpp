#include "Network.h"
#include "Packet.h"
#include "Edge.h"
#include "NetworkDevice.h"
#include <queue>
#include <unordered_set>
#include <algorithm>
#include <utility>
#include <functional>

void Network::addDevice(std::unique_ptr<NetworkDevice> device) {
    devices_.push_back(std::move(device));
}

NetworkDevice* Network::findDevice(int id) const {
    for (const auto& device : devices_)
        if (device->getId() == id) return device.get();
    return nullptr;
}

NetworkDevice* Network::findDeviceByName(const std::string& name) const {
    for (const auto& device : devices_)
        if (device->getName() == name) return device.get();
    return nullptr;
}

void Network::connect(int fromId, int toId, double latency) {
    NetworkDevice* from = findDevice(fromId);
    NetworkDevice* to   = findDevice(toId);
    if (!from || !to) return;

    edges_.push_back(std::make_unique<Edge>(from, to, latency));

    // adjacency list — çift yönlü (BFS için, ağırlıksız)
    adjacency_[from].push_back(to);
    adjacency_[to].push_back(from);

    // weighted adjacency — çift yönlü (Dijkstra için, latency'li)
    weightedAdjacency_[from].push_back({to, latency});
    weightedAdjacency_[to].push_back({from, latency});
}

std::vector<NetworkDevice*> Network::findPath(NetworkDevice* source,
                                              NetworkDevice* destination) {
    std::queue<NetworkDevice*> q;
    std::unordered_set<NetworkDevice*> visited;
    std::unordered_map<NetworkDevice*, NetworkDevice*> cameFrom;

    q.push(source);
    visited.insert(source);

    while (!q.empty()) {
        NetworkDevice* current = q.front();
        q.pop();
        if (current == destination) break;

        for (NetworkDevice* neighbor : adjacency_[current]) {
            if (visited.find(neighbor) == visited.end()) {
                visited.insert(neighbor);
                cameFrom[neighbor] = current;
                q.push(neighbor);
            }
        }
    }

    std::vector<NetworkDevice*> path;
    if (destination != source && cameFrom.find(destination) == cameFrom.end()) {
        return {};   // yol yok
    }
    for (NetworkDevice* at = destination; at != nullptr; ) {
        path.push_back(at);
        if (at == source) break;
        at = cameFrom[at];
    }
    std::reverse(path.begin(), path.end());
    return path;
}

std::vector<NetworkDevice*> Network::findShortestPathByLatency(NetworkDevice* source,
                                                                NetworkDevice* destination) {
    // (mesafe, cihaz) çiftlerini tutan min-heap: her zaman en düşük mesafeli üstte
    std::priority_queue<
        std::pair<double, NetworkDevice*>,
        std::vector<std::pair<double, NetworkDevice*>>,
        std::greater<std::pair<double, NetworkDevice*>>
    > pq;

    std::unordered_map<NetworkDevice*, double> dist;
    std::unordered_map<NetworkDevice*, NetworkDevice*> cameFrom;

    dist[source] = 0.0;
    pq.push({0.0, source});

    while (!pq.empty()) {
        auto [currentDist, current] = pq.top();
        pq.pop();

        if (current == destination) break;

        // bayat kayıt kontrolü: bu mesafe artık güncel değilse atla
        if (currentDist > dist[current]) continue;

        for (const auto& [neighbor, latency] : weightedAdjacency_[current]) {
            double newDist = currentDist + latency;

            // komşu hiç görülmediyse (dist'te yoksa) ya da daha iyi bir yol bulduysak
            if (dist.find(neighbor) == dist.end() || newDist < dist[neighbor]) {
                dist[neighbor] = newDist;
                cameFrom[neighbor] = current;
                pq.push({newDist, neighbor});
            }
        }
    }

    std::vector<NetworkDevice*> path;
    if (destination != source && cameFrom.find(destination) == cameFrom.end()) {
        return {};   // yol yok
    }
    for (NetworkDevice* at = destination; at != nullptr; ) {
        path.push_back(at);
        if (at == source) break;
        at = cameFrom[at];
    }
    std::reverse(path.begin(), path.end());
    return path;
}

void Network::sendPacket(Packet& packet) {
    NetworkDevice* source = findDeviceByName(packet.getSource());
    NetworkDevice* destination = findDeviceByName(packet.getDestination());
    if (!source || !destination) return;

    std::vector<NetworkDevice*> path = findShortestPathByLatency(source, destination);
    if (path.empty()) return;

    for (size_t i = 1; i < path.size(); ++i) {
        path[i]->receivePacket(packet);
        if (packet.isExpired()) return;
    }
}
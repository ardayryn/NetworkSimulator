#include "Network.h"
#include "Packet.h"
#include "Edge.h"
#include "NetworkDevice.h"
#include <queue>
#include <unordered_set>
#include <algorithm>
#include <utility>
#include <functional>
#include <iostream>

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

void Network::connect(int fromId, int toId, double latency, int capacity, double lossProbability) {
    NetworkDevice* from = findDevice(fromId);
    NetworkDevice* to   = findDevice(toId);
    if (!from || !to) return;

    edges_.push_back(std::make_unique<Edge>(from, to, latency, capacity, lossProbability));

    adjacency_[from].push_back(to);
    adjacency_[to].push_back(from);

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
        return {};
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

        if (currentDist > dist[current]) continue;

        for (const auto& [neighbor, latency] : weightedAdjacency_[current]) {
            double newDist = currentDist + latency;

            if (dist.find(neighbor) == dist.end() || newDist < dist[neighbor]) {
                dist[neighbor] = newDist;
                cameFrom[neighbor] = current;
                pq.push({newDist, neighbor});
            }
        }
    }

    std::vector<NetworkDevice*> path;
    if (destination != source && cameFrom.find(destination) == cameFrom.end()) {
        return {};
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

double Network::getLatencyBetween(NetworkDevice* a, NetworkDevice* b) const {
    auto it = weightedAdjacency_.find(a);
    if (it == weightedAdjacency_.end()) return 0.0;
    for (const auto& [neighbor, latency] : it->second) {
        if (neighbor == b) return latency;
    }
    return 0.0;
}

Edge* Network::getEdgeBetween(NetworkDevice* a, NetworkDevice* b) const {
    for (const auto& edge : edges_) {
        if ((edge->getFrom() == a && edge->getTo() == b) ||
            (edge->getFrom() == b && edge->getTo() == a)) {
            return edge.get();
        }
    }
    return nullptr;
}

void Network::schedulePacket(Packet& packet, double startTime) {
    NetworkDevice* source = findDeviceByName(packet.getSource());
    NetworkDevice* destination = findDeviceByName(packet.getDestination());
    if (!source || !destination) return;

    std::vector<NetworkDevice*> path = findShortestPathByLatency(source, destination);
    if (path.empty()) return;

    double currentTime = startTime;
    for (size_t i = 1; i < path.size(); ++i) {
        Edge* edge = getEdgeBetween(path[i - 1], path[i]);
        if (!edge) return;

        double actualStart = edge->reserveSlot(currentTime);
        currentTime = actualStart + edge->getLatency();

        std::string linkName = path[i - 1]->getName() + " -> " + path[i]->getName();
        TransmissionOutcome outcome = edge->evaluateTransmission();

        if (outcome == TransmissionOutcome::Lost) {
            std::string note = "Packet lost on link " + linkName;
            eventQueue_.push({currentTime, nextSequence_++, &packet, nullptr, note});
            return;
        }

        std::string note;
        switch (outcome) {
            case TransmissionOutcome::Corrupted:
                note = "Warning: data may be corrupted on link " + linkName;
                break;
            case TransmissionOutcome::Recoverable:
                note = "Data recovered on link " + linkName;
                break;
            case TransmissionOutcome::Intact:
                note = "Data transmitted almost intact on link " + linkName;
                break;
            default:
                break;
        }

        eventQueue_.push({currentTime, nextSequence_++, &packet, path[i], note});
    }
}

void Network::runSimulation() {
    while (!eventQueue_.empty()) {
        Event event = eventQueue_.top();
        eventQueue_.pop();

        if (event.packet->isExpired()) continue;

        if (!event.target) {
            std::cout << "[t=" << event.time << "] " << event.note << "\n";
            continue;
        }

        if (!event.note.empty()) {
            std::cout << "[t=" << event.time << "] " << event.note << "\n";
        }
        std::cout << "[t=" << event.time << "] ";
        event.target->receivePacket(*event.packet);
    }
}
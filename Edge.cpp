#include "Edge.h"
#include <algorithm>

Edge::Edge(NetworkDevice* from, NetworkDevice* to, double latency, int capacity)
    : from_(from), to_(to), latency_(latency), capacity_(capacity) {}

NetworkDevice* Edge::getFrom() const { return from_; }
NetworkDevice* Edge::getTo() const { return to_; }
double Edge::getLatency() const { return latency_; }
int Edge::getCapacity() const { return capacity_; }

double Edge::reserveSlot(double requestedTime) {
    if (busyUntil_.size() < static_cast<size_t>(capacity_)) {
        busyUntil_.push(requestedTime + latency_);
        return requestedTime;
    }

    double earliestFree = busyUntil_.top();
    busyUntil_.pop();

    double actualStart = std::max(requestedTime, earliestFree);
    busyUntil_.push(actualStart + latency_);
    return actualStart;
}
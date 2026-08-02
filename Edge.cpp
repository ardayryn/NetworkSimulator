#include "Edge.h"
#include <algorithm>
#include <random>

Edge::Edge(NetworkDevice* from, NetworkDevice* to, double latency,
           int capacity, double lossProbability)
    : from_(from), to_(to), latency_(latency),
      capacity_(capacity), lossProbability_(lossProbability) {}

NetworkDevice* Edge::getFrom() const { return from_; }
NetworkDevice* Edge::getTo() const { return to_; }
double Edge::getLatency() const { return latency_; }
int Edge::getCapacity() const { return capacity_; }
double Edge::getLossProbability() const { return lossProbability_; }

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

TransmissionOutcome Edge::evaluateTransmission() const {
    static std::mt19937 rng(std::random_device{}());
    std::uniform_real_distribution<double> dist(0.0, 1.0);
    double roll = dist(rng);

    if (roll >= 0.8) return TransmissionOutcome::Lost;
    if (roll >= 0.5) return TransmissionOutcome::Corrupted;
    if (roll >= 0.1) return TransmissionOutcome::Recoverable;
    return TransmissionOutcome::Intact;
}
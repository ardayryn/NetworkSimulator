// Edge.cpp
#include "Edge.h"

Edge::Edge(NetworkDevice* from, NetworkDevice* to, double latency)
    : from_(from),
      to_(to),
      latency_(latency)
{
}

NetworkDevice* Edge::getFrom() const {
    return from_;
}

NetworkDevice* Edge::getTo() const {
    return to_;
}

double Edge::getLatency() const {
    return latency_;
}
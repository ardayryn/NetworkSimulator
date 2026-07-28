#pragma once

class NetworkDevice;   // forward declaration (neden aşağıda)

class Edge {
public:
    Edge(NetworkDevice* from, NetworkDevice* to, double latency);

    NetworkDevice* getFrom()    const;
    NetworkDevice* getTo()      const;
    double         getLatency() const;

private:
    NetworkDevice* from_;   // non-owning
    NetworkDevice* to_;     // non-owning
    double         latency_;
};
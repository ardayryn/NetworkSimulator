#include "Packet.h"

Packet::Packet(int id, const std::string& source,
               const std::string& destination,
               const std::string& payload, int ttl)
    : id_(id), source_(source), destination_(destination),
      payload_(payload), ttl_(ttl) {}

int Packet::getId() const { return id_; }
std::string Packet::getSource() const { return source_; }
std::string Packet::getDestination() const { return destination_; }
std::string Packet::getPayload() const { return payload_; }
int Packet::getTTL() const { return ttl_; }

void Packet::decreaseTTL() { if (ttl_ > 0) ttl_--; }
bool Packet::isExpired() const { return ttl_ <= 0; }
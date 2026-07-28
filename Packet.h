#pragma once
#include <string>

class Packet {
public:
    Packet(int id,
           const std::string& source,
           const std::string& destination,
           const std::string& payload,
           int ttl);

    int         getId()          const;
    std::string getSource()      const;
    std::string getDestination() const;
    std::string getPayload()     const;
    int         getTTL()         const;

    void decreaseTTL();
    bool isExpired() const;

private:
    int         id_;
    std::string source_;
    std::string destination_;
    std::string payload_;
    int         ttl_;
};
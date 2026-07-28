#pragma once
#include <string>

class Packet;   // forward declaration

class NetworkDevice {
public:
    NetworkDevice(int id, const std::string& name);
    virtual ~NetworkDevice();                       // (!) virtual destructor

    int         getId()   const;
    std::string getName() const;

    // Her cihaz FARKLI yapar → pure virtual → sınıf abstract olur
    virtual void receivePacket(Packet& packet) = 0;

protected:
    int         id_;
    std::string name_;
};
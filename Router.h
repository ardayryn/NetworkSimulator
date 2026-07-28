// Router.h
#pragma once
#include "NetworkDevice.h"
class Packet;

class Router : public NetworkDevice {
public:
    Router(int id, const std::string& name);
    void receivePacket(Packet& packet) override;
    // routePacket ileride, Network'ün routing tasarımıyla birlikte gelecek
};
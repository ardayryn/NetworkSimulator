// NetworkDevice.cpp
#include "NetworkDevice.h"

NetworkDevice::NetworkDevice(int id, const std::string& name)
    : id_(id),
      name_(name)
{
}

NetworkDevice::~NetworkDevice() {
    // boş — ekstra kaynak yok, ama gövde şart (bildirildi)
}

int NetworkDevice::getId() const {
    return id_;
}

std::string NetworkDevice::getName() const {
    return name_;
}

// receivePacket burada YOK — pure virtual (= 0).
// Onu Computer.cpp ve Router.cpp yazacak.
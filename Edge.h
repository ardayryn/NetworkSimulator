#pragma once
#include <queue>
#include <vector>
#include <functional>
class NetworkDevice;

// Bir paketin bir bağlantıdan geçişinin dört olası sonucu (rastgele belirlenir)
enum class TransmissionOutcome { Lost, Corrupted, Recoverable, Intact };

class Edge {
public:
    Edge(NetworkDevice* from, NetworkDevice* to, double latency,
         int capacity = 1, double lossProbability = 0.0);

    NetworkDevice* getFrom() const;
    NetworkDevice* getTo() const;
    double getLatency() const;
    int getCapacity() const;
    double getLossProbability() const;

    double reserveSlot(double requestedTime);

    // Rastgele bir sayı çekip [0,1) aralığında, sonucu dört kademeye ayırır:
    // roll >= 0.8        -> Lost         (kayboldu)
    // 0.5 <= roll < 0.8   -> Corrupted    (veri yanlış olabilir)
    // 0.1 <= roll < 0.5   -> Recoverable  (kurtarılabilir)
    // roll < 0.1          -> Intact       (neredeyse eksiksiz)
    TransmissionOutcome evaluateTransmission() const;

private:
    NetworkDevice* from_;
    NetworkDevice* to_;
    double latency_;
    int capacity_;
    double lossProbability_;

    std::priority_queue<double, std::vector<double>, std::greater<double>> busyUntil_;
};
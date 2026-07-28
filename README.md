# Network Simulator

Basit bir ağ simülatörü — C++ OOP tasarım prensipleri üzerine kurulu.

## Mimari

- **Packet** — taşınan veri (immutable). id, source, destination, payload, ttl.
- **Edge** — iki cihaz arasındaki bağlantı. Cihazları *gösterir* (non-owning raw pointer), sahiplenmez. Latency bilgisini tutar.
- **NetworkDevice** — soyut temel sınıf (abstract). Ortak davranış + pure virtual `receivePacket`.
- **Computer** / **Router** — NetworkDevice'tan türer, `receivePacket`'i kendi tarzında override eder.
- **Network** — ağın sahibi. Cihazları ve edge'leri `unique_ptr` ile sahiplenir, routing'i yönetir.

## Tasarım kararları

| İlişki | Tip | Anlam |
|--------|-----|-------|
| Network → cihazlar/edge'ler | `unique_ptr` (sahiplik) | "yaşam süresini ben yönetirim" |
| Edge → cihazlar | raw pointer (gösterir) | "sadece bakarım" |
| Computer/Router → Packet | dependency | "kullanırım" |

## Routing

Network iki farklı yol bulma algoritması destekliyor:

- **BFS** (`findPath`) — en az hop sayılı yolu bulur. Ağırlıksız komşuluk listesi (`adjacency_`) kullanır.
- **Dijkstra** (`findShortestPathByLatency`) — toplam latency'si en düşük yolu bulur. Ağırlıklı komşuluk listesi (`weightedAdjacency_`) ve min-heap (`priority_queue`) kullanır.

`sendPacket` şu anda Dijkstra'yı kullanıyor — yani paket her zaman **en düşük toplam gecikmeli** yoldan gönderiliyor, hop sayısı az olsa bile daha yavaş bir yol varsa onu tercih etmez.

Her hop'ta paketin TTL'si azalır (`decreaseTTL`); TTL sıfırlanırsa paket düşer (`isExpired`) ve yolculuk durur.

## Derleme

```bash
g++ -std=c++17 *.cpp -o network_sim
./network_sim
```

## Test senaryoları

**Dijkstra doğrulaması** — kısa ama yavaş yol (latency 100+100) ile uzun ama hızlı yol (latency 1+1+1) arasında, Dijkstra hızlı olanı seçiyor:
# Network Simulator

C++ ile yazılmış, OOP tasarım prensipleri üzerine kurulu basit bir ağ simülatörü. BFS ve Dijkstra ile routing, TTL kontrolü, ve discrete event simulation ile gerçek zamanlı çoklu paket desteği içerir.

## Mimari

- **Packet** — taşınan veri (immutable). id, source, destination, payload, ttl.
- **Edge** — iki cihaz arasındaki bağlantı. Cihazları *gösterir* (non-owning raw pointer), sahiplenmez. Latency bilgisini tutar.
- **NetworkDevice** — soyut temel sınıf (abstract). Ortak davranış + pure virtual `receivePacket`.
- **Computer** / **Router** — NetworkDevice'tan türer, `receivePacket`'i kendi tarzında override eder.
- **Network** — ağın sahibi. Cihazları ve edge'leri `unique_ptr` ile sahiplenir, routing ve simülasyonu yönetir.

## Tasarım kararları

| İlişki | Tip | Anlam |
|--------|-----|-------|
| Network → cihazlar/edge'ler | `unique_ptr` (sahiplik) | "yaşam süresini ben yönetirim" |
| Edge → cihazlar | raw pointer (gösterir) | "sadece bakarım" |
| Computer/Router → Packet | dependency | "kullanırım" |
| Event → Packet | raw pointer (gösterir) | "aynı gerçek paketi işaret eder, kopyalamaz" |

## Routing

Network iki farklı yol bulma algoritması destekliyor:

- **BFS** (`findPath`) — en az hop sayılı yolu bulur. Ağırlıksız komşuluk listesi (`adjacency_`) kullanır.
- **Dijkstra** (`findShortestPathByLatency`) — toplam latency'si en düşük yolu bulur. Ağırlıklı komşuluk listesi (`weightedAdjacency_`) ve min-heap (`priority_queue`) kullanır.

Her hop'ta paketin TTL'si azalır (`decreaseTTL`); TTL sıfırlanırsa paket düşer (`isExpired`) ve yolculuk durur.

## Gerçek zamanlı simülasyon (discrete event simulation)

`sendPacket` paketi anında baştan sona teslim eder — tek paketlik hızlı testler için uygundur, ama paketler birbirinden bağımsız zamanlarda hareket edemez.

Gerçek zamanlı ve çoklu paket senaryoları için:

- **`schedulePacket(packet, startTime)`** — Dijkstra ile yolu bulur, her hop için latency'ye göre kümülatif bir zaman hesaplar, ve `"şu zamanda, şu pakete, şu cihaza ulaş"` bilgisini bir `Event` olarak `eventQueue_`'ya ekler. Hiçbir şeyi anında yapmaz, sadece planlar.
- **`runSimulation()`** — `eventQueue_`'yu (min-heap, `EventComparator` ile zamana göre sıralı) boşalana kadar işler; her adımda **en erken zamanlı** event'i çeker. Böylece birden fazla paket, gönderilme sırasına değil, **gerçek varış zamanlarına göre** iç içe işlenir — daha geç yola çıkan ama daha kısa yoldan giden bir paket, daha erken yola çıkan ama uzun yoldan giden bir paketi geçebilir.

`Event` içindeki `Packet*` bilerek **pointer** (kopya değil): aynı paketin farklı hop'ları için üretilen event'lerin hepsi, TTL gibi durumu paylaşan **tek bir gerçek nesneye** işaret etmeli — kopyalansaydı her event kendi bağımsız TTL'ine sahip olur, mekanizma bozulurdu.

**Eşit zamanlı olaylar (FIFO garantisi):** `priority_queue` sadece `time` değerine göre sıralasaydı, iki event'in zamanı eşit olduğunda hangisinin önce işleneceği C++ standardında tanımsız kalırdı (heap'in iç yapısına bağlı, çalıştırmadan çalıştırmaya değişebilir). Bunu önlemek için her `Event`'e artan bir `sequence` numarası veriliyor (`nextSequence_`); `EventComparator` önce zamana, zamanlar eşitse `sequence`'a bakıyor. Sonuç: eşit zamanlı iki event varsa, **kod akışında önce schedule edilen her zaman önce işlenir** — deterministik, tahmin edilebilir bir sıralama.

## Derleme

Tüm `.h`/`.cpp` dosyaları aynı klasördeyse:
```bash
g++ -std=c++17 *.cpp -o network_sim
./network_sim
```

## Test senaryoları

**Dijkstra doğrulaması** — kısa ama yavaş yol (latency 100+100) ile uzun ama hızlı yol (latency 1+1+1) arasında, Dijkstra hızlı olanı seçiyor:
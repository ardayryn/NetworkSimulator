# Network Simulator

C++ ile yazılmış, OOP tasarım prensipleri üzerine kurulu bir ağ simülatörü. BFS ve Dijkstra ile routing, TTL kontrolü, discrete event simulation ile gerçek zamanlı çoklu paket desteği, bant genişliği kısıtı ve olasılıksal paket kaybı/bozulma simülasyonu içerir.

## Mimari

- **Packet** — taşınan veri (immutable). id, source, destination, payload, ttl.
- **Edge** — iki cihaz arasındaki bağlantı. Cihazları *gösterir* (non-owning raw pointer), sahiplenmez. Latency, kapasite ve meşgul-slot bilgisini tutar.
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

- **`schedulePacket(packet, startTime)`** — Dijkstra ile yolu bulur, her hop için latency, bant genişliği kısıtı ve transmisyon kalitesine göre kümülatif bir zaman hesaplar, ve sonucu `Event` olarak `eventQueue_`'ya ekler. Hiçbir şeyi anında yapmaz, sadece planlar.
- **`runSimulation()`** — `eventQueue_`'yu (min-heap, `EventComparator` ile zamana göre sıralı) boşalana kadar işler; her adımda **en erken zamanlı** event'i çeker. Böylece birden fazla paket, gönderilme sırasına değil, **gerçek varış zamanlarına göre** iç içe işlenir.

`Event` içindeki `Packet*` bilerek **pointer** (kopya değil): aynı paketin farklı hop'ları için üretilen event'lerin hepsi, TTL gibi durumu paylaşan **tek bir gerçek nesneye** işaret etmeli — kopyalansaydı her event kendi bağımsız TTL'ine sahip olur, mekanizma bozulurdu.

**Önemli:** `schedulePacket`'e verilen `Packet` referansı, `runSimulation()` çağrısı bitene kadar **hayatta kalmalı**. Yerel bir değişkenin adresini event kuyruğuna kaydedip o değişken scope dışına çıkarsa (örn. bir `for` döngüsü içinde), event işlenene kadar paket zaten yok olmuş olur (dangling pointer). Bu yüzden birden fazla paket schedule edilecekse, hepsi `std::vector<std::unique_ptr<Packet>>` gibi runSimulation() bitene kadar yaşayacak bir kapta tutulmalı.

**Eşit zamanlı olaylar (FIFO garantisi):** `priority_queue` sadece `time` değerine göre sıralasaydı, iki event'in zamanı eşit olduğunda hangisinin önce işleneceği C++ standardında tanımsız kalırdı. Bunu önlemek için her `Event`'e artan bir `sequence` numarası veriliyor (`nextSequence_`); `EventComparator` önce zamana, zamanlar eşitse `sequence`'a bakıyor. Sonuç: eşit zamanlı iki event varsa, kod akışında önce schedule edilen her zaman önce işlenir.

## Bant genişliği kısıtı

Her `Edge`, aynı anda kaç paket taşıyabileceğini (`capacity_`, varsayılan 1) ve meşgul slotların ne zaman boşalacağını (`busyUntil_`, min-heap) tutar.

- **`Edge::reserveSlot(requestedTime)`** — boş slot varsa paket hemen `requestedTime`'da başlar. Kapasite doluysa, en erken boşalan slotu (`busyUntil_.top()`) bulur ve paketin başlangıcını `std::max(requestedTime, earliestFree)` ile erteler — yani kapasite doluyken gelen paket, önceki paketin bitmesini bekler.

`connect(fromId, toId, latency, capacity)` ile her bağlantının kapasitesi ayrı ayrı ayarlanabilir.

## Paket kaybı ve bozulma (olasılıksal transmisyon)

Her `Edge::evaluateTransmission()` çağrısı, `[0,1)` aralığında rastgele bir sayı çekip (`std::mt19937` + `std::uniform_real_distribution`) dört olası sonuçtan birini döndürür:

| Rastgele değer aralığı | Sonuç (`TransmissionOutcome`) | Anlamı |
|---|---|---|
| `roll >= 0.8` | `Lost` | Paket kayboldu, yolculuk burada durur |
| `0.5 <= roll < 0.8` | `Corrupted` | Veri bozulmuş olabilir, ama iletilmeye devam eder |
| `0.1 <= roll < 0.5` | `Recoverable` | Veri kurtarılabilir düzeyde, iletilmeye devam eder |
| `roll < 0.1` | `Intact` | Veri neredeyse eksiksiz iletildi |

Sadece `Lost` durumu paketin yolculuğunu durdurur — diğer üç durumda paket hedefine doğru ilerlemeye devam eder, sadece bir uyarı notu ile. Her hop'ta bu zar bağımsız olarak atılır; aynı paket bir bağlantıda "corrupted" çıkıp bir sonrakinde "lost" olabilir.

## Derleme

Tüm `.h`/`.cpp` dosyaları aynı klasördeyse:
```bash
g++ -std=c++17 *.cpp -o network_sim
./network_sim
```

## Test senaryoları

**Dijkstra doğrulaması** — kısa ama yavaş yol ile uzun ama hızlı yol arasında, Dijkstra hızlı olanı seçiyor.

**TTL doğrulaması** — 5 hop'luk bir yolda, TTL=2 olan bir paket 2. router'da düşüyor.

**Çoklu paket / gerçek zamanlı simülasyon** — geç başlayan ama kısa yoldan giden bir paket, erken başlayan ama uzun yoldan giden bir paketi geçiyor.

**Bant genişliği** — kapasitesi 1 olan bir bağlantıda, aynı anda gönderilen iki paketten biri diğerinin bitmesini bekliyor:
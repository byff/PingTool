#include "HostStat.h"
#include <algorithm>

HostStat::HostStat() : ip(0), ring(RING_SIZE, 0) {}

HostStat::HostStat(const std::string& host, uint32_t ip) 
    : host(host), ip(ip), ring(RING_SIZE, 0) {}

void HostStat::update(const PingResult& result) {
    sent.fetch_add(1, std::memory_order_relaxed);
    
    if (result.success) {
        recv.fetch_add(1, std::memory_order_relaxed);
        
        uint32_t rtt = result.rtt_ms;
        sum_ms.fetch_add(rtt, std::memory_order_relaxed);
        
        uint32_t current_min = min_ms.load(std::memory_order_relaxed);
        while (rtt < current_min && !min_ms.compare_exchange_weak(current_min, rtt,
            std::memory_order_relaxed, std::memory_order_relaxed)) {}
        
        uint32_t current_max = max_ms.load(std::memory_order_relaxed);
        while (rtt > current_max && !max_ms.compare_exchange_weak(current_max, rtt,
            std::memory_order_relaxed, std::memory_order_relaxed)) {}
        
        size_t pos = ring_pos.fetch_add(1, std::memory_order_relaxed) % RING_SIZE;
        ring[pos] = rtt;
    } else {
        fail.fetch_add(1, std::memory_order_relaxed);
    }
}

double HostStat::get_avg_ms() const {
    uint32_t total = recv.load(std::memory_order_relaxed);
    if (total == 0) return 0.0;
    uint64_t sum = sum_ms.load(std::memory_order_relaxed);
    return static_cast<double>(sum) / total;
}

double HostStat::get_fail_rate() const {
    uint32_t s = sent.load(std::memory_order_relaxed);
    if (s == 0) return 0.0;
    uint32_t f = fail.load(std::memory_order_relaxed);
    return static_cast<double>(f) / s * 100.0;
}

void HostStat::reset() {
    sent.store(0, std::memory_order_relaxed);
    recv.store(0, std::memory_order_relaxed);
    fail.store(0, std::memory_order_relaxed);
    sum_ms.store(0, std::memory_order_relaxed);
    min_ms.store(UINT32_MAX, std::memory_order_relaxed);
    max_ms.store(0, std::memory_order_relaxed);
    ring_pos.store(0, std::memory_order_relaxed);
    std::fill(ring.begin(), ring.end(), 0);
    seq = 0;
}

std::vector<uint32_t> HostStat::snapshot_rtt_history() const {
    std::vector<uint32_t> result;
    result.reserve(RING_SIZE);
    size_t pos = ring_pos.load(std::memory_order_relaxed);
    size_t count = std::min(static_cast<size_t>(recv.load(std::memory_order_relaxed)), RING_SIZE);
    for (size_t i = 0; i < count; ++i) {
        size_t idx = (pos - count + i) % RING_SIZE;
        result.push_back(ring[idx]);
    }
    return result;
}
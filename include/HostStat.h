#pragma once
#ifndef HOSTSTAT_H
#define HOSTSTAT_H

#include <atomic>
#include <cstdint>
#include <mutex>
#include <string>
#include <vector>

struct PingResult {
    bool success;
    uint32_t rtt_ms;
    uint32_t status;
};

struct HostStat {
    std::string host;
    std::string hostname_resolved;
    uint32_t ip;
    std::atomic<uint32_t> sent{0};
    std::atomic<uint32_t> recv{0};
    std::atomic<uint32_t> fail{0};
    std::atomic<uint64_t> sum_ms{0};
    std::atomic<uint32_t> min_ms{UINT32_MAX};
    std::atomic<uint32_t> max_ms{0};
    std::vector<uint32_t> ring;
    std::atomic<uint32_t> ring_pos{0};
    uint32_t seq{0};
    std::atomic<bool> enabled{true};
    std::mutex mtx;

    static constexpr size_t RING_SIZE = 64;

    HostStat();
    explicit HostStat(const std::string& host, uint32_t ip = 0);
    void update(const PingResult& result);
    double get_avg_ms() const;
    double get_fail_rate() const;
    void reset();
    std::vector<uint32_t> snapshot_rtt_history() const;
};

#endif
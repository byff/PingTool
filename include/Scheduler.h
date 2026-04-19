#pragma once
#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "HostStat.h"
#include "IcmpBackend.h"
#include <atomic>
#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

struct PingConfig {
    uint32_t timeout_ms = 1000;
    uint16_t payload_size = 32;
    uint32_t interval_ms = 1000;
    uint32_t max_concurrency = 200;
    uint32_t thread_count = 8;
    uint32_t poll_granularity_ms = 20;
};

class Scheduler {
public:
    using HostMap = std::vector<std::shared_ptr<HostStat>>;
    using UpdateCallback = std::function<void(std::shared_ptr<HostStat>, const PingResult&)>;

    Scheduler();
    ~Scheduler();

    void start(HostMap& hosts, std::shared_ptr<IPingBackend> backend, UpdateCallback callback);
    void stop();
    bool is_running() const;
    void update_config(const PingConfig& cfg);
    PingConfig get_config() const;

private:
    void schedule_loop();
    void worker_loop();

    std::atomic<bool> running{false};
    std::atomic<bool> stop_requested{false};
    PingConfig config;
    
    std::vector<std::thread> worker_threads;
    std::thread scheduler_thread;
    
    std::mutex host_mutex;
    std::condition_variable host_cv;
    std::queue<size_t> pending_hosts;
    
    HostMap* hosts_ptr = nullptr;
    std::shared_ptr<IPingBackend> backend;
    UpdateCallback update_callback;
};

#endif
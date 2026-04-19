#include "Scheduler.h"
#include <chrono>
#include <algorithm>

Scheduler::Scheduler() {}

Scheduler::~Scheduler() {
    stop();
}

void Scheduler::start(HostMap& hosts, std::shared_ptr<IPingBackend> backend, UpdateCallback callback) {
    if (running.load()) return;
    
    stop_requested.store(false);
    running.store(true);
    hosts_ptr = &hosts;
    this->backend = std::move(backend);
    this->update_callback = std::move(callback);
    
    worker_threads.reserve(config.thread_count);
    for (uint32_t i = 0; i < config.thread_count; ++i) {
        worker_threads.emplace_back([this] { worker_loop(); });
    }
    
    scheduler_thread = std::thread([this] { schedule_loop(); });
}

void Scheduler::stop() {
    if (!running.load()) return;
    
    stop_requested.store(true);
    running.store(false);
    
    host_cv.notify_all();
    
    if (scheduler_thread.joinable()) {
        scheduler_thread.join();
    }
    
    for (auto& t : worker_threads) {
        if (t.joinable()) t.join();
    }
}

bool Scheduler::is_running() const {
    return running.load();
}

void Scheduler::update_config(const PingConfig& cfg) {
    std::lock_guard<std::mutex> lock(host_mutex);
    config = cfg;
}

PingConfig Scheduler::get_config() const {
    return config;
}

void Scheduler::schedule_loop() {
    using namespace std::chrono;
    
    auto next_tick = steady_clock::now();
    size_t host_count = 0;
    
    {
        std::lock_guard<std::mutex> lock(host_mutex);
        if (hosts_ptr) host_count = hosts_ptr->size();
    }
    
    std::vector<size_t> indices(host_count);
    std::iota(indices.begin(), indices.end(), 0);
    
    while (!stop_requested.load()) {
        auto now = steady_clock::now();
        
        if (now >= next_tick) {
            next_tick = now + milliseconds(config.poll_granularity_ms);
            
            std::lock_guard<std::mutex> lock(host_mutex);
            if (hosts_ptr && !hosts_ptr->empty()) {
                for (size_t idx : indices) {
                    if (idx >= hosts_ptr->size()) continue;
                    auto& host = hosts_ptr->at(idx);
                    if (!host || !host->enabled.load()) continue;
                    
                    pending_hosts.push(idx);
                }
                host_cv.notify_all();
            }
        }
        
        std::this_thread::sleep_for(milliseconds(1));
    }
}

void Scheduler::worker_loop() {
    while (!stop_requested.load()) {
        size_t host_idx;
        
        {
            std::unique_lock<std::mutex> lock(host_mutex);
            
            host_cv.wait(lock, [this] { 
                return !pending_hosts.empty() || stop_requested.load(); 
            });
            
            if (stop_requested.load()) break;
            if (pending_hosts.empty()) continue;
            
            host_idx = pending_hosts.front();
            pending_hosts.pop();
        }
        
        std::shared_ptr<HostStat> host;
        {
            std::lock_guard<std::mutex> lock(host_mutex);
            if (host_idx >= hosts_ptr->size()) continue;
            host = hosts_ptr->at(host_idx);
        }
        
        if (!host || !host->enabled.load()) continue;
        
        uint16_t seq = ++host->seq;
        uint32_t ip = host->ip;
        
        PingResult result = backend->ping_once(
            ip,
            config.timeout_ms,
            config.payload_size,
            seq
        );
        
        host->update(result);
        
        if (update_callback) {
            update_callback(host, result);
        }
    }
}
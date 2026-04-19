#pragma once
#ifndef ICMPBACKEND_H
#define ICMPBACKEND_H

#include "HostStat.h"
#include <memory>
#include <cstdint>

class IPingBackend {
public:
    virtual ~IPingBackend() = default;
    virtual PingResult ping_once(uint32_t ip, uint32_t timeout_ms, uint16_t payload_size, uint16_t seq) = 0;
    virtual bool is_valid() const = 0;
};

class WinIcmpBackend : public IPingBackend {
public:
    WinIcmpBackend();
    ~WinIcmpBackend() override;
    
    PingResult ping_once(uint32_t ip, uint32_t timeout_ms, uint16_t payload_size, uint16_t seq) override;
    bool is_valid() const override;

private:
    void* hIcmp;
    bool valid;
};

std::unique_ptr<IPingBackend> create_ping_backend();

#endif
#include "IcmpBackend.h"
#include <windows.h>
#include <iphlpapi.h>
#include <icmpapi.h>
#include <cstring>

WinIcmpBackend::WinIcmpBackend() : hIcmp(INVALID_HANDLE_VALUE), valid(false) {
    hIcmp = IcmpCreateFile();
    valid = (hIcmp != INVALID_HANDLE_VALUE);
}

WinIcmpBackend::~WinIcmpBackend() {
    if (hIcmp != INVALID_HANDLE_VALUE) {
        IcmpCloseHandle(hIcmp);
        hIcmp = INVALID_HANDLE_VALUE;
    }
}

bool WinIcmpBackend::is_valid() const {
    return valid;
}

PingResult WinIcmpBackend::ping_once(uint32_t ip, uint32_t timeout_ms, uint16_t payload_size, uint16_t seq) {
    PingResult result = {false, 0, 0};
    
    if (!valid || hIcmp == INVALID_HANDLE_VALUE) {
        result.status = static_cast<uint32_t>(-1);
        return result;
    }

    DWORD reply_size = sizeof(ICMP_ECHO_REPLY) + payload_size + 8;
    std::vector<char> reply_buffer(reply_size);
    
    char send_data[64] = {0};
    std::memset(send_data, 0x41, payload_size);
    *reinterpret_cast<uint16_t*>(send_data) = seq;

    IP_OPTION_INFORMATION options = {};
    options.Ttl = 64;
    options.Flags = 0;
    options.OptionsSize = 0;
    options.OptionsData = nullptr;

    LARGE_INTEGER freq, start, end;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&start);

    DWORD ret = IcmpSendEcho2(hIcmp, nullptr, nullptr, nullptr,
                              ip, send_data, payload_size,
                              &options, reply_buffer.data(), reply_size, timeout_ms);

    QueryPerformanceCounter(&end);

    if (ret > 0) {
        PICMP_ECHO_REPLY reply = reinterpret_cast<PICMP_ECHO_REPLY>(reply_buffer.data());
        result.status = reply->Status;
        
        if (reply->Status == IP_SUCCESS || reply->Status == IP_TTL_EXPIRED_TRANSIT) {
            result.success = true;
            result.rtt_ms = reply->RoundTripTime;
        } else {
            result.success = false;
        }
    } else {
        result.status = GetLastError();
        result.success = false;
    }

    return result;
}

std::unique_ptr<IPingBackend> create_ping_backend() {
    return std::make_unique<WinIcmpBackend>();
}
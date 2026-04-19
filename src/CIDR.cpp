#include "CIDR.h"
#include <algorithm>
#include <cctype>
#include <sstream>
#include <stdexcept>

bool parse_ipv4(const std::string& str, uint32_t& out_ip) {
    if (str.empty()) return false;
    
    uint32_t ip = 0;
    uint32_t parts[4] = {0};
    int part_idx = 0;
    
    std::stringstream ss(str);
    std::string token;
    
    while (std::getline(ss, token, '.') && part_idx < 4) {
        if (token.empty()) return false;
        for (char c : token) {
            if (!std::isdigit(c)) return false;
        }
        try {
            size_t pos;
            long val = std::stol(token, &pos);
            if (val < 0 || val > 255 || pos != token.size()) return false;
            parts[part_idx++] = static_cast<uint32_t>(val);
        } catch (...) {
            return false;
        }
    }
    
    if (part_idx != 4) return false;
    
    ip = (parts[0] << 24) | (parts[1] << 16) | (parts[2] << 8) | parts[3];
    out_ip = ip;
    return true;
}

bool is_valid_ipv4(const std::string& str) {
    uint32_t ip;
    return parse_ipv4(str, ip);
}

bool parse_cidr(const std::string& str, CIDRResult& out_result) {
    auto slash_pos = str.find('/');
    std::string ip_str, prefix_len_str;
    
    if (slash_pos == std::string::npos) {
        ip_str = str;
        prefix_len_str = "32";
    } else {
        ip_str = str.substr(0, slash_pos);
        prefix_len_str = str.substr(slash_pos + 1);
    }
    
    uint32_t ip;
    if (!parse_ipv4(ip_str, ip)) return false;
    
    if (prefix_len_str.empty()) return false;
    for (char c : prefix_len_str) {
        if (!std::isdigit(c)) return false;
    }
    
    try {
        size_t pos;
        long prefix = std::stol(prefix_len_str, &pos);
        if (prefix < 0 || prefix > 32 || pos != prefix_len_str.size()) return false;
        
        out_result.ip = ip;
        out_result.mask = (prefix == 0) ? 0 : (~0u << (32 - prefix));
        out_result.network = ip & out_result.mask;
        out_result.broadcast = out_result.network | ~out_result.mask;
        out_result.count = (1u << (32 - prefix));
        
        return true;
    } catch (...) {
        return false;
    }
}

std::vector<uint32_t> expand_cidr(const CIDRResult& cidr, size_t max_expand) {
    std::vector<uint32_t> result;
    
    if (cidr.count > max_expand) {
        return result;
    }
    
    result.reserve(cidr.count);
    
    for (uint32_t i = 0; i < cidr.count; ++i) {
        uint32_t ip = cidr.network + i;
        if (ip != cidr.network && ip != cidr.broadcast) {
            result.push_back(ip);
        }
    }
    
    return result;
}

std::string ip_to_string(uint32_t ip) {
    char buf[16];
    std::snprintf(buf, sizeof(buf), "%u.%u.%u.%u",
                  (ip >> 24) & 0xFF,
                  (ip >> 16) & 0xFF,
                  (ip >> 8) & 0xFF,
                  ip & 0xFF);
    return std::string(buf);
}
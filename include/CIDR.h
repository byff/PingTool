#pragma once
#ifndef CIDR_H
#define CIDR_H

#include <cstdint>
#include <string>
#include <vector>

struct CIDRResult {
    uint32_t ip;
    uint32_t mask;
    uint32_t network;
    uint32_t broadcast;
    uint32_t count;
};

bool parse_ipv4(const std::string& str, uint32_t& out_ip);
bool parse_cidr(const std::string& str, CIDRResult& out_result);
std::vector<uint32_t> expand_cidr(const CIDRResult& cidr, size_t max_expand = 1024);
std::string ip_to_string(uint32_t ip);
bool is_valid_ipv4(const std::string& str);

#endif
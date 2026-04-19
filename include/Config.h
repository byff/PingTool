#pragma once
#ifndef CONFIG_H
#define CONFIG_H

#include "Scheduler.h"
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

struct AppConfig {
    PingConfig ping;
    std::string last_directory;
    std::vector<std::string> recent_hosts;
    bool remember_hosts = true;
    size_t max_recent_hosts = 50;
    uint32_t cidr_max_expand = 1024;
    std::vector<std::string> export_columns;
    
    static AppConfig default_config();
};

class ConfigManager {
public:
    ConfigManager();
    
    bool load(const std::string& path);
    bool save(const std::string& path) const;
    AppConfig get_config() const;
    void update_config(const AppConfig& config);
    std::string get_config_path() const;

private:
    AppConfig config;
    std::string config_path;
};

using json = nlohmann::json;

#endif
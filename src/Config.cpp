#include "Config.h"
#include <fstream>
#include <windows.h>

AppConfig AppConfig::default_config() {
    AppConfig cfg;
    cfg.ping.timeout_ms = 1000;
    cfg.ping.payload_size = 32;
    cfg.ping.interval_ms = 1000;
    cfg.ping.max_concurrency = 200;
    cfg.ping.thread_count = 4;
    cfg.ping.poll_granularity_ms = 20;
    cfg.remember_hosts = true;
    cfg.cidr_max_expand = 1024;
    cfg.export_columns = {"host", "ip", "sent", "recv", "fail", "fail_rate", "avg_ms", "min_ms", "max_ms"};
    return cfg;
}

ConfigManager::ConfigManager() : config(AppConfig::default_config()) {
    char appdata[MAX_PATH];
    if (GetEnvironmentVariableA("APPDATA", appdata, MAX_PATH) > 0) {
        config_path = std::string(appdata) + "\\PingTool\\config.json";
    } else {
        config_path = "config.json";
    }
}

bool ConfigManager::load(const std::string& path) {
    std::string load_path = path.empty() ? config_path : path;
    
    std::ifstream file(load_path);
    if (!file.is_open()) {
        config = AppConfig::default_config();
        return false;
    }
    
    try {
        json j;
        file >> j;
        file.close();
        
        if (j.contains("ping")) {
            const auto& p = j["ping"];
            config.ping.timeout_ms = p.value("timeout_ms", 1000);
            config.ping.payload_size = p.value("payload_size", 32);
            config.ping.interval_ms = p.value("interval_ms", 1000);
            config.ping.max_concurrency = p.value("max_concurrency", 200);
            config.ping.thread_count = p.value("thread_count", 4);
            config.ping.poll_granularity_ms = p.value("poll_granularity_ms", 20);
        }
        
        config.last_directory = j.value("last_directory", "");
        config.remember_hosts = j.value("remember_hosts", true);
        config.cidr_max_expand = j.value("cidr_max_expand", 1024);
        config.max_recent_hosts = j.value("max_recent_hosts", 50);
        
        if (j.contains("recent_hosts")) {
            config.recent_hosts.clear();
            for (const auto& h : j["recent_hosts"]) {
                config.recent_hosts.push_back(h.get<std::string>());
            }
        }
        
        if (j.contains("export_columns")) {
            config.export_columns.clear();
            for (const auto& c : j["export_columns"]) {
                config.export_columns.push_back(c.get<std::string>());
            }
        }
        
        return true;
    } catch (const std::exception&) {
        config = AppConfig::default_config();
        return false;
    }
}

bool ConfigManager::save(const std::string& path) const {
    std::string save_path = path.empty() ? config_path : path;
    
    size_t pos = save_path.find_last_of("\\/");
    if (pos != std::string::npos) {
        std::string dir = save_path.substr(0, pos);
        CreateDirectoryA(dir.c_str(), NULL);
    }
    
    try {
        json j;
        j["ping"]["timeout_ms"] = config.ping.timeout_ms;
        j["ping"]["payload_size"] = config.ping.payload_size;
        j["ping"]["interval_ms"] = config.ping.interval_ms;
        j["ping"]["max_concurrency"] = config.ping.max_concurrency;
        j["ping"]["thread_count"] = config.ping.thread_count;
        j["ping"]["poll_granularity_ms"] = config.ping.poll_granularity_ms;
        j["last_directory"] = config.last_directory;
        j["remember_hosts"] = config.remember_hosts;
        j["cidr_max_expand"] = config.cidr_max_expand;
        j["max_recent_hosts"] = config.max_recent_hosts;
        
        for (const auto& h : config.recent_hosts) {
            j["recent_hosts"].push_back(h);
        }
        
        for (const auto& c : config.export_columns) {
            j["export_columns"].push_back(c);
        }
        
        std::ofstream file(save_path);
        if (!file.is_open()) return false;
        
        file << j.dump(2);
        file.close();
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

AppConfig ConfigManager::get_config() const {
    return config;
}

void ConfigManager::update_config(const AppConfig& cfg) {
    config = cfg;
}

std::string ConfigManager::get_config_path() const {
    return config_path;
}
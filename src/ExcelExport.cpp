#include "ExcelExport.h"
#include "CIDR.h"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cctype>

std::vector<ExportColumn> default_columns() {
    return {
        {"host", "Host"},
        {"ip", "IP Address"},
        {"sent", "Sent"},
        {"recv", "Received"},
        {"fail", "Failed"},
        {"fail_rate", "Fail Rate (%)"},
        {"avg_ms", "Avg (ms)"},
        {"min_ms", "Min (ms)"},
        {"max_ms", "Max (ms)"},
        {"last_ms", "Last (ms)"},
        {"status", "Status"}
    };
}

std::vector<ExportColumn> get_all_exportable_columns() {
    return default_columns();
}

std::string ExcelExporter::get_cell_value(const HostStat& host, const std::string& column_name) {
    if (column_name == "host") return host.host;
    if (column_name == "ip") return ip_to_string(host.ip);
    if (column_name == "sent") return std::to_string(host.sent.load());
    if (column_name == "recv") return std::to_string(host.recv.load());
    if (column_name == "fail") return std::to_string(host.fail.load());
    if (column_name == "fail_rate") {
        std::ostringstream ss;
        ss << std::fixed << std::setprecision(2) << host.get_fail_rate();
        return ss.str();
    }
    if (column_name == "avg_ms") {
        double avg = host.get_avg_ms();
        if (avg == 0) return "N/A";
        std::ostringstream ss;
        ss << std::fixed << std::setprecision(2) << avg;
        return ss.str();
    }
    if (column_name == "min_ms") {
        uint32_t min = host.min_ms.load();
        return min == UINT32_MAX ? "N/A" : std::to_string(min);
    }
    if (column_name == "max_ms") {
        uint32_t max = host.max_ms.load();
        return max == 0 ? "N/A" : std::to_string(max);
    }
    if (column_name == "last_ms") {
        auto history = host.snapshot_rtt_history();
        if (history.empty()) return "N/A";
        return std::to_string(history.back());
    }
    if (column_name == "status") {
        return host.enabled.load() ? "Active" : "Disabled";
    }
    return "";
}

std::string ExcelExporter::extract_ip_from_value(const std::string& value) {
    std::string ip;
    std::istringstream iss(value);
    std::string part;
    int dots = 0;
    
    while (std::getline(iss, part, '.')) {
        if (part.empty() || part.size() > 3) continue;
        for (char c : part) {
            if (!std::isdigit(c)) return "";
        }
        int num = std::stoi(part);
        if (num < 0 || num > 255) return "";
        dots++;
    }
    
    if (dots == 4) {
        return value;
    }
    return "";
}

int ExcelExporter::find_ip_column(const std::vector<ExcelColumnInfo>& columns) {
    const char* ip_keywords[] = {"ip", "address", "host", "ipaddress", "ip_address", "ip addr", "主机", "地址", "ip addr"};
    
    for (const auto& col : columns) {
        std::string lower_name = col.name;
        std::transform(lower_name.begin(), lower_name.end(), lower_name.begin(),
                      [](unsigned char c){ return std::tolower(c); });
        
        for (const char* kw : ip_keywords) {
            if (lower_name.find(kw) != std::string::npos) {
                for (const auto& val : col.values) {
                    if (!extract_ip_from_value(val).empty()) {
                        return col.index;
                    }
                }
            }
        }
    }
    
    for (const auto& col : columns) {
        for (const auto& val : col.values) {
            if (!extract_ip_from_value(val).empty()) {
                return col.index;
            }
        }
    }
    
    return -1;
}

std::vector<ExcelColumnInfo> ExcelExporter::read_excel_columns(const std::string& filepath, int max_rows) {
    std::vector<ExcelColumnInfo> result;
    
    std::string lower = filepath;
    std::transform(lower.begin(), lower.end(), lower.begin(),
                  [](unsigned char c){ return std::tolower(c); });
    
    if (lower.find(".csv") != std::string::npos || lower.find(".txt") != std::string::npos) {
        std::ifstream file(filepath);
        if (!file.is_open()) return result;
        
        std::string line;
        int row = 0;
        std::vector<std::vector<std::string>> rows;
        
        while (std::getline(file, line) && row < max_rows + 1) {
            std::vector<std::string> cols;
            std::istringstream iss(line);
            std::string cell;
            
            while (std::getline(iss, cell, ',')) {
                cell.erase(remove(cell.begin(), cell.end(), '"'), cell.end());
                cols.push_back(cell);
            }
            
            if (!cols.empty()) {
                rows.push_back(cols);
            }
            row++;
        }
        
        if (rows.empty()) return result;
        
        size_t col_count = rows[0].size();
        for (size_t i = 0; i < col_count; i++) {
            ExcelColumnInfo info;
            info.index = (int)i;
            info.name = rows[0][i];
            
            for (size_t j = 1; j < rows.size() && j <= (size_t)max_rows; j++) {
                if (i < rows[j].size()) {
                    info.values.push_back(rows[j][i]);
                }
            }
            result.push_back(info);
        }
    }
    
    return result;
}

bool ExcelExporter::export_to_file(const std::string& filepath,
                                   const std::vector<std::shared_ptr<HostStat>>& hosts,
                                   const std::vector<ExportColumn>& columns) {
    std::ofstream file(filepath);
    if (!file.is_open()) return false;
    
    for (size_t i = 0; i < columns.size(); i++) {
        file << "\"" << columns[i].header << "\"";
        if (i < columns.size() - 1) file << ",";
    }
    file << "\n";
    
    for (const auto& host : hosts) {
        for (size_t i = 0; i < columns.size(); i++) {
            file << "\"" << get_cell_value(*host, columns[i].name) << "\"";
            if (i < columns.size() - 1) file << ",";
        }
        file << "\n";
    }
    
    file.close();
    return true;
}

bool ExcelExporter::insert_results(const std::string& source_filepath,
                                   const std::string& dest_filepath,
                                   const std::vector<std::shared_ptr<HostStat>>& hosts,
                                   const std::vector<ExportColumn>& columns) {
    auto source_cols = read_excel_columns(source_filepath);
    if (source_cols.empty()) {
        return export_to_file(dest_filepath, hosts, columns);
    }
    
    int ip_col_idx = find_ip_column(source_cols);
    
    std::ifstream source_file(source_filepath);
    if (!source_file.is_open()) {
        return export_to_file(dest_filepath, hosts, columns);
    }
    
    std::ofstream dest_file(dest_filepath);
    if (!dest_file.is_open()) {
        return false;
    }
    
    std::string line;
    int row = 0;
    std::map<std::string, size_t> ip_to_host_idx;
    
    for (size_t i = 0; i < hosts.size(); i++) {
        ip_to_host_idx[ip_to_string(hosts[i]->ip)] = i;
    }
    
    while (std::getline(source_file, line)) {
        dest_file << line;
        
        if (ip_col_idx >= 0) {
            std::istringstream iss(line);
            std::vector<std::string> cols;
            std::string cell;
            
            while (std::getline(iss, cell, ',')) {
                cell.erase(remove(cell.begin(), cell.end(), '"'), cell.end());
                cols.push_back(cell);
            }
            
            if ((int)cols.size() > ip_col_idx) {
                std::string ip = extract_ip_from_value(cols[ip_col_idx]);
                auto it = ip_to_host_idx.find(ip);
                if (it != ip_to_host_idx.end()) {
                    for (const auto& col : columns) {
                        dest_file << ",\"" << get_cell_value(*hosts[it->second], col.name) << "\"";
                    }
                }
            }
        }
        
        dest_file << "\n";
        row++;
    }
    
    source_file.close();
    dest_file.close();
    return true;
}
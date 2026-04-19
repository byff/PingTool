#pragma once
#ifndef EXCELEXPORT_H
#define EXCELEXPORT_H

#include "HostStat.h"
#include <string>
#include <vector>
#include <map>

struct ExportColumn {
    std::string name;
    std::string header;
};

struct ExcelColumnInfo {
    int index;
    std::string name;
    std::vector<std::string> values;
};

class ExcelExporter {
public:
    ExcelExporter() = default;
    
    bool export_to_file(const std::string& filepath, 
                        const std::vector<std::shared_ptr<HostStat>>& hosts,
                        const std::vector<ExportColumn>& columns);
    
    bool insert_results(const std::string& source_filepath,
                        const std::string& dest_filepath,
                        const std::vector<std::shared_ptr<HostStat>>& hosts,
                        const std::vector<ExportColumn>& columns);
    
    std::vector<ExcelColumnInfo> read_excel_columns(const std::string& filepath, int max_rows = 100);
    int find_ip_column(const std::vector<ExcelColumnInfo>& columns);

private:
    std::string get_cell_value(const HostStat& host, const std::string& column_name);
    bool write_xlsx(const std::string& filepath,
                   const std::vector<std::shared_ptr<HostStat>>& hosts,
                   const std::vector<ExportColumn>& columns,
                   const std::vector<std::vector<std::string>>* insert_data = nullptr,
                   int ip_col = -1);
    
    std::string extract_ip_from_value(const std::string& value);
};

std::vector<ExportColumn> default_columns();
std::vector<ExportColumn> get_all_exportable_columns();

#endif
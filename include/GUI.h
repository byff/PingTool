#pragma once
#ifndef GUI_H
#define GUI_H

#include "HostStat.h"
#include "Scheduler.h"
#include "Config.h"
#include "ExcelExport.h"
#include <memory>
#include <string>
#include <vector>
#include <atomic>
#include <commctrl.h>

struct GUIState {
    std::atomic<bool> running{false};
    std::atomic<bool> pinging{false};
    std::atomic<bool> needs_refresh{false};
    std::string status_message;
    std::string filter_text;
    int selected_host{-1};
    int sort_column{4};
    bool sort_ascending{false};
};

struct ColumnDef {
    const char* name;
    const char* header;
    int width;
    int format;
};

class PingToolApp {
public:
    PingToolApp();
    ~PingToolApp();
    
    bool init(HINSTANCE hInstance, int nCmdShow);
    void run();
    void shutdown();

private:
    static const int COLUMN_COUNT = 12;
    static const ColumnDef columns[COLUMN_COUNT];
    
    void createControls(HWND hwnd);
    void setupListView(HWND hwnd);
    void refreshHostList();
    void updateHostItem(int index);
    void addHostsFromInput(const std::string& input);
    void importFromFile(const std::string& filepath);
    void exportToFile(const std::string& filepath);
    void showSettingsDialog();
    void showAddHostDialog();
    void applyDarkTheme();
    void startPinging();
    void stopPinging();
    
    int getHostSortValue(const std::shared_ptr<HostStat>& host, int column);
    static int CALLBACK sortCallback(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
    
    HINSTANCE hInstance = nullptr;
    HWND hwnd = nullptr;
    HWND hwndList = nullptr;
    HWND hwndInput = nullptr;
    HWND hwndStatus = nullptr;
    
    GUIState state;
    ConfigManager config_manager;
    Scheduler scheduler;
    std::shared_ptr<IPingBackend> backend;
    std::vector<std::shared_ptr<HostStat>> hosts;
    AppConfig app_config;
    std::vector<ExportColumn> export_columns;
    
    HFONT hFont = nullptr;
    
    static PingToolApp* g_pApp;
};

#endif
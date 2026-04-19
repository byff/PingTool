#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define NOSHLWAPI

#ifdef _MSC_VER
#define override __identifier_override
#pragma warning(disable: 4814)
#endif

#include "GUI.h"
#include "CIDR.h"
#include <windows.h>
#include <commctrl.h>
#include <commdlg.h>
#include <shellapi.h>
#include <fstream>
#include <sstream>
#include <algorithm>

#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "comdlg32.lib")

const ColumnDef PingToolApp::columns[] = {
    {"#", "#", 40, LVCFMT_RIGHT},
    {"Host", "Host", 150, LVCFMT_LEFT},
    {"IP", "IP Address", 120, LVCFMT_LEFT},
    {"Hostname", "Hostname", 140, LVCFMT_LEFT},
    {"Sent", "Sent", 60, LVCFMT_RIGHT},
    {"Recv", "Received", 60, LVCFMT_RIGHT},
    {"Fail", "Failed", 60, LVCFMT_RIGHT},
    {"FailRate", "Fail %", 60, LVCFMT_RIGHT},
    {"Avg", "Avg (ms)", 70, LVCFMT_RIGHT},
    {"Min", "Min (ms)", 70, LVCFMT_RIGHT},
    {"Max", "Max (ms)", 70, LVCFMT_RIGHT},
    {"Status", "Status", 70, LVCFMT_CENTER}
};

PingToolApp* PingToolApp::g_pApp = nullptr;

PingToolApp::PingToolApp() {
    export_columns = default_columns();
    g_pApp = this;
}

PingToolApp::~PingToolApp() {
    shutdown();
}

bool PingToolApp::init(HINSTANCE hInstance, int nCmdShow) {
    this->hInstance = hInstance;
    
    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_LISTVIEW_CLASSES | ICC_WIN95_CLASSES | ICC_BAR_CLASSES;
    InitCommonControlsEx(&icex);
    
    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = [](HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) -> LRESULT {
        if (msg == WM_DESTROY) {
            PostQuitMessage(0);
            return 0;
        }
        if (msg == WM_SIZE) {
            if (g_pApp) g_pApp->refreshHostList();
        }
        if (msg == WM_NOTIFY) {
            LPNMHDR pnmh = (LPNMHDR)lParam;
            if (pnmh->code == LVN_COLUMNCLICK && g_pApp) {
                NMLISTVIEW* pLV = (NMLISTVIEW*)lParam;
                int col = pLV->iSubItem;
                if (col >= 0 && col < COLUMN_COUNT) {
                    if (g_pApp->state.sort_column == col) {
                        g_pApp->state.sort_ascending = !g_pApp->state.sort_ascending;
                    } else {
                        g_pApp->state.sort_column = col;
                        g_pApp->state.sort_ascending = false;
                    }
                    g_pApp->refreshHostList();
                }
            }
            if (pnmh->code == NM_RCLICK && g_pApp) {
                int sel = ListView_GetNextItem(g_pApp->hwndList, -1, LVNI_SELECTED);
                if (sel >= 0) {
                    HMENU hMenu = CreatePopupMenu();
                    AppendMenu(hMenu, MF_STRING, 1001, L"Enable/Disable");
                    AppendMenu(hMenu, MF_STRING, 1002, L"Remove");
                    POINT pt;
                    GetCursorPos(&pt);
                    int cmd = TrackPopupMenu(hMenu, TPM_RETURNCMD | TPM_RIGHTBUTTON, pt.x, pt.y, 0, g_pApp->hwnd, nullptr);
                    if (cmd == 1001) {
                        if (sel >= 0 && sel < (int)g_pApp->hosts.size()) {
                            g_pApp->hosts[sel]->enabled = !g_pApp->hosts[sel]->enabled;
                            g_pApp->updateHostItem(sel);
                        }
                    } else if (cmd == 1002) {
                        if (sel >= 0 && sel < (int)g_pApp->hosts.size()) {
                            g_pApp->hosts.erase(g_pApp->hosts.begin() + sel);
                            ListView_DeleteItem(g_pApp->hwndList, sel);
                        }
                    }
                    DestroyMenu(hMenu);
                }
            }
        }
        if (msg == WM_DROPFILES && g_pApp) {
            HDROP hDrop = (HDROP)wParam;
            char filename[MAX_PATH];
            UINT count = DragQueryFileA(hDrop, 0xFFFFFFFF, nullptr, 0);
            if (count > 0) {
                DragQueryFileA(hDrop, 0, filename, sizeof(filename));
                g_pApp->importFromFile(filename);
            }
            DragFinish(hDrop);
        }
        return DefWindowProcW(hWnd, msg, wParam, lParam);
    };
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = CreateSolidBrush(RGB(30, 30, 40));
    wc.lpszClassName = L"PingToolClass";
    RegisterClassExW(&wc);
    
    hwnd = CreateWindowW(wc.lpszClassName, L"PingTool - Multi-Host Ping Utility",
                        WS_OVERLAPPEDWINDOW, 100, 100, 1100, 700,
                        nullptr, nullptr, hInstance, nullptr);
    
    if (!hwnd) return false;
    
    createControls(hwnd);
    
    DragAcceptFiles(hwnd, TRUE);
    
    backend = create_ping_backend();
    config_manager.load("");
    app_config = config_manager.get_config();
    
    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);
    
    state.running = true;
    return true;
}

void PingToolApp::createControls(HWND hwnd) {
    hwndStatus = CreateWindowW(L"msctls_statusbar32", L"Ready",
        WS_CHILD | WS_VISIBLE, 0, 0, 0, 0, hwnd, (HMENU)99, hInstance, nullptr);
    
    hwndInput = CreateWindowW(L"EDIT", L"",
        WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT,
        10, 50, 700, 28, hwnd, (HMENU)200, hInstance, nullptr);
    
    HWND hwndAddBtn = CreateWindowW(L"BUTTON", L"Add",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        720, 48, 60, 28, hwnd, (HMENU)201, hInstance, nullptr);
    
    HWND hwndImportBtn = CreateWindowW(L"BUTTON", L"Import",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        790, 48, 70, 28, hwnd, (HMENU)202, hInstance, nullptr);
    
    HWND hwndExportBtn = CreateWindowW(L"BUTTON", L"Export",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        870, 48, 70, 28, hwnd, (HMENU)203, hInstance, nullptr);
    
    HWND hwndStartBtn = CreateWindowW(L"BUTTON", L"Start",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_FLAT,
        950, 48, 60, 28, hwnd, (HMENU)204, hInstance, nullptr);
    
    hwndList = CreateWindowW(WC_LISTVIEW, L"",
        WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS |
        LVS_NOSORTHEADER | WS_BORDER | WS_HSCROLL | WS_VSCROLL,
        10, 85, 1060, 500, hwnd, (HMENU)300, hInstance, nullptr);
    
    ListView_SetExtendedListViewStyle(hwndList, 
        LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_HEADERDRAGDROP);
    
    setupListView(hwndList);
    
    applyDarkTheme();
}

void PingToolApp::setupListView(HWND hwnd) {
    LVCOLUMN lvc = {};
    lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
    
    for (int i = 0; i < COLUMN_COUNT; i++) {
        lvc.iSubItem = i;
        lvc.pszText = (LPWSTR)columns[i].header;
        lvc.cx = columns[i].width;
        lvc.fmt = columns[i].format;
        ListView_InsertColumn(hwnd, i, &lvc);
    }
}

void PingToolApp::applyDarkTheme() {
    HTHEME hTheme = OpenThemeData(hwnd, L"Explorer");
    if (hTheme) {
        SetWindowTheme(hwndList, L"Explorer", nullptr);
        OpenThemeData(NULL, L"Explorer");
    }
    
    ListView_SetBkColor(hwndList, RGB(25, 25, 35));
    ListView_SetTextBkColor(hwndList, RGB(25, 25, 35));
    ListView_SetTextColor(hwndList, RGB(200, 200, 210));
    
    HWND hHeader = ListView_GetHeader(hwndList);
    if (hHeader) {
        Header_SetBkColor(hHeader, RGB(30, 30, 40));
        Header_SetTextColor(hHeader, RGB(180, 180, 190));
    }
}

void PingToolApp::run() {
    MSG msg;
    while (state.running) {
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                state.running = false;
                break;
            }
            if (msg.message == WM_COMMAND) {
                int id = LOWORD(msg.wParam);
                if (id == 201) {
                    showAddHostDialog();
                } else if (id == 202) {
                    OPENFILENAMEA ofn = {};
                    char filename[MAX_PATH] = "";
                    ofn.lStructSize = sizeof(ofn);
                    ofn.hwndOwner = hwnd;
                    ofn.lpstrFile = filename;
                    ofn.nMaxFile = sizeof(filename);
                    ofn.lpstrFilter = "All Files\0*.*\0Text Files\0*.txt\0CSV Files\0*.csv\0";
                    ofn.Flags = OFN_FILEMUSTEXIST;
                    if (GetOpenFileNameA(&ofn)) {
                        importFromFile(filename);
                    }
                } else if (id == 203) {
                    OPENFILENAMEA ofn = {};
                    char filename[MAX_PATH] = "";
                    ofn.lStructSize = sizeof(ofn);
                    ofn.hwndOwner = hwnd;
                    ofn.lpstrFile = filename;
                    ofn.nMaxFile = sizeof(filename);
                    ofn.lpstrFilter = "Excel Files\0*.xlsx\0";
                    ofn.DefaultExt = "xlsx";
                    ofn.Flags = OFN_OVERWRITEPROMPT;
                    if (GetSaveFileNameA(&ofn)) {
                        exportToFile(filename);
                    }
                } else if (id == 204) {
                    if (state.pinging.load()) {
                        stopPinging();
                    } else {
                        startPinging();
                    }
                } else if (id == 205) {
                    showSettingsDialog();
                }
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        
        if (state.pinging.load() && state.needs_refresh.load()) {
            refreshHostList();
            state.needs_refresh = false;
        }
        
        Sleep(10);
    }
}

void PingToolApp::shutdown() {
    stopPinging();
    state.running = false;
    
    if (hwnd) {
        DestroyWindow(hwnd);
        hwnd = nullptr;
    }
    
    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(WNDCLASSEXW);
    GetClassInfoExW(hInstance, L"PingToolClass", &wc);
    UnregisterClassW(wc.lpszClassName, hInstance);
}

int PingToolApp::getHostSortValue(const std::shared_ptr<HostStat>& host, int column) {
    switch (column) {
        case 0: return (int)(&(*host) - &(*hosts[0]));
        case 1: return (int)(intptr_t)host->host.c_str();
        case 2: return (int)host->ip;
        case 3: return (int)(intptr_t)host->hostname_resolved.c_str();
        case 4: return host->sent.load();
        case 5: return host->recv.load();
        case 6: return host->fail.load();
        case 7: return (int)(host->get_fail_rate() * 100);
        case 8: return (int)(host->get_avg_ms() * 100);
        case 9: return host->min_ms.load() == UINT32_MAX ? INT_MAX : host->min_ms.load();
        case 10: return host->max_ms.load();
        case 11: return host->enabled.load() ? 1 : 0;
        default: return 0;
    }
}

int CALLBACK PingToolApp::sortCallback(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort) {
    int col = g_pApp->state.sort_column;
    bool asc = g_pApp->state.sort_ascending;
    auto& h1 = g_pApp->hosts[lParam1];
    auto& h2 = g_pApp->hosts[lParam2];
    int v1 = g_pApp->getHostSortValue(h1, col);
    int v2 = g_pApp->getHostSortValue(h2, col);
    return asc ? (v1 - v2) : (v2 - v1);
}

void PingToolApp::refreshHostList() {
    if (!hwndList) return;
    
    ListView_DeleteAllItems(hwndList);
    
    LVITEM lvi = {};
    char buf[64];
    
    for (size_t i = 0; i < hosts.size(); i++) {
        auto& host = hosts[i];
        lvi.iItem = i;
        lvi.mask = LVIF_TEXT;
        
        lvi.iSubItem = 0;
        sprintf(buf, "%zu", i + 1);
        lvi.pszText = buf;
        ListView_InsertItem(hwndList, &lvi);
        
        lvi.iSubItem = 1;
        strncpy(buf, host->host.c_str(), sizeof(buf) - 1);
        buf[sizeof(buf) - 1] = 0;
        lvi.pszText = buf;
        ListView_SetItem(hwndList, &lvi);
        
        lvi.iSubItem = 2;
        strncpy(buf, ip_to_string(host->ip).c_str(), sizeof(buf) - 1);
        buf[sizeof(buf) - 1] = 0;
        lvi.pszText = buf;
        ListView_SetItem(hwndList, &lvi);
        
        lvi.iSubItem = 3;
        strncpy(buf, host->hostname_resolved.c_str(), sizeof(buf) - 1);
        buf[sizeof(buf) - 1] = 0;
        lvi.pszText = buf;
        ListView_SetItem(hwndList, &lvi);
        
        lvi.iSubItem = 4;
        sprintf(buf, "%u", host->sent.load());
        lvi.pszText = buf;
        ListView_SetItem(hwndList, &lvi);
        
        lvi.iSubItem = 5;
        sprintf(buf, "%u", host->recv.load());
        lvi.pszText = buf;
        ListView_SetItem(hwndList, &lvi);
        
        lvi.iSubItem = 6;
        sprintf(buf, "%u", host->fail.load());
        lvi.pszText = buf;
        ListView_SetItem(hwndList, &lvi);
        
        lvi.iSubItem = 7;
        sprintf(buf, "%.1f%%", host->get_fail_rate());
        lvi.pszText = buf;
        ListView_SetItem(hwndList, &lvi);
        
        lvi.iSubItem = 8;
        double avg = host->get_avg_ms();
        sprintf(buf, "%.2f", avg);
        lvi.pszText = buf;
        ListView_SetItem(hwndList, &lvi);
        
        lvi.iSubItem = 9;
        uint32_t min = host->min_ms.load();
        sprintf(buf, "%s", min == UINT32_MAX ? "-" : std::to_string(min).c_str());
        lvi.pszText = buf;
        ListView_SetItem(hwndList, &lvi);
        
        lvi.iSubItem = 10;
        uint32_t max = host->max_ms.load();
        sprintf(buf, "%s", max == 0 ? "-" : std::to_string(max).c_str());
        lvi.pszText = buf;
        ListView_SetItem(hwndList, &lvi);
        
        lvi.iSubItem = 11;
        strcpy(buf, host->enabled.load() ? "Active" : "Disabled");
        lvi.pszText = buf;
        ListView_SetItem(hwndList, &lvi);
    }
    
    if (!hosts.empty()) {
        ListView_SortItems(hwndList, sortCallback, 0);
    }
}

void PingToolApp::updateHostItem(int index) {
    if (index < 0 || index >= (int)hosts.size()) return;
    
    char buf[64];
    auto& host = hosts[index];
    LVITEM lvi = {};
    lvi.iItem = index;
    lvi.mask = LVIF_TEXT;
    
    lvi.iSubItem = 4;
    sprintf(buf, "%u", host->sent.load());
    lvi.pszText = buf;
    ListView_SetItem(hwndList, &lvi);
    
    lvi.iSubItem = 5;
    sprintf(buf, "%u", host->recv.load());
    lvi.pszText = buf;
    ListView_SetItem(hwndList, &lvi);
    
    lvi.iSubItem = 6;
    sprintf(buf, "%u", host->fail.load());
    lvi.pszText = buf;
    ListView_SetItem(hwndList, &lvi);
    
    lvi.iSubItem = 7;
    sprintf(buf, "%.1f%%", host->get_fail_rate());
    lvi.pszText = buf;
    ListView_SetItem(hwndList, &lvi);
    
    lvi.iSubItem = 8;
    sprintf(buf, "%.2f", host->get_avg_ms());
    lvi.pszText = buf;
    ListView_SetItem(hwndList, &lvi);
    
    lvi.iSubItem = 11;
    strcpy(buf, host->enabled.load() ? "Active" : "Disabled");
    lvi.pszText = buf;
    ListView_SetItem(hwndList, &lvi);
}

void PingToolApp::startPinging() {
    if (hosts.empty()) return;
    
    auto callback = [this](std::shared_ptr<HostStat> host, const PingResult& result) {
        state.needs_refresh = true;
    };
    
    scheduler.start(hosts, backend, callback);
    state.pinging = true;
    
    SetWindowTextA(GetDlgItem(hwnd, 204), "Stop");
    SendMessage(hwndStatus, SB_SETTEXT, 0, (LPARAM)"Running...");
}

void PingToolApp::stopPinging() {
    scheduler.stop();
    state.pinging = false;
    
    SetWindowTextA(GetDlgItem(hwnd, 204), "Start");
    SendMessage(hwndStatus, SB_SETTEXT, 0, (LPARAM)"Stopped");
}

void PingToolApp::addHostsFromInput(const std::string& input) {
    std::vector<std::string> lines;
    std::stringstream ss(input);
    std::string line;
    
    while (std::getline(ss, line)) {
        line.erase(line.find_last_not_of(" \t\r\n") + 1);
        line.erase(0, line.find_first_not_of(" \t\r\n"));
        
        if (line.empty() || line[0] == '#') continue;
        
        if (line.find(',') != std::string::npos) {
            std::stringstream ls(line);
            std::string part;
            while (std::getline(ls, part, ',')) {
                part.erase(part.find_last_not_of(" \t\r\n") + 1);
                part.erase(0, part.find_first_not_of(" \t\r\n"));
                if (!part.empty()) lines.push_back(part);
            }
        } else {
            lines.push_back(line);
        }
    }
    
    for (const auto& h : lines) {
        CIDRResult cidr;
        uint32_t ip;
        
        if (parse_cidr(h, cidr)) {
            if (cidr.count > app_config.cidr_max_expand) {
                char msg[256];
                sprintf(msg, "CIDR %s expands to %u IPs (max %u). Continue?",
                        h.c_str(), cidr.count, app_config.cidr_max_expand);
                if (MessageBoxA(hwnd, msg, "Warning", MB_YESNO | MB_ICONWARNING) != IDYES) {
                    continue;
                }
            }
            
            auto ips = expand_cidr(cidr, app_config.cidr_max_expand * 2);
            for (uint32_t expanded_ip : ips) {
                auto stat = std::make_shared<HostStat>(ip_to_string(expanded_ip), expanded_ip);
                hosts.push_back(stat);
            }
        } else if (parse_ipv4(h, ip)) {
            auto stat = std::make_shared<HostStat>(h, ip);
            hosts.push_back(stat);
        } else {
            auto stat = std::make_shared<HostStat>(h, 0);
            hosts.push_back(stat);
        }
    }
    
    refreshHostList();
    
    char buf[64];
    sprintf(buf, "Hosts: %zu", hosts.size());
    SendMessage(hwndStatus, SB_SETTEXT, 1, (LPARAM)buf);
}

void PingToolApp::importFromFile(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) return;
    
    std::string content((std::istreambuf_iterator<char>(file)),
                         std::istreambuf_iterator<char>());
    file.close();
    
    addHostsFromInput(content);
    
    size_t pos = filepath.find_last_of("\\/");
    if (pos != std::string::npos) {
        app_config.last_directory = filepath.substr(0, pos);
        config_manager.update_config(app_config);
    }
}

void PingToolApp::exportToFile(const std::string& filepath) {
    ExcelExporter exporter;
    exporter.export_to_file(filepath, hosts, export_columns);
}

void PingToolApp::showSettingsDialog() {
    MessageBoxA(hwnd, "Settings dialog - configure timeout, interval, concurrency, etc.",
                "Settings", MB_OK);
}

void PingToolApp::showAddHostDialog() {
    char buf[4096] = {0};
    GetWindowTextA(hwndInput, buf, sizeof(buf));
    
    if (strlen(buf) > 0) {
        addHostsFromInput(buf);
        SetWindowTextA(hwndInput, "");
    }
}
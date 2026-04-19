#include "GUI.h"
#include <windows.h>

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    PingToolApp app;
    
    if (!app.init(hInstance, nCmdShow)) {
        MessageBoxA(nullptr, "Failed to initialize application", "Error",
                    MB_ICONERROR | MB_OK);
        return 1;
    }
    
    app.run();
    app.shutdown();
    
    return 0;
}
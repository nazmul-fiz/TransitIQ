#include "Common.h"
#include "App.h"
#include "GUIApp.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPSTR lpCmdLine, int /*nCmdShow*/) {
    std::string cmd(lpCmdLine ? lpCmdLine : "");
    if (cmd.find("--cli") != std::string::npos || cmd.find("--console") != std::string::npos) {
        AllocConsole();
        initConsole();
        App app;
        app.run();
        return 0;
    }

    GUIApp guiApp(hInstance);
    if (!guiApp.initialize()) {
        MessageBox(NULL, "Failed to initialize TransitIQ GUI window.", "Error", MB_ICONERROR);
        return 1;
    }
    return guiApp.run();
}

int main(int argc, char* argv[]) {
    HINSTANCE hInstance = GetModuleHandle(NULL);
    for (int i = 1; i < argc; ++i) {
        std::string arg(argv[i]);
        if (arg == "--cli" || arg == "--console") {
            initConsole();
            App app;
            app.run();
            return 0;
        }
    }

    GUIApp guiApp(hInstance);
    if (!guiApp.initialize()) {
        MessageBox(NULL, "Failed to initialize TransitIQ GUI window.", "Error", MB_ICONERROR);
        return 1;
    }
    return guiApp.run();
}

#include "Common.h"
#include "App.h"

int main() {
    // Initialize Windows Console for UTF-8 support and ANSI escape sequences
    initConsole();

    // Start TransitIQ Application
    App app;
    app.run();

    return 0;
}


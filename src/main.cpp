#include "Common.h"
#include "App.h"
#include "QtMainWindow.h"
#include <QApplication>

int main(int argc, char* argv[]) {
    // Check if user requested console terminal mode
    for (int i = 1; i < argc; ++i) {
        std::string arg(argv[i]);
        if (arg == "--cli" || arg == "--console") {
            initConsole();
            App app;
            app.run();
            return 0;
        }
    }

    // Default: Start Qt6 GUI Application
    QApplication app(argc, argv);
    app.setApplicationName("TransitIQ");
    app.setApplicationDisplayName("TransitIQ — Smart Bus Network Explorer");

    QtMainWindow mainWindow;
    mainWindow.show();

    return app.exec();
}

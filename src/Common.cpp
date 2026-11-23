#include "Common.h"

AnimationConfig g_animConfig;

void initConsole() {
#ifdef _WIN32
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut != INVALID_HANDLE_VALUE) {
        DWORD dwMode = 0;
        if (GetConsoleMode(hOut, &dwMode)) {
            dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
            dwMode |= ENABLE_PROCESSED_OUTPUT;
            SetConsoleMode(hOut, dwMode);
        }
    }
    // Set UTF-8 encoding for clean box drawing characters
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif
}

void animatedSleep(int ms) {
    if (g_animConfig.enabled && ms > 0) {
#ifdef _WIN32
        Sleep(ms);
#else
        usleep(ms * 1000);
#endif
    }
}

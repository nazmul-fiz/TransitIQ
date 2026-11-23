#ifndef COMMON_H
#define COMMON_H

#include <iostream>
#include <vector>
#include <string>
#include <queue>
#include <stack>
#include <map>
#include <set>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <memory>

#ifdef _WIN32
#include <windows.h>
#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif
#else
#include <unistd.h>
#endif

// ANSI Color Codes for Modern Terminal Output
namespace Color {
    const std::string RESET       = "\033[0m";
    const std::string BOLD        = "\033[1m";
    const std::string DIM         = "\033[2m";
    const std::string UNDERLINE   = "\033[4m";

    // Regular Colors
    const std::string BLACK       = "\033[30m";
    const std::string RED         = "\033[31m";
    const std::string GREEN       = "\033[32m";
    const std::string YELLOW      = "\033[33m";
    const std::string BLUE        = "\033[34m";
    const std::string MAGENTA     = "\033[35m";
    const std::string CYAN        = "\033[36m";
    const std::string WHITE       = "\033[37m";

    // Bright Colors
    const std::string BRIGHT_RED     = "\033[91m";
    const std::string BRIGHT_GREEN   = "\033[92m";
    const std::string BRIGHT_YELLOW  = "\033[93m";
    const std::string BRIGHT_BLUE    = "\033[94m";
    const std::string BRIGHT_MAGENTA = "\033[95m";
    const std::string BRIGHT_CYAN    = "\033[96m";
    const std::string BRIGHT_WHITE   = "\033[97m";

    // Backgrounds
    const std::string BG_RED      = "\033[41m";
    const std::string BG_GREEN    = "\033[42m";
    const std::string BG_BLUE     = "\033[44m";
    const std::string BG_MAGENTA  = "\033[45m";
    const std::string BG_CYAN     = "\033[46m";
}

// Global Animation Configuration
struct AnimationConfig {
    bool enabled = true;
    int stepDelayMs = 200; // Delay between queue/stack visual steps
    int fastDelayMs = 60;  // Delay for text effects
};

extern AnimationConfig g_animConfig;

// Initialize Windows Console for UTF-8 and ANSI colors
void initConsole();

// Helper sleep function respecting animation toggle
void animatedSleep(int ms);

#endif // COMMON_H

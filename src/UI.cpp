#include "UI.h"

namespace UI {

void clearScreen() {
#ifdef _WIN32
    // Use ANSI escape sequence to clear screen and move cursor to top-left
    std::cout << "\033[2J\033[H" << std::flush;
#else
    std::cout << "\033[2J\033[H" << std::flush;
#endif
}

void pauseScreen() {
    std::cout << "\n" << Color::DIM << "Press Enter to continue..." << Color::RESET;
    std::string dummy;
    std::getline(std::cin, dummy);
}

void printBanner() {
    std::cout << Color::BRIGHT_CYAN << Color::BOLD;
    std::cout << R"(
  ╔═════════════════════════════════════════════════════════════════════════════╗
  ║   ████████╗██████╗  █████╗ ███╗   ██╗███████╗██╗████████╗██╗ ██████╗       ║
  ║   ╚══██╔══╝██╔══██╗██╔══██╗████╗  ██║██╔════╝██║╚══██╔══╝██║██╔═══██╗      ║
  ║      ██║   ██████╔╝███████║██╔██╗ ██║███████╗██║   ██║   ██║██║   ██║      ║
  ║      ██║   ██╔══██╗██╔══██║██║╚██╗██║╚════██║██║   ██║   ██║██║▄▄ ██║      ║
  ║      ██║   ██║  ██║██║  ██║██║ ╚████║███████║██║   ██║   ██║╚██████╔╝      ║
  ║      ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═══╝╚══════╝╚═╝   ╚═╝   ╚═╝ ╚══▀▀═╝       ║
  ║         Smart Bus Network Explorer & Journey Finder (Dhaka Transit)         ║
  ║                  Powered by Breadth-First & Depth-First Search              ║
  ╚═════════════════════════════════════════════════════════════════════════════╝
)" << Color::RESET;
}

void printHeader(const std::string& title) {
    std::cout << "\n" << Color::BRIGHT_BLUE << "┌";
    for (int i = 0; i < 74; ++i) std::cout << "─";
    std::cout << "┐\n│ " << Color::BOLD << Color::BRIGHT_WHITE;

    int pad = 72 - static_cast<int>(title.length());
    int padLeft = pad / 2;
    int padRight = pad - padLeft;

    for (int i = 0; i < padLeft; ++i) std::cout << " ";
    std::cout << title;
    for (int i = 0; i < padRight; ++i) std::cout << " ";

    std::cout << Color::RESET << Color::BRIGHT_BLUE << " │\n└";
    for (int i = 0; i < 74; ++i) std::cout << "─";
    std::cout << "┘" << Color::RESET << "\n\n";
}

void showLoadingAnimation(const std::string& taskName, int durationMs) {
    if (!g_animConfig.enabled) return;

    const char spinner[] = {'|', '/', '-', '\\'};
    int steps = durationMs / 100;
    std::cout << Color::CYAN << "  " << taskName << " " << std::flush;
    for (int i = 0; i < steps; ++i) {
        std::cout << Color::BRIGHT_YELLOW << "[" << spinner[i % 4] << "]" << "\b\b\b" << std::flush;
        animatedSleep(100);
    }
    std::cout << Color::BRIGHT_GREEN << "[Done!]" << Color::RESET << "\n";
}

void displayQueueState(const std::vector<int>& queueElements,
                       int currentPopped,
                       const Graph& graph) {
    std::cout << Color::DIM << "  ┌─── " << Color::RESET
              << Color::BOLD << Color::YELLOW << "BFS FIFO Queue State" << Color::RESET
              << Color::DIM << " ──────────────────────────────────────────────┐" << Color::RESET << "\n";

    std::string poppedName = (graph.getStop(currentPopped) ? graph.getStop(currentPopped)->name : "Unknown");
    std::cout << Color::DIM << "  │ " << Color::RESET
              << Color::BOLD << Color::BRIGHT_GREEN << "Processing (Popped): " << Color::RESET
              << Color::WHITE << poppedName << " (ID: " << currentPopped << ")" << Color::RESET << "\n";

    std::cout << Color::DIM << "  │ " << Color::RESET << Color::CYAN << "Frontier Queue: " << Color::RESET;
    if (queueElements.size() <= 1) {
        std::cout << Color::DIM << "[Empty / Expanding Neighbors]" << Color::RESET;
    } else {
        std::cout << "[";
        for (size_t i = 1; i < queueElements.size(); ++i) {
            int id = queueElements[i];
            std::string sName = (graph.getStop(id) ? graph.getStop(id)->name : std::to_string(id));
            std::cout << sName;
            if (i + 1 < queueElements.size()) std::cout << " ◄ ";
        }
        std::cout << "]";
    }
    std::cout << "\n" << Color::DIM << "  └─────────────────────────────────────────────────────────────┘" << Color::RESET << "\n";
}

void displayStackState(int current, int depth, bool isBacktracking, const Graph& graph) {
    std::string sName = (graph.getStop(current) ? graph.getStop(current)->name : std::to_string(current));
    std::string indent = "";
    for (int i = 0; i < depth; ++i) {
        indent += "│  ";
    }

    if (isBacktracking) {
        std::cout << Color::DIM << indent << "└── " << Color::RESET
                  << Color::MAGENTA << "Backtracking from: " << sName
                  << Color::RESET << "\n";
    } else {
        std::cout << Color::CYAN << indent << "├── " << Color::RESET
                  << Color::BOLD << Color::BRIGHT_YELLOW << "[Depth " << depth << "] "
                  << Color::RESET << Color::WHITE << "Exploring: "
                  << Color::BOLD << sName << Color::RESET
                  << Color::DIM << " (ID: " << current << ")" << Color::RESET << "\n";
    }
}

void displayRouteAscii(const std::vector<int>& path,
                       const Graph& graph,
                       const std::string& title) {
    if (path.empty()) {
        std::cout << Color::BRIGHT_RED << "  No available route found." << Color::RESET << "\n";
        return;
    }

    double totalDist = 0.0;
    int totalTime = 0;
    graph.calculatePathMetrics(path, totalDist, totalTime);

    std::cout << "\n" << Color::BRIGHT_GREEN << "═════════════════════════════════════════════════════════════════════════════" << Color::RESET << "\n";
    std::cout << Color::BOLD << Color::BRIGHT_GREEN << "  " << title << Color::RESET << "\n";
    std::cout << Color::BRIGHT_GREEN << "═════════════════════════════════════════════════════════════════════════════" << Color::RESET << "\n\n";

    for (size_t i = 0; i < path.size(); ++i) {
        int stopId = path[i];
        const Stop* s = graph.getStop(stopId);
        std::string sName = (s ? s->name : "Stop " + std::to_string(stopId));
        std::string sZone = (s ? s->zone : "");

        if (i == 0) {
            // Source Stop
            std::cout << "  " << Color::BOLD << Color::BG_GREEN << Color::BLACK << " BOARD " << Color::RESET
                      << "  " << Color::BOLD << Color::BRIGHT_YELLOW << "[ " << sName << " ]" << Color::RESET
                      << " " << Color::DIM << "(" << sZone << ")" << Color::RESET << "\n";
        } else if (i == path.size() - 1) {
            // Destination Stop
            std::cout << "  " << Color::BOLD << Color::BG_BLUE << Color::BLACK << " ALIGHT " << Color::RESET
                      << " " << Color::BOLD << Color::BRIGHT_CYAN << "[ " << sName << " ]" << Color::RESET
                      << " " << Color::DIM << "(" << sZone << ")" << Color::RESET << "\n";
        } else {
            // Intermediate Stop
            std::cout << "  " << Color::DIM << "   ●    " << Color::RESET
                      << Color::BOLD << Color::WHITE << "[ " << sName << " ]" << Color::RESET
                      << " " << Color::DIM << "(" << sZone << ")" << Color::RESET << "\n";
        }

        // Draw connecting link if not last stop
        if (i + 1 < path.size()) {
            const Road* r = graph.getRoad(path[i], path[i + 1]);
            double d = (r ? r->distanceKm : 3.0);
            int t = (r ? r->timeMin : 8);

            std::cout << Color::BRIGHT_GREEN << "         │\n";
            std::cout << "         ├─► " << Color::DIM << "(" << std::fixed << std::setprecision(1) << d
                      << " km, ~" << t << " min)" << Color::RESET << "\n";
            std::cout << Color::BRIGHT_GREEN << "         │" << Color::RESET << "\n";
        }
    }

    std::cout << "\n" << Color::CYAN << "┌─── JOURNEY METRICS SUMMARY ────────────────────────────────────────────────┐\n";
    std::cout << "│  " << Color::BOLD << "Total Stops:" << Color::RESET << " " << path.size()
              << " | " << Color::BOLD << "Transit Connections (Hops):" << Color::RESET << " " << (path.size() - 1) << "\n";
    std::cout << "│  " << Color::BOLD << "Estimated Road Distance:" << Color::RESET << " " << std::fixed << std::setprecision(1) << totalDist << " km\n";
    std::cout << "│  " << Color::BOLD << "Estimated Bus Travel Time:" << Color::RESET << " ~" << totalTime << " minutes\n";
    std::cout << "└────────────────────────────────────────────────────────────────────────────┘" << Color::RESET << "\n";
}

void displayTransferJourney(const std::vector<JourneyLeg>& journey, const Graph& graph) {
    if (journey.empty()) {
        std::cout << Color::BRIGHT_RED << "  No bus combination found connecting these stops." << Color::RESET << "\n";
        return;
    }

    std::cout << "\n" << Color::BRIGHT_MAGENTA << "═════════════════════════════════════════════════════════════════════════════" << Color::RESET << "\n";
    std::cout << Color::BOLD << Color::BRIGHT_MAGENTA << "  OPTIMAL BUS ROUTE (MINIMUM BUS TRANSFERS: "
              << (journey.size() - 1) << " SWITCH" << (journey.size() > 2 ? "ES" : "") << ")" << Color::RESET << "\n";
    std::cout << Color::BRIGHT_MAGENTA << "═════════════════════════════════════════════════════════════════════════════" << Color::RESET << "\n\n";

    double totalDist = 0.0;
    int totalTime = 0;

    for (size_t legIdx = 0; legIdx < journey.size(); ++legIdx) {
        const auto& leg = journey[legIdx];
        const Stop* bStop = graph.getStop(leg.boardStopId);
        const Stop* aStop = graph.getStop(leg.alightStopId);

        std::string bName = (bStop ? bStop->name : "Stop " + std::to_string(leg.boardStopId));
        std::string aName = (aStop ? aStop->name : "Stop " + std::to_string(leg.alightStopId));

        double legDist = 0.0;
        int legTime = 0;
        graph.calculatePathMetrics(leg.stopsInLeg, legDist, legTime);
        totalDist += legDist;
        totalTime += legTime;

        std::cout << Color::BOLD << Color::BRIGHT_YELLOW
                  << "  LEG " << (legIdx + 1) << " OF " << journey.size() << ": "
                  << Color::RESET << Color::BOLD << Color::CYAN << leg.busName
                  << Color::RESET << " [" << leg.busType << "]\n";
        std::cout << Color::DIM << "  ─────────────────────────────────────────────────────────────────" << Color::RESET << "\n";
        std::cout << "  Board at:  " << Color::BOLD << Color::BRIGHT_GREEN << bName << Color::RESET << "\n";

        std::cout << "  Waypoints: " << Color::DIM;
        for (size_t s = 0; s < leg.stopsInLeg.size(); ++s) {
            const Stop* st = graph.getStop(leg.stopsInLeg[s]);
            std::cout << (st ? st->name : std::to_string(leg.stopsInLeg[s]));
            if (s + 1 < leg.stopsInLeg.size()) std::cout << " -> ";
        }
        std::cout << Color::RESET << "\n";

        std::cout << "  Alight at: " << Color::BOLD << Color::BRIGHT_CYAN << aName << Color::RESET
                  << "  " << Color::DIM << "(" << std::fixed << std::setprecision(1) << legDist
                  << " km, ~" << legTime << " min)" << Color::RESET << "\n\n";

        if (legIdx + 1 < journey.size()) {
            std::cout << Color::BOLD << Color::BRIGHT_RED
                      << "  [TRANSFER INTERCHANGE] " << Color::RESET
                      << "Switch buses at " << Color::BOLD << aName << Color::RESET
                      << " (Allow ~5-10 min for transfer)\n\n";
            totalTime += 8; // average transfer wait time
        }
    }

    std::cout << Color::MAGENTA << "┌─── MULTI-LEG JOURNEY TOTALS ───────────────────────────────────────────────┐\n";
    std::cout << "│  " << Color::BOLD << "Total Boardings:" << Color::RESET << " " << journey.size() << " bus(es) | "
              << Color::BOLD << "Transfers Required:" << Color::RESET << " " << (journey.size() - 1) << "\n";
    std::cout << "│  " << Color::BOLD << "Total Estimated Distance:" << Color::RESET << " " << std::fixed << std::setprecision(1) << totalDist << " km\n";
    std::cout << "│  " << Color::BOLD << "Total Estimated Time:" << Color::RESET << " ~" << totalTime << " minutes (including wait)\n";
    std::cout << "└────────────────────────────────────────────────────────────────────────────┘" << Color::RESET << "\n";
}

void displayRoadClosureReport(const RoadClosureAnalysis& report, const Graph& graph) {
    std::cout << "\n" << Color::BRIGHT_RED << "═════════════════════════════════════════════════════════════════════════════" << Color::RESET << "\n";
    std::cout << Color::BOLD << Color::BRIGHT_RED << "  ROAD CLOSURE & BOTTLENECK DFS IMPACT REPORT" << Color::RESET << "\n";
    std::cout << Color::BRIGHT_RED << "═════════════════════════════════════════════════════════════════════════════" << Color::RESET << "\n\n";

    std::cout << "  Closed Corridor: " << Color::BOLD << Color::BRIGHT_YELLOW
              << report.nameU << " <──[BLOCKED]──> " << report.nameV << Color::RESET << "\n\n";

    std::cout << "  • Network Components Before Closure: " << Color::BOLD << report.componentsBefore << Color::RESET << "\n";
    std::cout << "  • Network Components After Closure:  " << Color::BOLD << report.componentsAfter << Color::RESET << "\n\n";

    if (report.isCriticalBridge) {
        std::cout << Color::BG_RED << Color::BOLD << Color::BRIGHT_WHITE
                  << "  CRITICAL BOTTLENECK / BRIDGE DETECTED!  " << Color::RESET << "\n";
        std::cout << Color::BRIGHT_RED
                  << "  Closing this road splits the Dhaka transit network into "
                  << report.componentsAfter << " disconnected parts!" << Color::RESET << "\n";
        std::cout << Color::RED << "  No alternative road path exists between "
                  << report.nameU << " and " << report.nameV << "!" << Color::RESET << "\n";
    } else {
        std::cout << Color::BG_GREEN << Color::BLACK << Color::BOLD
                  << "  NETWORK REMAINS CONNECTED  " << Color::RESET << "\n";
        std::cout << Color::GREEN << "  This road is not a single point of failure. Redundant routes exist." << Color::RESET << "\n\n";

        if (!report.alternativeRoute.empty()) {
            std::cout << Color::BOLD << Color::CYAN << "  Alternative Bypass Route (BFS):" << Color::RESET << "\n  ";
            for (size_t i = 0; i < report.alternativeRoute.size(); ++i) {
                const Stop* s = graph.getStop(report.alternativeRoute[i]);
                std::cout << Color::BOLD << (s ? s->name : std::to_string(report.alternativeRoute[i])) << Color::RESET;
                if (i + 1 < report.alternativeRoute.size()) {
                    std::cout << Color::GREEN << " ──► " << Color::RESET;
                }
            }
            std::cout << "\n";
        }
    }
}

void displayBusRecommendations(const std::vector<BusRecommendation>& recs,
                               int srcId, int destId,
                               const Graph& graph) {
    const Stop* s = graph.getStop(srcId);
    const Stop* d = graph.getStop(destId);

    std::string srcName = (s ? s->name : "Origin");
    std::string destName = (d ? d->name : "Destination");

    std::cout << "\n" << Color::BRIGHT_CYAN << "═════════════════════════════════════════════════════════════════════════════" << Color::RESET << "\n";
    std::cout << Color::BOLD << Color::BRIGHT_CYAN << "  BUS RECOMMENDATIONS FOR: "
              << srcName << "  ──►  " << destName << Color::RESET << "\n";
    std::cout << Color::DIM << "  (Ranked by travel time & connection convenience using predefined bus routes)" << Color::RESET << "\n";
    std::cout << Color::BRIGHT_CYAN << "═════════════════════════════════════════════════════════════════════════════" << Color::RESET << "\n\n";

    if (recs.empty()) {
        std::cout << Color::BRIGHT_RED << "  No recommended bus routes found between these two locations." << Color::RESET << "\n";
        return;
    }

    for (size_t i = 0; i < recs.size(); ++i) {
        const auto& rec = recs[i];
        std::string medal = (i == 0 ? "🥇 TOP CHOICE" : (i == 1 ? "🥈 RUNNER UP" : "🥉 ALTERNATIVE"));

        std::cout << Color::BOLD << Color::BRIGHT_YELLOW << "  " << medal << ": "
                  << Color::BRIGHT_WHITE << rec.bus.name << Color::RESET
                  << " (" << rec.bus.type << ")\n";

        std::cout << Color::DIM << "  ┌────────────────────────────────────────────────────────────────────────┐" << Color::RESET << "\n";
        std::cout << Color::DIM << "  │ " << Color::RESET
                  << Color::BOLD << "Highlight:  " << Color::RESET << Color::BRIGHT_GREEN << rec.highlightReason << Color::RESET << "\n";
        std::cout << Color::DIM << "  │ " << Color::RESET
                  << Color::BOLD << "Stops:      " << Color::RESET << rec.numStops << " stops along route\n";
        std::cout << Color::DIM << "  │ " << Color::RESET
                  << Color::BOLD << "Distance:   " << Color::RESET << std::fixed << std::setprecision(1) << rec.estimatedDistanceKm << " km\n";
        std::cout << Color::DIM << "  │ " << Color::RESET
                  << Color::BOLD << "Est. Time:  " << Color::RESET << "~" << rec.estimatedTimeMin << " minutes\n";
        std::cout << Color::DIM << "  │ " << Color::RESET
                  << Color::BOLD << "Frequency:  " << Color::RESET << "Every " << rec.bus.frequencyMin << " mins | Base Fare: " << rec.bus.baseFare << " BDT\n";

        std::cout << Color::DIM << "  │ " << Color::RESET << Color::BOLD << "Corridor:   " << Color::RESET << Color::CYAN;
        for (size_t sIdx = 0; sIdx < rec.traversedStops.size(); ++sIdx) {
            const Stop* st = graph.getStop(rec.traversedStops[sIdx]);
            std::cout << (st ? st->name : std::to_string(rec.traversedStops[sIdx]));
            if (sIdx + 1 < rec.traversedStops.size()) std::cout << " -> ";
        }
        std::cout << Color::RESET << "\n";
        std::cout << Color::DIM << "  └────────────────────────────────────────────────────────────────────────┘" << Color::RESET << "\n\n";
    }
}

void displayComponents(const std::vector<std::vector<int>>& components, const Graph& graph) {
    std::cout << "\n" << Color::BRIGHT_YELLOW << "═════════════════════════════════════════════════════════════════════════════" << Color::RESET << "\n";
    std::cout << Color::BOLD << Color::BRIGHT_YELLOW << "  DFS CONNECTED COMPONENTS & NETWORK TOPOLOGY" << Color::RESET << "\n";
    std::cout << Color::BRIGHT_YELLOW << "═════════════════════════════════════════════════════════════════════════════" << Color::RESET << "\n\n";

    std::cout << "  Total Network Subgraphs (Connected Components): "
              << Color::BOLD << components.size() << Color::RESET << "\n\n";

    std::vector<int> isolatedStops;

    for (size_t i = 0; i < components.size(); ++i) {
        if (components[i].size() == 1) {
            isolatedStops.push_back(components[i][0]);
            continue;
        }

        std::cout << Color::BOLD << Color::CYAN << "  Component #" << (i + 1)
                  << " (" << components[i].size() << " connected stops):" << Color::RESET << "\n  ";

        for (size_t j = 0; j < components[i].size(); ++j) {
            const Stop* s = graph.getStop(components[i][j]);
            std::cout << (s ? s->name : std::to_string(components[i][j]));
            if (j + 1 < components[i].size()) std::cout << ", ";
            if ((j + 1) % 6 == 0) std::cout << "\n  ";
        }
        std::cout << "\n\n";
    }

    if (!isolatedStops.empty()) {
        std::cout << Color::BRIGHT_RED << Color::BOLD
                  << "  ⚠️  ISOLATED / UNREACHABLE STOPS DETECTED (" << isolatedStops.size() << "):" << Color::RESET << "\n";
        for (int id : isolatedStops) {
            const Stop* s = graph.getStop(id);
            std::cout << Color::RED << "    • " << (s ? s->name : "ID: " + std::to_string(id))
                      << " (No open roads connected)" << Color::RESET << "\n";
        }
        std::cout << "\n";
    } else {
        std::cout << Color::BRIGHT_GREEN << "  ✓ No completely isolated stops in current network state." << Color::RESET << "\n\n";
    }
}

void displayReachableStops(int srcId,
                           const std::vector<std::pair<int, int>>& results,
                           const Graph& graph) {
    const Stop* srcStop = graph.getStop(srcId);
    std::string srcName = (srcStop ? srcStop->name : "Stop " + std::to_string(srcId));

    std::cout << "\n" << Color::BRIGHT_CYAN << "═════════════════════════════════════════════════════════════════════════════" << Color::RESET << "\n";
    std::cout << Color::BOLD << Color::BRIGHT_CYAN << "  ALL REACHABLE STOPS FROM: " << srcName << Color::RESET << "\n";
    std::cout << Color::BRIGHT_CYAN << "═════════════════════════════════════════════════════════════════════════════" << Color::RESET << "\n\n";

    // Group by hops
    std::map<int, std::vector<int>> byHop;
    for (const auto& item : results) {
        byHop[item.second].push_back(item.first);
    }

    for (const auto& pair : byHop) {
        int hop = pair.first;
        std::cout << Color::BOLD << Color::YELLOW << "  [Connection Level " << hop << "]: " << Color::RESET;
        if (hop == 0) {
            std::cout << Color::BRIGHT_WHITE << srcName << " (Origin Point)\n";
        } else {
            for (size_t i = 0; i < pair.second.size(); ++i) {
                const Stop* s = graph.getStop(pair.second[i]);
                std::cout << Color::WHITE << (s ? s->name : std::to_string(pair.second[i])) << Color::RESET;
                if (i + 1 < pair.second.size()) std::cout << ", ";
            }
            std::cout << "\n";
        }
    }
    std::cout << "\n  " << Color::BOLD << "Total Reachable Locations: " << Color::RESET
              << results.size() << " / " << graph.getAllStops().size() << " network stops.\n";
}

int readInt(const std::string& prompt, int minVal, int maxVal) {
    while (true) {
        std::cout << Color::BOLD << prompt << Color::RESET << " ";
        std::string input;
        if (!std::getline(std::cin, input)) {
            return minVal;
        }

        // Trim input
        input.erase(0, input.find_first_not_of(" \t\r\n"));
        input.erase(input.find_last_not_of(" \t\r\n") + 1);

        if (input.empty()) {
            std::cout << Color::RED << "  Please enter a valid number." << Color::RESET << "\n";
            continue;
        }

        try {
            int val = std::stoi(input);
            if (val >= minVal && val <= maxVal) {
                return val;
            }
            std::cout << Color::RED << "  Value must be between " << minVal << " and " << maxVal << "." << Color::RESET << "\n";
        } catch (...) {
            std::cout << Color::RED << "  Invalid numeric input. Please try again." << Color::RESET << "\n";
        }
    }
}

std::string readLine(const std::string& prompt) {
    std::cout << Color::BOLD << prompt << Color::RESET << " ";
    std::string input;
    std::getline(std::cin, input);
    // Trim
    size_t first = input.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return "";
    size_t last = input.find_last_not_of(" \t\r\n");
    return input.substr(first, (last - first + 1));
}

bool confirmAction(const std::string& prompt) {
    std::cout << Color::BOLD << prompt << " (y/n): " << Color::RESET;
    std::string input;
    std::getline(std::cin, input);
    if (!input.empty() && (input[0] == 'y' || input[0] == 'Y')) {
        return true;
    }
    return false;
}

} // namespace UI

#include "App.h"

App::App() : isRunning(true) {
    dataMgr.ensureDefaultFilesExist(graph, busMgr);
    dataMgr.loadAll(graph, busMgr);
}

int App::promptSelectStop(const std::string& role) {
    std::cout << "\n" << Color::BOLD << Color::YELLOW << "Select " << role << " Bus Stop:" << Color::RESET << "\n";
    std::cout << Color::DIM << "Enter Stop ID or type part of the stop name (or 'list' to view all stops):" << Color::RESET << "\n";

    while (true) {
        std::string input = UI::readLine("  " + role + " >");
        if (input.empty()) continue;

        if (input == "list" || input == "LIST") {
            std::cout << "\n" << Color::CYAN << "─── AVAILABLE BUS STOPS ───" << Color::RESET << "\n";
            int count = 0;
            for (const auto& pair : graph.getAllStops()) {
                std::cout << std::setw(3) << pair.first << ". "
                          << std::left << std::setw(18) << pair.second.name
                          << Color::DIM << "(" << pair.second.zone << ")" << Color::RESET;
                if (++count % 2 == 0) std::cout << "\n";
                else std::cout << "   ";
            }
            if (count % 2 != 0) std::cout << "\n";
            std::cout << "\n";
            continue;
        }

        // Try parsing numeric ID
        try {
            int id = std::stoi(input);
            if (graph.getStop(id)) {
                return id;
            }
            std::cout << Color::RED << "  Stop ID " << id << " not found." << Color::RESET << "\n";
        } catch (...) {
            // Match by name
            int id = graph.getStopIdByName(input);
            if (id != -1) {
                return id;
            }

            // Substring search
            std::vector<int> matches;
            std::string lowerInput = input;
            std::transform(lowerInput.begin(), lowerInput.end(), lowerInput.begin(), ::tolower);
            for (const auto& pair : graph.getAllStops()) {
                std::string sName = pair.second.name;
                std::transform(sName.begin(), sName.end(), sName.begin(), ::tolower);
                if (sName.find(lowerInput) != std::string::npos) {
                    matches.push_back(pair.first);
                }
            }

            if (matches.size() == 1) {
                return matches[0];
            } else if (matches.size() > 1) {
                std::cout << Color::YELLOW << "  Multiple matching stops found:" << Color::RESET << "\n";
                for (int mId : matches) {
                    std::cout << "    [" << mId << "] " << graph.getStop(mId)->name << "\n";
                }
                std::cout << "  Please enter the specific Stop ID.\n";
            } else {
                std::cout << Color::RED << "  No stop matching '" << input << "'. Enter 'list' to see all stops." << Color::RESET << "\n";
            }
        }
    }
}

int App::promptSelectBus() {
    std::cout << "\n" << Color::CYAN << "─── AVAILABLE BUS FLEET ───" << Color::RESET << "\n";
    for (const auto& b : busMgr.getAllBuses()) {
        std::cout << "  [" << b.id << "] " << std::left << std::setw(22) << b.name
                  << " | " << std::setw(12) << b.type
                  << " | Freq: " << b.frequencyMin << " min\n";
    }
    return UI::readInt("\nSelect Bus ID:", 1, 99999);
}

void App::runJourneyFinder() {
    UI::clearScreen();
    UI::printHeader("JOURNEY FINDER (BFS - SHORTEST CONNECTION ROUTE)");
    std::cout << "  Calculates the route with the MINIMUM number of intermediate stops\n"
              << "  using Breadth-First Search. Also finds detours around blocked roads.\n";

    int src = promptSelectStop("Origin / Boarding");
    int dest = promptSelectStop("Final Destination");

    if (src == dest) {
        std::cout << Color::YELLOW << "\n  Origin and Destination are identical. No transit needed!" << Color::RESET << "\n";
        UI::pauseScreen();
        return;
    }

    std::vector<int> path;
    bool found = graph.bfsShortestRoute(src, dest, path, true);

    if (found) {
        UI::displayRouteAscii(path, graph, "SHORTEST CONNECTION ROUTE (BFS)");
    } else {
        std::cout << "\n" << Color::BRIGHT_RED << "  [X] No available route found between "
                  << graph.getStop(src)->name << " and " << graph.getStop(dest)->name
                  << " due to road closures or network disconnection." << Color::RESET << "\n";
    }

    UI::pauseScreen();
}

void App::runBusTransferFinder() {
    UI::clearScreen();
    UI::printHeader("BUS TRANSFER FINDER (BFS - MINIMUM BUS CHANGES)");
    std::cout << "  Finds the optimal journey using Breadth-First Search across the bus lines\n"
              << "  to minimize vehicle switches (transfers).\n";

    int src = promptSelectStop("Origin / Boarding");
    int dest = promptSelectStop("Final Destination");

    if (src == dest) {
        std::cout << Color::YELLOW << "\n  Origin and Destination are identical." << Color::RESET << "\n";
        UI::pauseScreen();
        return;
    }

    std::vector<JourneyLeg> journey;
    bool found = graph.bfsMinimumBusTransfers(src, dest, busMgr.getAllBuses(), journey, true);

    if (found) {
        UI::displayTransferJourney(journey, graph);
    } else {
        std::cout << "\n" << Color::BRIGHT_RED << "  [X] No bus route combination connects "
                  << graph.getStop(src)->name << " and " << graph.getStop(dest)->name << Color::RESET << "\n";
    }

    UI::pauseScreen();
}

void App::runReachabilityExplorer() {
    UI::clearScreen();
    UI::printHeader("REACHABILITY EXPLORER (BFS LEVEL-BY-LEVEL)");
    std::cout << "  Discovers all bus stops reachable from a starting location up to a hop depth.\n";

    int src = promptSelectStop("Starting Location");
    std::cout << "\n  Enter maximum connections/hops limit (-1 for all reachable stops): ";
    int maxHops = UI::readInt("", -1, 50);

    std::vector<std::pair<int, int>> results;
    graph.bfsAllReachable(src, maxHops, results, true);

    UI::displayReachableStops(src, results, graph);
    UI::pauseScreen();
}

void App::runNetworkExplorer() {
    UI::clearScreen();
    UI::printHeader("NETWORK EXPLORER (DFS ALGORITHM)");
    std::cout << "  1. Deep Network Walk from a Stop (Depth-First Recursion Trace)\n"
              << "  2. Detect Connected Components & Isolated/Unreachable Stops\n"
              << "  0. Return to Main Menu\n\n";

    int choice = UI::readInt("Enter choice (0-2):", 0, 2);
    if (choice == 1) {
        int src = promptSelectStop("Starting Location");
        std::vector<int> visitOrder;
        graph.dfsExplore(src, visitOrder, true);

        std::cout << "\n" << Color::BRIGHT_GREEN << "  DFS Complete Traversal Order ("
                  << visitOrder.size() << " stops reached):" << Color::RESET << "\n  ";
        for (size_t i = 0; i < visitOrder.size(); ++i) {
            std::cout << graph.getStop(visitOrder[i])->name;
            if (i + 1 < visitOrder.size()) std::cout << " -> ";
            if ((i + 1) % 5 == 0) std::cout << "\n  ";
        }
        std::cout << "\n";
        UI::pauseScreen();
    } else if (choice == 2) {
        UI::showLoadingAnimation("Analyzing Dhaka bus network topology with DFS");
        auto components = graph.dfsFindConnectedComponents();
        UI::displayComponents(components, graph);
        UI::pauseScreen();
    }
}

void App::runRoadClosureImpact() {
    UI::clearScreen();
    UI::printHeader("ROAD CLOSURE & BOTTLENECK DFS IMPACT ANALYSIS");
    std::cout << "  Simulates a road closure and uses DFS to analyze if the corridor is a\n"
              << "  critical bridge (single point of failure) or if redundant bypasses exist.\n\n";

    std::cout << "Select the road segment to simulate closure:\n";
    int u = promptSelectStop("First Endpoint of Road");
    int v = promptSelectStop("Second Endpoint of Road");

    const Road* r = graph.getRoad(u, v);
    if (!r) {
        std::cout << Color::RED << "\n  No direct road exists between "
                  << graph.getStop(u)->name << " and " << graph.getStop(v)->name << "." << Color::RESET << "\n";
        UI::pauseScreen();
        return;
    }

    UI::showLoadingAnimation("Running DFS connectivity and bridge analysis");
    auto report = graph.dfsAnalyzeClosureImpact(u, v, busMgr.getAllBuses());
    UI::displayRoadClosureReport(report, graph);

    UI::pauseScreen();
}

void App::runBusRecommendations() {
    UI::clearScreen();
    UI::printHeader("BUS RECOMMENDATION & COMPARISON");
    std::cout << "  Compares and recommends up to 3 suitable buses for a passenger trip\n"
              << "  based on travel time, distance, intermediate stops, and service frequency.\n";

    int src = promptSelectStop("Boarding Location");
    int dest = promptSelectStop("Destination Location");

    if (src == dest) {
        std::cout << Color::YELLOW << "\n  Source and destination are the same." << Color::RESET << "\n";
        UI::pauseScreen();
        return;
    }

    UI::showLoadingAnimation("Evaluating bus fleets and corridor timings");
    auto recs = busMgr.recommendBuses(src, dest, graph);
    UI::displayBusRecommendations(recs, src, dest, graph);

    UI::pauseScreen();
}

void App::runSearchStopsAndBuses() {
    UI::clearScreen();
    UI::printHeader("SEARCH BUS STOPS & FLEET");
    std::cout << "  1. Search Stops by Name / Zone\n"
              << "  2. Search Buses by Name or Stops Served\n"
              << "  3. List All Stops (" << graph.getAllStops().size() << ")\n"
              << "  4. List All Buses (" << busMgr.getAllBuses().size() << ")\n"
              << "  0. Back\n\n";

    int choice = UI::readInt("Enter choice (0-4):", 0, 4);
    if (choice == 1) {
        std::string query = UI::readLine("Enter search keyword:");
        std::string lowerQ = query;
        std::transform(lowerQ.begin(), lowerQ.end(), lowerQ.begin(), ::tolower);

        std::cout << "\n" << Color::CYAN << "Search Results for '" << query << "':" << Color::RESET << "\n";
        int matches = 0;
        for (const auto& p : graph.getAllStops()) {
            std::string n = p.second.name;
            std::string z = p.second.zone;
            std::string comb = n + " " + z;
            std::transform(comb.begin(), comb.end(), comb.begin(), ::tolower);
            if (comb.find(lowerQ) != std::string::npos) {
                std::cout << "  [" << p.first << "] " << std::left << std::setw(20) << p.second.name
                          << " | Zone: " << p.second.zone << "\n";
                matches++;
            }
        }
        if (matches == 0) std::cout << Color::RED << "  No matching stops found.\n" << Color::RESET;
        UI::pauseScreen();
    } else if (choice == 2) {
        std::string query = UI::readLine("Enter bus name or stop name:");
        std::string lowerQ = query;
        std::transform(lowerQ.begin(), lowerQ.end(), lowerQ.begin(), ::tolower);

        std::cout << "\n" << Color::CYAN << "Search Results:" << Color::RESET << "\n";
        int matches = 0;
        for (const auto& b : busMgr.getAllBuses()) {
            std::string bName = b.name;
            std::transform(bName.begin(), bName.end(), bName.begin(), ::tolower);

            bool servesStop = false;
            for (int sId : b.route) {
                const Stop* s = graph.getStop(sId);
                if (s) {
                    std::string sName = s->name;
                    std::transform(sName.begin(), sName.end(), sName.begin(), ::tolower);
                    if (sName.find(lowerQ) != std::string::npos) {
                        servesStop = true;
                        break;
                    }
                }
            }

            if (bName.find(lowerQ) != std::string::npos || servesStop) {
                std::cout << Color::BOLD << "  [" << b.id << "] " << b.name << Color::RESET
                          << " (" << b.type << ") | Fare: " << b.baseFare << " BDT | Freq: " << b.frequencyMin << " min\n";
                std::cout << "    Route: ";
                for (size_t i = 0; i < b.route.size(); ++i) {
                    const Stop* st = graph.getStop(b.route[i]);
                    std::cout << (st ? st->name : std::to_string(b.route[i]));
                    if (i + 1 < b.route.size()) std::cout << " -> ";
                }
                std::cout << "\n\n";
                matches++;
            }
        }
        if (matches == 0) std::cout << Color::RED << "  No matching buses found.\n" << Color::RESET;
        UI::pauseScreen();
    } else if (choice == 3) {
        std::cout << "\n" << Color::CYAN << "─── ALL DHAKA BUS STOPS ───" << Color::RESET << "\n";
        for (const auto& p : graph.getAllStops()) {
            std::cout << "  [" << std::setw(2) << p.first << "] "
                      << std::left << std::setw(20) << p.second.name
                      << " | Zone: " << p.second.zone << "\n";
        }
        UI::pauseScreen();
    } else if (choice == 4) {
        std::cout << "\n" << Color::CYAN << "─── ALL REGISTERED BUSES ───" << Color::RESET << "\n";
        for (const auto& b : busMgr.getAllBuses()) {
            std::cout << Color::BOLD << "  [" << b.id << "] " << b.name << Color::RESET
                      << " [" << b.type << "] - Base Fare: " << b.baseFare << " BDT, Frequency: " << b.frequencyMin << " min\n";
            std::cout << "    Route: ";
            for (size_t i = 0; i < b.route.size(); ++i) {
                const Stop* s = graph.getStop(b.route[i]);
                std::cout << (s ? s->name : std::to_string(b.route[i]));
                if (i + 1 < b.route.size()) std::cout << " ─► ";
            }
            std::cout << "\n\n";
        }
        UI::pauseScreen();
    }
}

void App::runRoadBlockSimulator() {
    UI::clearScreen();
    UI::printHeader("ROAD BLOCK SIMULATOR (DYNAMIC ROAD CLOSURES)");

    std::cout << Color::CYAN << "Currently Blocked Road Corridors:" << Color::RESET << "\n";
    const auto& blocked = graph.getBlockedRoads();
    if (blocked.empty()) {
        std::cout << Color::GREEN << "  (No roads currently blocked in the network)" << Color::RESET << "\n\n";
    } else {
        for (const auto& edge : blocked) {
            const Stop* u = graph.getStop(edge.first);
            const Stop* v = graph.getStop(edge.second);
            std::cout << Color::RED << "  ⛔ [CLOSED] "
                      << (u ? u->name : std::to_string(edge.first)) << " <──► "
                      << (v ? v->name : std::to_string(edge.second)) << Color::RESET << "\n";
        }
        std::cout << "\n";
    }

    std::cout << "  1. Block a Road Corridor\n"
              << "  2. Unblock a Road Corridor\n"
              << "  3. Clear All Road Blocks\n"
              << "  0. Back\n\n";

    int choice = UI::readInt("Enter choice (0-3):", 0, 3);
    if (choice == 1) {
        int u = promptSelectStop("Road Stop 1");
        int v = promptSelectStop("Road Stop 2");
        if (graph.blockRoad(u, v)) {
            std::cout << Color::GREEN << "\n  ✓ Road between " << graph.getStop(u)->name
                      << " and " << graph.getStop(v)->name << " is now BLOCKED." << Color::RESET << "\n";
            std::cout << Color::DIM << "  (Journey Finder and DFS will now automatically avoid this road)" << Color::RESET << "\n";
        } else {
            std::cout << Color::RED << "\n  Road segment does not exist." << Color::RESET << "\n";
        }
        UI::pauseScreen();
    } else if (choice == 2) {
        if (blocked.empty()) {
            std::cout << Color::YELLOW << "\n  No roads are currently blocked.\n" << Color::RESET;
            UI::pauseScreen();
            return;
        }
        int u = promptSelectStop("Road Stop 1");
        int v = promptSelectStop("Road Stop 2");
        if (graph.unblockRoad(u, v)) {
            std::cout << Color::GREEN << "\n  ✓ Road between " << graph.getStop(u)->name
                      << " and " << graph.getStop(v)->name << " has been REOPENED." << Color::RESET << "\n";
        } else {
            std::cout << Color::RED << "\n  Road was not blocked or does not exist." << Color::RESET << "\n";
        }
        UI::pauseScreen();
    } else if (choice == 3) {
        auto copyBlocked = blocked;
        for (const auto& edge : copyBlocked) {
            graph.unblockRoad(edge.first, edge.second);
        }
        std::cout << Color::GREEN << "\n  ✓ All road blocks cleared! Full network restored." << Color::RESET << "\n";
        UI::pauseScreen();
    }
}

void App::runAdminMenu() {
    while (true) {
        UI::clearScreen();
        UI::printHeader("ADMIN DATA MANAGEMENT (PERSIST TO TEXT FILES)");
        std::cout << "  1. Manage Bus Stops (Add / Update / Delete)\n"
                  << "  2. Manage Roads (Add / Remove)\n"
                  << "  3. Manage Buses (Add / Update / Remove)\n"
                  << "  4. Reload Network Data from Disk\n"
                  << "  5. Force Save All to Disk\n"
                  << "  0. Return to Main Menu\n\n";

        int choice = UI::readInt("Enter choice (0-5):", 0, 5);
        if (choice == 0) break;

        switch (choice) {
            case 1: adminManageStops(); break;
            case 2: adminManageRoads(); break;
            case 3: adminManageBuses(); break;
            case 4:
                dataMgr.loadAll(graph, busMgr);
                std::cout << Color::GREEN << "\n  ✓ Data reloaded successfully from files." << Color::RESET << "\n";
                UI::pauseScreen();
                break;
            case 5:
                dataMgr.saveAll(graph, busMgr);
                std::cout << Color::GREEN << "\n  ✓ All current network data saved to .txt files." << Color::RESET << "\n";
                UI::pauseScreen();
                break;
        }
    }
}

void App::adminManageStops() {
    UI::clearScreen();
    UI::printHeader("MANAGE BUS STOPS");
    std::cout << "  1. Add New Bus Stop\n"
              << "  2. Update Bus Stop Name/Zone\n"
              << "  3. Remove Bus Stop\n"
              << "  0. Back\n\n";

    int choice = UI::readInt("Choice (0-3):", 0, 3);
    if (choice == 1) {
        // Find next ID
        int nextId = 1;
        for (const auto& p : graph.getAllStops()) {
            if (p.first >= nextId) nextId = p.first + 1;
        }
        std::string name = UI::readLine("Enter stop name (e.g. Rampura):");
        std::string zone = UI::readLine("Enter zone (e.g. East Zone):");
        if (!name.empty()) {
            graph.addStop(Stop(nextId, name, zone));
            dataMgr.saveStops(graph);
            std::cout << Color::GREEN << "\n  ✓ Stop added with ID " << nextId << " and saved!" << Color::RESET << "\n";
        }
        UI::pauseScreen();
    } else if (choice == 2) {
        int id = promptSelectStop("Stop to Update");
        std::string newName = UI::readLine("Enter new name:");
        std::string newZone = UI::readLine("Enter new zone:");
        if (graph.updateStop(id, newName, newZone)) {
            dataMgr.saveStops(graph);
            std::cout << Color::GREEN << "\n  ✓ Stop updated and saved!" << Color::RESET << "\n";
        }
        UI::pauseScreen();
    } else if (choice == 3) {
        int id = promptSelectStop("Stop to Remove");
        if (UI::confirmAction("Are you sure you want to remove this stop and its connected roads?")) {
            if (graph.removeStop(id)) {
                dataMgr.saveAll(graph, busMgr);
                std::cout << Color::GREEN << "\n  ✓ Stop removed successfully." << Color::RESET << "\n";
            }
        }
        UI::pauseScreen();
    }
}

void App::adminManageRoads() {
    UI::clearScreen();
    UI::printHeader("MANAGE ROADS");
    std::cout << "  1. Add New Road Segment\n"
              << "  2. Remove Road Segment\n"
              << "  0. Back\n\n";

    int choice = UI::readInt("Choice (0-2):", 0, 2);
    if (choice == 1) {
        int u = promptSelectStop("Stop 1");
        int v = promptSelectStop("Stop 2");
        if (u == v) {
            std::cout << Color::RED << "  Cannot add a road from a stop to itself.\n" << Color::RESET;
            UI::pauseScreen();
            return;
        }
        std::cout << "Enter road distance in km (e.g. 3.5): ";
        std::string dStr = UI::readLine("");
        double dist = 3.0;
        try { dist = std::stod(dStr); } catch (...) {}

        int timeMin = UI::readInt("Enter average travel time in minutes:", 1, 180);
        graph.addRoad(Road(u, v, dist, timeMin, false));
        dataMgr.saveRoads(graph);
        std::cout << Color::GREEN << "\n  ✓ Road corridor added and saved!" << Color::RESET << "\n";
        UI::pauseScreen();
    } else if (choice == 2) {
        int u = promptSelectStop("Stop 1");
        int v = promptSelectStop("Stop 2");
        if (graph.removeRoad(u, v)) {
            dataMgr.saveRoads(graph);
            std::cout << Color::GREEN << "\n  ✓ Road removed and changes saved." << Color::RESET << "\n";
        } else {
            std::cout << Color::RED << "\n  Road does not exist." << Color::RESET << "\n";
        }
        UI::pauseScreen();
    }
}

void App::adminManageBuses() {
    UI::clearScreen();
    UI::printHeader("MANAGE BUS SERVICES");
    std::cout << "  1. Add New Bus Route\n"
              << "  2. Remove Bus Route\n"
              << "  0. Back\n\n";

    int choice = UI::readInt("Choice (0-2):", 0, 2);
    if (choice == 1) {
        int nextId = 1;
        for (const auto& b : busMgr.getAllBuses()) {
            if (b.id >= nextId) nextId = b.id + 1;
        }
        std::string name = UI::readLine("Enter Bus Name (e.g. Victor Classic):");
        std::string type = UI::readLine("Enter Bus Type (AC Express / Regular / Semi-Seating):");
        int freq = UI::readInt("Enter Frequency in minutes:", 1, 60);
        int fare = UI::readInt("Enter Base Fare in BDT:", 5, 500);

        std::cout << "Enter comma-separated Stop IDs for the bus route (e.g. 1,2,4,6,9):\n";
        std::string stopsStr = UI::readLine("Route Stop IDs:");
        std::vector<int> route;
        std::stringstream ss(stopsStr);
        std::string idStr;
        while (std::getline(ss, idStr, ',')) {
            try {
                int sId = std::stoi(idStr);
                if (graph.getStop(sId)) route.push_back(sId);
            } catch (...) {}
        }

        if (route.size() >= 2) {
            busMgr.addBus(Bus(nextId, name, type, freq, fare, route));
            dataMgr.saveBuses(busMgr);
            std::cout << Color::GREEN << "\n  ✓ Bus added successfully with ID " << nextId << "!" << Color::RESET << "\n";
        } else {
            std::cout << Color::RED << "\n  Invalid route. At least 2 valid stops required." << Color::RESET << "\n";
        }
        UI::pauseScreen();
    } else if (choice == 2) {
        int id = promptSelectBus();
        if (busMgr.removeBus(id)) {
            dataMgr.saveBuses(busMgr);
            std::cout << Color::GREEN << "\n  ✓ Bus removed and changes saved." << Color::RESET << "\n";
        } else {
            std::cout << Color::RED << "\n  Bus not found." << Color::RESET << "\n";
        }
        UI::pauseScreen();
    }
}

void App::runSettingsMenu() {
    UI::clearScreen();
    UI::printHeader("VISUAL & ANIMATION SETTINGS");
    std::cout << "  Current Status:\n"
              << "  • Animations Enabled: " << (g_animConfig.enabled ? Color::GREEN + "YES" : Color::RED + "NO") << Color::RESET << "\n"
              << "  • Algorithm Step Delay: " << Color::CYAN << g_animConfig.stepDelayMs << " ms" << Color::RESET << "\n\n"
              << "  1. Toggle Animations ON / OFF\n"
              << "  2. Set Animation Speed to FAST (60ms)\n"
              << "  3. Set Animation Speed to NORMAL (180ms)\n"
              << "  4. Set Animation Speed to DEMO / SLOW (350ms)\n"
              << "  0. Back\n\n";

    int choice = UI::readInt("Enter choice (0-4):", 0, 4);
    switch (choice) {
        case 1:
            g_animConfig.enabled = !g_animConfig.enabled;
            std::cout << Color::GREEN << "  ✓ Animations toggled to: "
                      << (g_animConfig.enabled ? "ENABLED" : "DISABLED") << Color::RESET << "\n";
            UI::pauseScreen();
            break;
        case 2:
            g_animConfig.enabled = true;
            g_animConfig.stepDelayMs = 60;
            g_animConfig.fastDelayMs = 20;
            std::cout << Color::GREEN << "  ✓ Speed set to FAST." << Color::RESET << "\n";
            UI::pauseScreen();
            break;
        case 3:
            g_animConfig.enabled = true;
            g_animConfig.stepDelayMs = 180;
            g_animConfig.fastDelayMs = 50;
            std::cout << Color::GREEN << "  ✓ Speed set to NORMAL." << Color::RESET << "\n";
            UI::pauseScreen();
            break;
        case 4:
            g_animConfig.enabled = true;
            g_animConfig.stepDelayMs = 350;
            g_animConfig.fastDelayMs = 100;
            std::cout << Color::GREEN << "  ✓ Speed set to DEMO / SLOW." << Color::RESET << "\n";
            UI::pauseScreen();
            break;
    }
}

void App::run() {
    UI::clearScreen();
    UI::printBanner();
    UI::showLoadingAnimation("Initializing TransitIQ Dhaka City transit graph", 700);
    animatedSleep(300);

    while (isRunning) {
        UI::clearScreen();
        UI::printBanner();

        // Dashboard Summary Bar
        int blockedCount = static_cast<int>(graph.getBlockedRoads().size());
        std::cout << "  " << Color::DIM << "Network Status:" << Color::RESET << " "
                  << Color::BOLD << Color::GREEN << graph.getAllStops().size() << " Stops" << Color::RESET << " | "
                  << Color::BOLD << Color::CYAN << graph.getAllRoads().size() << " Road Links" << Color::RESET << " | "
                  << Color::BOLD << Color::MAGENTA << busMgr.getAllBuses().size() << " Bus Services" << Color::RESET;
        if (blockedCount > 0) {
            std::cout << " | " << Color::BG_RED << Color::WHITE << Color::BOLD
                      << " " << blockedCount << " ROAD(S) BLOCKED " << Color::RESET;
        }
        std::cout << "\n\n";

        std::cout << Color::BOLD << Color::BRIGHT_WHITE << "  ─── BFS JOURNEY & EXPLORATION FEATURES ───" << Color::RESET << "\n"
                  << "   " << Color::BRIGHT_GREEN << "1." << Color::RESET << " 🧭  Journey Finder (BFS Shortest Stop Path & Detours)\n"
                  << "   " << Color::BRIGHT_GREEN << "2." << Color::RESET << " 🔄  Bus Transfer Finder (BFS Minimum Vehicle Switches)\n"
                  << "   " << Color::BRIGHT_GREEN << "3." << Color::RESET << " 🌐  Reachability Explorer (BFS Hops Expansion)\n\n"
                  << Color::BOLD << Color::BRIGHT_WHITE << "  ─── DFS NETWORK TOPOLOGY FEATURES ─────────" << Color::RESET << "\n"
                  << "   " << Color::BRIGHT_YELLOW << "4." << Color::RESET << " 🔍  Network Explorer & Component / Island Detection (DFS)\n"
                  << "   " << Color::BRIGHT_YELLOW << "5." << Color::RESET << " ⚠️   Road Closure & Bottleneck Bridge Analysis (DFS)\n\n"
                  << Color::BOLD << Color::BRIGHT_WHITE << "  ─── PASSENGER SERVICES & MANAGEMENT ───────" << Color::RESET << "\n"
                  << "   " << Color::BRIGHT_CYAN << "6." << Color::RESET << " 🚌  Bus Recommendation System (Top 3 Direct/Transfer Buses)\n"
                  << "   " << Color::BRIGHT_CYAN << "7." << Color::RESET << " 🔎  Search Bus Stops & Bus Lines\n"
                  << "   " << Color::BRIGHT_CYAN << "8." << Color::RESET << " 🚧  Road Block Simulator (Block / Unblock Roads)\n"
                  << "   " << Color::BRIGHT_CYAN << "9." << Color::RESET << " 🛠️   Admin Menu (Add/Edit/Delete Network Data)\n"
                  << "   " << Color::BRIGHT_CYAN << "10." << Color::RESET << " ⚙️  Visual & Animation Settings\n"
                  << "   " << Color::RED << "0." << Color::RESET << " 🚪  Exit TransitIQ\n\n";

        int choice = UI::readInt("Select an option (0-10):", 0, 10);

        switch (choice) {
            case 1: runJourneyFinder(); break;
            case 2: runBusTransferFinder(); break;
            case 3: runReachabilityExplorer(); break;
            case 4: runNetworkExplorer(); break;
            case 5: runRoadClosureImpact(); break;
            case 6: runBusRecommendations(); break;
            case 7: runSearchStopsAndBuses(); break;
            case 8: runRoadBlockSimulator(); break;
            case 9: runAdminMenu(); break;
            case 10: runSettingsMenu(); break;
            case 0:
                isRunning = false;
                std::cout << "\n" << Color::BRIGHT_GREEN
                          << "  Thank you for using TransitIQ! Safe travels across Dhaka."
                          << Color::RESET << "\n\n";
                UI::pauseScreen();
                break;
        }
    }
}


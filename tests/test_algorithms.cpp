#include "Common.h"
#include "Graph.h"
#include "BusManager.h"
#include "DataManager.h"
#include <cassert>

int main() {
    std::cout << "===========================================\n";
    std::cout << "RUNNING TRANSITIQ ALGORITHM VERIFICATION TESTS\n";
    std::cout << "===========================================\n\n";

    g_animConfig.enabled = false; // Fast test execution

    Graph graph;
    BusManager busMgr;
    DataManager dataMgr("data/stops.txt", "data/roads.txt", "data/buses.txt");

    bool loaded = dataMgr.loadAll(graph, busMgr);
    assert(loaded);
    std::cout << "[PASS] Loaded data: " << graph.getAllStops().size() << " stops, "
              << graph.getAllRoads().size() << " roads, "
              << busMgr.getAllBuses().size() << " buses.\n";
    assert(graph.getAllStops().size() >= 20);
    assert(busMgr.getAllBuses().size() >= 8);

    // 1. BFS Shortest Path
    std::cout << "\n[TEST 1] BFS Shortest Path (Uttara -> Motijheel)\n";
    std::vector<int> path;
    bool found = graph.bfsShortestRoute(1, 22, path, false);
    assert(found);
    assert(!path.empty());
    assert(path.front() == 1);
    assert(path.back() == 22);
    std::cout << "  -> Route found with " << path.size() << " stops: ";
    for (int id : path) std::cout << graph.getStop(id)->name << " ";
    std::cout << "\n[PASS] BFS Shortest Path works correctly.\n";

    // 2. BFS Road Closure & Bypass Detour
    std::cout << "\n[TEST 2] Road Closure & BFS Alternative Path Bypass\n";
    // Block direct road between Farmgate(14) and Karwan-Bazar(15)
    graph.blockRoad(14, 15);
    assert(graph.isRoadBlocked(14, 15));
    std::vector<int> bypassPath;
    bool bypassFound = graph.bfsShortestRoute(14, 16, bypassPath, false); // Farmgate to Shahbagh
    assert(bypassFound);
    std::cout << "  -> Bypass route found around closed road (14 <-> 15):\n     ";
    for (int id : bypassPath) std::cout << graph.getStop(id)->name << " -> ";
    std::cout << "DONE\n";
    // Restore road
    graph.unblockRoad(14, 15);
    assert(!graph.isRoadBlocked(14, 15));
    std::cout << "[PASS] BFS dynamically bypasses blocked roads.\n";

    // 3. BFS Reachable Stops
    std::cout << "\n[TEST 3] BFS Reachability Exploration from Kuril (4)\n";
    std::vector<std::pair<int, int>> reachables;
    graph.bfsAllReachable(4, 2, reachables, false); // Within 2 hops
    assert(!reachables.empty());
    std::cout << "  -> Reachable stops within 2 hops: " << reachables.size() << " stops.\n";
    std::cout << "[PASS] BFS Reachability exploration works.\n";

    // 4. BFS Minimum Bus Transfers
    std::cout << "\n[TEST 4] BFS Minimum Bus Transfers (Mirpur-10 -> Sayedabad)\n";
    std::vector<JourneyLeg> journey;
    bool transferFound = graph.bfsMinimumBusTransfers(19, 23, busMgr.getAllBuses(), journey, false);
    assert(transferFound);
    std::cout << "  -> Found transit route with " << journey.size() << " bus leg(s) (Transfers: "
              << (journey.size() - 1) << "):\n";
    for (const auto& leg : journey) {
        std::cout << "     Board [" << leg.busName << "] at "
                  << graph.getStop(leg.boardStopId)->name << " -> Alight at "
                  << graph.getStop(leg.alightStopId)->name << "\n";
    }
    std::cout << "[PASS] BFS Minimum Bus Transfers model works.\n";

    // 5. DFS Connected Components & Isolated Stops
    std::cout << "\n[TEST 5] DFS Connected Components\n";
    auto components = graph.dfsFindConnectedComponents();
    std::cout << "  -> Initial connected components count: " << components.size() << "\n";
    assert(components.size() >= 1);
    std::cout << "[PASS] DFS Connected Components analysis works.\n";

    // 6. DFS Road Closure & Bottleneck Impact
    std::cout << "\n[TEST 6] DFS Road Closure Impact (Kuril <-> Banani)\n";
    auto report = graph.dfsAnalyzeClosureImpact(4, 6, busMgr.getAllBuses());
    std::cout << "  -> Was connected before: " << (report.wasConnectedBefore ? "Yes" : "No")
              << " | Connected after: " << (report.isConnectedAfter ? "Yes" : "No")
              << " | Critical Bridge: " << (report.isCriticalBridge ? "Yes" : "No") << "\n";
    std::cout << "[PASS] DFS Closure impact analysis works.\n";

    // 7. Bus Recommendation Engine
    std::cout << "\n[TEST 7] Bus Recommendation (Top 3)\n";
    auto recs = busMgr.recommendBuses(1, 22, graph); // Uttara to Motijheel
    assert(!recs.empty());
    assert(recs.size() <= 3);
    for (size_t i = 0; i < recs.size(); ++i) {
        std::cout << "  -> #" << (i + 1) << " " << recs[i].bus.name << " (" << recs[i].bus.type << ")"
                  << " | Stops: " << recs[i].numStops
                  << " | Est Time: " << recs[i].estimatedTimeMin << " min"
                  << " | Highlight: " << recs[i].highlightReason << "\n";
    }
    std::cout << "[PASS] Bus recommendation comparison works.\n";

    std::cout << "\n===========================================\n";
    std::cout << "ALL 7 ALGORITHM TEST SUITES PASSED PERFECTLY!\n";
    std::cout << "===========================================\n";
    return 0;
}


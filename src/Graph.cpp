#include "Graph.h"
#include "UI.h"

Graph::Graph() {}

void Graph::addStop(const Stop& stop) {
    stops[stop.id] = stop;
    if (adjList.find(stop.id) == adjList.end()) {
        adjList[stop.id] = std::vector<int>();
    }
}

bool Graph::removeStop(int stopId) {
    if (stops.find(stopId) == stops.end()) return false;
    stops.erase(stopId);
    adjList.erase(stopId);

    // Remove from other adjacency lists
    for (auto& pair : adjList) {
        auto& neighbors = pair.second;
        neighbors.erase(std::remove(neighbors.begin(), neighbors.end(), stopId), neighbors.end());
    }

    // Remove associated roads
    std::vector<std::pair<int, int>> toRemove;
    for (const auto& r : roads) {
        if (r.first.first == stopId || r.first.second == stopId) {
            toRemove.push_back(r.first);
        }
    }
    for (const auto& key : toRemove) {
        roads.erase(key);
        blockedRoads.erase(key);
    }
    return true;
}

bool Graph::updateStop(int stopId, const std::string& newName, const std::string& newZone) {
    auto it = stops.find(stopId);
    if (it == stops.end()) return false;
    it->second.name = newName;
    it->second.zone = newZone;
    return true;
}

const Stop* Graph::getStop(int id) const {
    auto it = stops.find(id);
    if (it != stops.end()) return &(it->second);
    return nullptr;
}

const std::map<int, Stop>& Graph::getAllStops() const {
    return stops;
}

int Graph::getStopIdByName(const std::string& name) const {
    std::string lowerQuery = name;
    std::transform(lowerQuery.begin(), lowerQuery.end(), lowerQuery.begin(), ::tolower);
    for (const auto& pair : stops) {
        std::string stopNameLower = pair.second.name;
        std::transform(stopNameLower.begin(), stopNameLower.end(), stopNameLower.begin(), ::tolower);
        if (stopNameLower == lowerQuery) return pair.first;
    }
    return -1;
}

void Graph::addRoad(const Road& road) {
    roads[makeEdgeKey(road.u, road.v)] = road;
    if (road.isBlocked) {
        blockedRoads.insert(makeEdgeKey(road.u, road.v));
    }

    // Add to adjacency list (undirected)
    auto& neighborsU = adjList[road.u];
    if (std::find(neighborsU.begin(), neighborsU.end(), road.v) == neighborsU.end()) {
        neighborsU.push_back(road.v);
    }

    auto& neighborsV = adjList[road.v];
    if (std::find(neighborsV.begin(), neighborsV.end(), road.u) == neighborsV.end()) {
        neighborsV.push_back(road.u);
    }
}

bool Graph::removeRoad(int u, int v) {
    auto key = makeEdgeKey(u, v);
    if (roads.find(key) == roads.end()) return false;

    roads.erase(key);
    blockedRoads.erase(key);

    auto& neighborsU = adjList[u];
    neighborsU.erase(std::remove(neighborsU.begin(), neighborsU.end(), v), neighborsU.end());

    auto& neighborsV = adjList[v];
    neighborsV.erase(std::remove(neighborsV.begin(), neighborsV.end(), u), neighborsV.end());

    return true;
}

bool Graph::blockRoad(int u, int v) {
    auto key = makeEdgeKey(u, v);
    if (roads.find(key) == roads.end()) return false;
    roads[key].isBlocked = true;
    blockedRoads.insert(key);
    return true;
}

bool Graph::unblockRoad(int u, int v) {
    auto key = makeEdgeKey(u, v);
    if (roads.find(key) == roads.end()) return false;
    roads[key].isBlocked = false;
    blockedRoads.erase(key);
    return true;
}

bool Graph::isRoadBlocked(int u, int v) const {
    auto key = makeEdgeKey(u, v);
    return blockedRoads.find(key) != blockedRoads.end();
}

const std::map<std::pair<int, int>, Road>& Graph::getAllRoads() const {
    return roads;
}

const Road* Graph::getRoad(int u, int v) const {
    auto it = roads.find(makeEdgeKey(u, v));
    if (it != roads.end()) return &(it->second);
    return nullptr;
}

const std::set<std::pair<int, int>>& Graph::getBlockedRoads() const {
    return blockedRoads;
}

void Graph::calculatePathMetrics(const std::vector<int>& path, double& totalDistKm, int& totalTimeMin) const {
    totalDistKm = 0.0;
    totalTimeMin = 0;
    if (path.size() < 2) return;

    for (size_t i = 0; i < path.size() - 1; ++i) {
        const Road* r = getRoad(path[i], path[i + 1]);
        if (r) {
            totalDistKm += r->distanceKm;
            totalTimeMin += r->timeMin;
        } else {
            // Default estimation if direct road missing
            totalDistKm += 3.0;
            totalTimeMin += 8;
        }
    }
}

// -------------------------------------------------------------
// BFS: Shortest path by minimum connections (hops)
// -------------------------------------------------------------
bool Graph::bfsShortestRoute(int src, int dest, std::vector<int>& path, bool visualize) const {
    path.clear();
    if (stops.find(src) == stops.end() || stops.find(dest) == stops.end()) {
        return false;
    }

    if (src == dest) {
        path.push_back(src);
        return true;
    }

    std::queue<int> q;
    std::map<int, bool> visited;
    std::map<int, int> parent;

    q.push(src);
    visited[src] = true;

    if (visualize) {
        std::cout << "\n" << Color::BRIGHT_CYAN << "─── BFS JOURNEY FINDER STEP-BY-STEP TRACE ───" << Color::RESET << "\n";
        std::cout << Color::DIM << "Guarantees route with MINIMUM intermediate connections (stop hops)." << Color::RESET << "\n\n";
    }

    bool found = false;

    while (!q.empty()) {
        // Snapshot queue elements for visualization
        std::vector<int> currentQueue;
        std::queue<int> tempQ = q;
        while (!tempQ.empty()) {
            currentQueue.push_back(tempQ.front());
            tempQ.pop();
        }

        int curr = q.front();
        q.pop();

        if (visualize) {
            UI::displayQueueState(currentQueue, curr, *this);
            animatedSleep(g_animConfig.stepDelayMs);
        }

        if (curr == dest) {
            found = true;
            if (visualize) {
                std::cout << Color::BRIGHT_GREEN << "  [✓] Target destination '"
                          << stops.at(dest).name << "' reached in BFS traversal!" << Color::RESET << "\n\n";
            }
            break;
        }

        auto it = adjList.find(curr);
        if (it != adjList.end()) {
            for (int neighbor : it->second) {
                // Check if road is blocked
                if (isRoadBlocked(curr, neighbor)) {
                    if (visualize) {
                        std::cout << Color::BRIGHT_RED << "    [BLOCKED ROAD] "
                                  << stops.at(curr).name << " <-> "
                                  << stops.at(neighbor).name << " (Skipping traversal)"
                                  << Color::RESET << "\n";
                        animatedSleep(g_animConfig.fastDelayMs);
                    }
                    continue;
                }

                if (!visited[neighbor]) {
                    visited[neighbor] = true;
                    parent[neighbor] = curr;
                    q.push(neighbor);

                    if (visualize) {
                        std::cout << Color::CYAN << "    -> Enqueuing neighbor: "
                                  << Color::BOLD << stops.at(neighbor).name << Color::RESET
                                  << " (from " << stops.at(curr).name << ")\n";
                        animatedSleep(g_animConfig.fastDelayMs);
                    }
                }
            }
        }
    }

    if (!found) {
        return false;
    }

    // Backtrack path from dest to src
    int curr = dest;
    while (curr != src) {
        path.push_back(curr);
        curr = parent[curr];
    }
    path.push_back(src);
    std::reverse(path.begin(), path.end());

    return true;
}

// -------------------------------------------------------------
// BFS: All reachable stops from a source stop
// -------------------------------------------------------------
void Graph::bfsAllReachable(int src, int maxHops,
                            std::vector<std::pair<int, int>>& results,
                            bool visualize) const {
    results.clear();
    if (stops.find(src) == stops.end()) return;

    std::queue<std::pair<int, int>> q; // (stopId, hopDistance)
    std::map<int, bool> visited;

    q.push({src, 0});
    visited[src] = true;

    if (visualize) {
        std::cout << "\n" << Color::BRIGHT_CYAN
                  << "─── BFS REACHABILITY EXPLORATION FROM: "
                  << Color::BOLD << stops.at(src).name
                  << Color::RESET << Color::BRIGHT_CYAN << " ───" << Color::RESET << "\n\n";
    }

    while (!q.empty()) {
        auto current = q.front();
        q.pop();

        int currId = current.first;
        int hops = current.second;

        results.push_back({currId, hops});

        if (maxHops != -1 && hops >= maxHops) {
            continue;
        }

        auto it = adjList.find(currId);
        if (it != adjList.end()) {
            for (int neighbor : it->second) {
                if (isRoadBlocked(currId, neighbor)) continue;

                if (!visited[neighbor]) {
                    visited[neighbor] = true;
                    q.push({neighbor, hops + 1});

                    if (visualize) {
                        std::cout << Color::GREEN << "  [Hop " << (hops + 1) << "] "
                                  << Color::RESET << stops.at(neighbor).name
                                  << Color::DIM << " (via " << stops.at(currId).name << ")"
                                  << Color::RESET << "\n";
                        animatedSleep(g_animConfig.fastDelayMs);
                    }
                }
            }
        }
    }
}

// -------------------------------------------------------------
// BFS: Minimum Bus Transfers on Bus-Route Transit Model
// -------------------------------------------------------------
// In this model, state is (stopId). In 1 step of BFS, a passenger can take any bus
// passing through current stop and alight at ANY stop served by that bus.
// BFS on this state space finds the route with the MINIMUM number of bus transfers.
bool Graph::bfsMinimumBusTransfers(int src, int dest,
                                   const std::vector<Bus>& buses,
                                   std::vector<JourneyLeg>& journey,
                                   bool visualize) const {
    journey.clear();
    if (src == dest) return true;

    // Struct to track BFS transit state
    struct TransferState {
        int stopId;
        int parentStopId;
        int busIdUsed;
    };

    std::queue<int> q;
    std::map<int, bool> visitedStops;
    std::map<int, std::pair<int, int>> backtracker; // stopId -> {parentStopId, busId}

    q.push(src);
    visitedStops[src] = true;

    if (visualize) {
        std::cout << "\n" << Color::BRIGHT_MAGENTA
                  << "─── BFS MINIMUM BUS TRANSFER FINDER ───" << Color::RESET << "\n";
        std::cout << Color::DIM
                  << "Exploring bus lines level-by-level to guarantee minimum vehicle switches."
                  << Color::RESET << "\n\n";
    }

    bool reached = false;

    while (!q.empty()) {
        int currStop = q.front();
        q.pop();

        if (currStop == dest) {
            reached = true;
            break;
        }

        // Find all buses passing through currStop
        for (const auto& bus : buses) {
            // Check if bus visits currStop
            auto itStop = std::find(bus.route.begin(), bus.route.end(), currStop);
            if (itStop == bus.route.end()) continue;

            // From currStop, this bus can reach all other stops on its route
            for (int targetStop : bus.route) {
                if (targetStop == currStop) continue;

                if (!visitedStops[targetStop]) {
                    visitedStops[targetStop] = true;
                    backtracker[targetStop] = {currStop, bus.id};
                    q.push(targetStop);

                    if (visualize) {
                        std::cout << Color::CYAN << "  Board Bus [" << bus.name << "] at "
                                  << Color::BOLD << stops.at(currStop).name << Color::RESET
                                  << " -> Can reach: " << Color::GREEN
                                  << stops.at(targetStop).name << Color::RESET << "\n";
                        animatedSleep(g_animConfig.fastDelayMs);
                    }

                    if (targetStop == dest) {
                        reached = true;
                        break;
                    }
                }
            }
            if (reached) break;
        }
    }

    if (!reached) return false;

    // Reconstruct journey legs by backtracking
    int curr = dest;
    std::vector<JourneyLeg> reversedLegs;

    while (curr != src) {
        int parent = backtracker[curr].first;
        int busId = backtracker[curr].second;

        // Find the bus
        const Bus* busPtr = nullptr;
        for (const auto& b : buses) {
            if (b.id == busId) { busPtr = &b; break; }
        }

        JourneyLeg leg;
        leg.busId = busId;
        leg.busName = (busPtr ? busPtr->name : "Transit Bus");
        leg.busType = (busPtr ? busPtr->type : "Regular");
        leg.boardStopId = parent;
        leg.alightStopId = curr;
        if (busPtr) {
            leg.stopsInLeg = busPtr->getSubRoute(parent, curr);
        } else {
            leg.stopsInLeg = {parent, curr};
        }

        reversedLegs.push_back(leg);
        curr = parent;
    }

    journey.assign(reversedLegs.rbegin(), reversedLegs.rend());
    return true;
}

// -------------------------------------------------------------
// DFS: Recursive Network Explorer
// -------------------------------------------------------------
void Graph::dfsUtil(int current, std::set<int>& visited, std::vector<int>& order,
                    int depth, bool visualize) const {
    visited.insert(current);
    order.push_back(current);

    if (visualize) {
        UI::displayStackState(current, depth, false, *this);
        animatedSleep(g_animConfig.stepDelayMs);
    }

    auto it = adjList.find(current);
    if (it != adjList.end()) {
        for (int neighbor : it->second) {
            if (isRoadBlocked(current, neighbor)) {
                if (visualize) {
                    std::string indent(depth * 3 + 2, ' ');
                    std::cout << indent << Color::BRIGHT_RED
                              << "[BLOCKED ROAD] " << stops.at(current).name
                              << " <-> " << stops.at(neighbor).name << Color::RESET << "\n";
                }
                continue;
            }

            if (visited.find(neighbor) == visited.end()) {
                dfsUtil(neighbor, visited, order, depth + 1, visualize);

                if (visualize) {
                    // Backtracking visual notice
                    UI::displayStackState(current, depth, true, *this);
                    animatedSleep(g_animConfig.fastDelayMs);
                }
            }
        }
    }
}

void Graph::dfsExplore(int src, std::vector<int>& visitOrder, bool visualize) const {
    visitOrder.clear();
    if (stops.find(src) == stops.end()) return;

    std::set<int> visited;
    if (visualize) {
        std::cout << "\n" << Color::BRIGHT_YELLOW
                  << "─── DFS NETWORK EXPLORATION TRACE (DEPTH RECURSION) ───"
                  << Color::RESET << "\n";
        std::cout << Color::DIM
                  << "Deeply explores corridors until dead-ends, then backtracks."
                  << Color::RESET << "\n\n";
    }

    dfsUtil(src, visited, visitOrder, 0, visualize);
}

// -------------------------------------------------------------
// DFS: Connected Components & Isolated Stops Detection
// -------------------------------------------------------------
void Graph::dfsComponentUtil(int current, std::set<int>& visited, std::vector<int>& comp) const {
    visited.insert(current);
    comp.push_back(current);

    auto it = adjList.find(current);
    if (it != adjList.end()) {
        for (int neighbor : it->second) {
            if (!isRoadBlocked(current, neighbor) && visited.find(neighbor) == visited.end()) {
                dfsComponentUtil(neighbor, visited, comp);
            }
        }
    }
}

std::vector<std::vector<int>> Graph::dfsFindConnectedComponents() const {
    std::vector<std::vector<int>> components;
    std::set<int> visited;

    for (const auto& pair : stops) {
        int stopId = pair.first;
        if (visited.find(stopId) == visited.end()) {
            std::vector<int> currentComp;
            dfsComponentUtil(stopId, visited, currentComp);
            components.push_back(currentComp);
        }
    }
    return components;
}

// -------------------------------------------------------------
// DFS: Road Closure Impact Analysis
// -------------------------------------------------------------
RoadClosureAnalysis Graph::dfsAnalyzeClosureImpact(int u, int v, const std::vector<Bus>& /*buses*/) const {
    RoadClosureAnalysis report;
    report.u = u;
    report.v = v;
    report.nameU = (getStop(u) ? getStop(u)->name : "Unknown");
    report.nameV = (getStop(v) ? getStop(v)->name : "Unknown");

    // 1. Connectivity before closure
    auto compsBefore = dfsFindConnectedComponents();
    report.componentsBefore = static_cast<int>(compsBefore.size());

    // Check if u and v were originally in the same component
    report.wasConnectedBefore = false;
    for (const auto& comp : compsBefore) {
        bool hasU = (std::find(comp.begin(), comp.end(), u) != comp.end());
        bool hasV = (std::find(comp.begin(), comp.end(), v) != comp.end());
        if (hasU && hasV) {
            report.wasConnectedBefore = true;
            break;
        }
    }

    // 2. Temporarily simulate closure
    bool alreadyBlocked = isRoadBlocked(u, v);
    const_cast<Graph*>(this)->blockRoad(u, v);

    // 3. Connectivity after closure
    auto compsAfter = dfsFindConnectedComponents();
    report.componentsAfter = static_cast<int>(compsAfter.size());

    report.isConnectedAfter = false;
    for (const auto& comp : compsAfter) {
        bool hasU = (std::find(comp.begin(), comp.end(), u) != comp.end());
        bool hasV = (std::find(comp.begin(), comp.end(), v) != comp.end());
        if (hasU && hasV) {
            report.isConnectedAfter = true;
            break;
        }
    }

    // If components increased, (u, v) is a bridge (critical transit corridor)
    report.isCriticalBridge = (report.componentsAfter > report.componentsBefore);

    // Find alternative path using BFS if still connected
    if (report.isConnectedAfter) {
        std::vector<int> altPath;
        bfsShortestRoute(u, v, altPath, false);
        report.alternativeRoute = altPath;
    }

    // 4. Restore original state
    if (!alreadyBlocked) {
        const_cast<Graph*>(this)->unblockRoad(u, v);
    }

    return report;
}


#ifndef GRAPH_H
#define GRAPH_H

#include "Common.h"
#include "Stop.h"
#include "Road.h"
#include "Bus.h"

// Report structure for road closure analysis
struct RoadClosureAnalysis {
    int u, v;
    std::string nameU, nameV;
    bool wasConnectedBefore;
    bool isConnectedAfter;
    int componentsBefore;
    int componentsAfter;
    bool isCriticalBridge;
    std::vector<int> alternativeRoute;
};

class Graph {
private:
    std::map<int, Stop> stops;
    std::map<int, std::vector<int>> adjList;
    std::map<std::pair<int, int>, Road> roads;
    std::set<std::pair<int, int>> blockedRoads;

    // Helper to normalize edge representation
    std::pair<int, int> makeEdgeKey(int u, int v) const {
        return (u < v) ? std::make_pair(u, v) : std::make_pair(v, u);
    }

    // Recursive helper for DFS traversal
    void dfsUtil(int current, std::set<int>& visited, std::vector<int>& order,
                 int depth, bool visualize) const;

    // Helper for finding components
    void dfsComponentUtil(int current, std::set<int>& visited, std::vector<int>& comp) const;

public:
    Graph();

    // Stop Management
    void addStop(const Stop& stop);
    bool removeStop(int stopId);
    bool updateStop(int stopId, const std::string& newName, const std::string& newZone);
    const Stop* getStop(int id) const;
    const std::map<int, Stop>& getAllStops() const;
    int getStopIdByName(const std::string& name) const;

    // Road Management
    void addRoad(const Road& road);
    bool removeRoad(int u, int v);
    bool blockRoad(int u, int v);
    bool unblockRoad(int u, int v);
    bool isRoadBlocked(int u, int v) const;
    const std::map<std::pair<int, int>, Road>& getAllRoads() const;
    const Road* getRoad(int u, int v) const;
    const std::set<std::pair<int, int>>& getBlockedRoads() const;

    // BFS Algorithms
    // 1. Shortest path by minimum connections (hops)
    bool bfsShortestRoute(int src, int dest, std::vector<int>& path, bool visualize = true) const;

    // 2. All reachable stops from a source stop
    void bfsAllReachable(int src, int maxHops,
                         std::vector<std::pair<int, int>>& results,
                         bool visualize = true) const;

    // 3. Minimum bus transfers journey using BFS on bus-route transit network
    bool bfsMinimumBusTransfers(int src, int dest,
                                const std::vector<Bus>& buses,
                                std::vector<JourneyLeg>& journey,
                                bool visualize = true) const;

    // DFS Algorithms
    // 1. Explore connected stops starting from src
    void dfsExplore(int src, std::vector<int>& visitOrder, bool visualize = true) const;

    // 2. Connected components detection (finds isolated/disconnected networks)
    std::vector<std::vector<int>> dfsFindConnectedComponents() const;

    // 3. Road closure impact analysis using DFS
    RoadClosureAnalysis dfsAnalyzeClosureImpact(int u, int v, const std::vector<Bus>& buses) const;

    // Helper to calculate total distance and estimated time for a path of stop IDs
    void calculatePathMetrics(const std::vector<int>& path, double& totalDistKm, int& totalTimeMin) const;
};

#endif // GRAPH_H


#ifndef BUS_MANAGER_H
#define BUS_MANAGER_H

#include "Common.h"
#include "Bus.h"
#include "Graph.h"

// Comparison recommendation structure
struct BusRecommendation {
    Bus bus;
    int boardingStopId;
    int destinationStopId;
    std::vector<int> traversedStops;
    int numStops;
    double estimatedDistanceKm;
    int estimatedTimeMin;
    std::string highlightReason; // e.g., "Fastest Time", "Fewest Stops", "Frequent Service"
};

class BusManager {
private:
    std::vector<Bus> buses;

public:
    BusManager();

    // Bus Management
    void addBus(const Bus& bus);
    bool removeBus(int busId);
    bool updateBus(int busId, const std::string& name, const std::string& type,
                   int freq, int fare, const std::vector<int>& route);
    const Bus* getBusById(int id) const;
    const std::vector<Bus>& getAllBuses() const;

    // Direct Bus Search
    std::vector<Bus> findDirectBuses(int src, int dest) const;

    // Bus Recommendation System (Top 3 suitable buses)
    // Uses predefined bus metrics and road distances (honestly framed, NOT Dijkstra!)
    std::vector<BusRecommendation> recommendBuses(int src, int dest, const Graph& graph) const;
};

#endif // BUS_MANAGER_H


#ifndef DATA_MANAGER_H
#define DATA_MANAGER_H

#include "Common.h"
#include "Graph.h"
#include "BusManager.h"

class DataManager {
private:
    std::string stopsFilePath;
    std::string roadsFilePath;
    std::string busesFilePath;

public:
    DataManager(const std::string& stopsPath = "data/stops.txt",
                const std::string& roadsPath = "data/roads.txt",
                const std::string& busesPath = "data/buses.txt");

    // Load functions
    bool loadStops(Graph& graph);
    bool loadRoads(Graph& graph);
    bool loadBuses(BusManager& busMgr);
    bool loadAll(Graph& graph, BusManager& busMgr);

    // Save functions
    bool saveStops(const Graph& graph);
    bool saveRoads(const Graph& graph);
    bool saveBuses(const BusManager& busMgr);
    bool saveAll(const Graph& graph, const BusManager& busMgr);

    // Create default files if missing
    void ensureDefaultFilesExist(Graph& graph, BusManager& busMgr);
};

#endif // DATA_MANAGER_H


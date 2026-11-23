#ifndef APP_H
#define APP_H

#include "Common.h"
#include "Graph.h"
#include "BusManager.h"
#include "DataManager.h"
#include "UI.h"

class App {
private:
    Graph graph;
    BusManager busMgr;
    DataManager dataMgr;
    bool isRunning;

    // Submenu & action handlers
    void runJourneyFinder();
    void runBusTransferFinder();
    void runReachabilityExplorer();
    void runNetworkExplorer();
    void runRoadClosureImpact();
    void runBusRecommendations();
    void runSearchStopsAndBuses();
    void runRoadBlockSimulator();
    void runAdminMenu();
    void runSettingsMenu();

    // Admin helpers
    void adminManageStops();
    void adminManageRoads();
    void adminManageBuses();

    // Selection helpers
    int promptSelectStop(const std::string& role);
    int promptSelectBus();

public:
    App();
    void run();
};

#endif // APP_H


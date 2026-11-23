#ifndef UI_H
#define UI_H

#include "Common.h"
#include "Stop.h"
#include "Road.h"
#include "Bus.h"
#include "Graph.h"
#include "BusManager.h"

namespace UI {
    // Screen management
    void clearScreen();
    void pauseScreen();
    void printHeader(const std::string& title);
    void printBanner();
    void showLoadingAnimation(const std::string& taskName, int durationMs = 600);

    // Visualizers
    void displayQueueState(const std::vector<int>& queueElements,
                           int currentPopped,
                           const Graph& graph);

    void displayStackState(int current, int depth, bool isBacktracking, const Graph& graph);

    void displayRouteAscii(const std::vector<int>& path,
                           const Graph& graph,
                           const std::string& title = "RECOMMENDED JOURNEY ROUTE");

    void displayTransferJourney(const std::vector<JourneyLeg>& journey, const Graph& graph);

    void displayRoadClosureReport(const RoadClosureAnalysis& report, const Graph& graph);

    void displayBusRecommendations(const std::vector<BusRecommendation>& recs,
                                   int srcId, int destId,
                                   const Graph& graph);

    void displayComponents(const std::vector<std::vector<int>>& components, const Graph& graph);

    void displayReachableStops(int srcId,
                               const std::vector<std::pair<int, int>>& results,
                               const Graph& graph);

    // Helpers
    void printBoxLine(const std::string& text, int width = 72, const std::string& color = Color::CYAN);
    int readInt(const std::string& prompt, int minVal = 0, int maxVal = 999999);
    std::string readLine(const std::string& prompt);
    bool confirmAction(const std::string& prompt);
}

#endif // UI_H


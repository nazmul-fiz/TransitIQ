#ifndef GUI_APP_H
#define GUI_APP_H

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif

#include "Common.h"
#include "Graph.h"
#include "BusManager.h"
#include "DataManager.h"
#include <windows.h>
#include <commctrl.h>

struct StopCoord {
    int x;
    int y;
};

class GUIApp {
private:
    Graph graph;
    BusManager busMgr;
    DataManager dataMgr;

    HINSTANCE hInstance;
    HWND hWndMain;
    HWND hComboSrc;
    HWND hComboDest;
    HWND hComboMode;
    HWND hComboRoadBlock;
    HWND hBtnRun;
    HWND hBtnAnimate;
    HWND hBtnReset;
    HWND hEditResults;

    // 2D Coordinates for Dhaka Map
    std::map<int, StopCoord> stopCoords;

    // Current State for Visualization
    int selectedSrc;
    int selectedDest;
    int selectedMode;
    int blockedRoadOption;

    std::vector<int> currentPath;
    std::vector<JourneyLeg> currentJourney;
    std::vector<std::vector<int>> currentComponents;
    std::vector<std::pair<int, int>> currentReachable;
    RoadClosureAnalysis currentClosureReport;
    std::vector<BusRecommendation> currentRecommendations;

    // Animation state
    bool isAnimating;
    int animationStep;
    int animationTotalSteps;
    std::vector<int> animationVisitedOrder;
    std::vector<std::pair<int, int>> animationEdges;
    std::vector<std::string> animationStepLogs;

    void initCoordinates();
    void createControls(HWND hWnd);
    void populateDropdowns();
    void runSelectedAlgorithm(bool startAnimation = false);
    void resetState();
    void updateResultsText();

    // Drawing helpers
    void drawMap(HDC hdc, const RECT& mapRect);
    void drawRoad(HDC hdc, int u, int v, COLORREF color, int width, bool isDashed);
    void drawStopNode(HDC hdc, int stopId, COLORREF bgColor, COLORREF borderColor, bool isHighlight);
    void drawArrow(HDC hdc, int x1, int y1, int x2, int y2, COLORREF color);

public:
    GUIApp(HINSTANCE hInst);
    ~GUIApp();

    bool initialize();
    int run();

    static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
};

#endif // GUI_APP_H

#include "GUIApp.h"
#include <cmath>

#define ID_COMBO_SRC        101
#define ID_COMBO_DEST       102
#define ID_COMBO_MODE       103
#define ID_COMBO_BLOCK      104
#define ID_BTN_RUN          105
#define ID_BTN_ANIMATE      106
#define ID_BTN_RESET        107
#define ID_EDIT_RESULTS     108
#define ID_TIMER_ANIM       201

static GUIApp* g_pApp = nullptr;

GUIApp::GUIApp(HINSTANCE hInst)
    : hInstance(hInst), hWndMain(NULL),
      selectedSrc(1), selectedDest(22), selectedMode(0), blockedRoadOption(0),
      isAnimating(false), animationStep(0), animationTotalSteps(0) {
    g_pApp = this;
    dataMgr.ensureDefaultFilesExist(graph, busMgr);
    dataMgr.loadAll(graph, busMgr);
    initCoordinates();
}

GUIApp::~GUIApp() {
    if (g_pApp == this) g_pApp = nullptr;
}

void GUIApp::initCoordinates() {
    // Map of Dhaka Stops relative to map canvas (800 x 700)
    stopCoords[1]  = {360, 50};   // Uttara
    stopCoords[2]  = {360, 110};  // Airport
    stopCoords[3]  = {460, 140};  // Khilkhet
    stopCoords[4]  = {510, 200};  // Kuril
    stopCoords[5]  = {640, 210};  // Bashundhara
    stopCoords[6]  = {350, 220};  // Banani
    stopCoords[7]  = {450, 235};  // Gulshan-2
    stopCoords[8]  = {450, 310};  // Gulshan-1
    stopCoords[9]  = {350, 300};  // Mohakhali
    stopCoords[10] = {560, 310};  // Badda
    stopCoords[11] = {560, 390};  // Rampura
    stopCoords[12] = {500, 470};  // Malibagh
    stopCoords[13] = {410, 470};  // Moghbazar
    stopCoords[14] = {270, 400};  // Farmgate
    stopCoords[15] = {290, 470};  // Karwan-Bazar
    stopCoords[16] = {330, 540};  // Shahbagh
    stopCoords[17] = {180, 470};  // Dhanmondi
    stopCoords[18] = {220, 560};  // New-Market
    stopCoords[19] = {160, 200};  // Mirpur-10
    stopCoords[20] = {140, 280};  // Mirpur-1
    stopCoords[21] = {80,  370};  // Gabtoli
    stopCoords[22] = {430, 610};  // Motijheel
    stopCoords[23] = {560, 600};  // Sayedabad
}

bool GUIApp::initialize() {
    InitCommonControls();

    WNDCLASSEX wc;
    ZeroMemory(&wc, sizeof(wc));
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.lpfnWndProc = GUIApp::WndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    wc.lpszClassName = "TransitIQ_GUI_Class";
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);

    if (!RegisterClassEx(&wc)) return false;

    // Window size: 1240 x 760
    hWndMain = CreateWindowEx(
        WS_EX_CLIENTEDGE,
        "TransitIQ_GUI_Class",
        "TransitIQ - Smart Bus Network Explorer & Journey Finder (Visual GUI)",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT,
        1240, 760,
        NULL, NULL, hInstance, NULL
    );

    if (!hWndMain) return false;

    createControls(hWndMain);
    populateDropdowns();
    resetState();

    ShowWindow(hWndMain, SW_SHOW);
    UpdateWindow(hWndMain);

    return true;
}

void GUIApp::createControls(HWND hWnd) {
    HFONT hFont = CreateFont(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                             DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                             DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");

    HFONT hFontBold = CreateFont(18, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                                 DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                 DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");

    // Title Header
    HWND hTitle = CreateWindow("STATIC", "TransitIQ Visual Control Panel",
                               WS_CHILD | WS_VISIBLE | SS_LEFT,
                               20, 15, 360, 25, hWnd, NULL, hInstance, NULL);
    SendMessage(hTitle, WM_SETFONT, (WPARAM)hFontBold, TRUE);

    HWND hSub = CreateWindow("STATIC", "Select options below to explore BFS & DFS visually:",
                             WS_CHILD | WS_VISIBLE | SS_LEFT,
                             20, 42, 360, 20, hWnd, NULL, hInstance, NULL);
    SendMessage(hSub, WM_SETFONT, (WPARAM)hFont, TRUE);

    // 1. Origin Stop
    HWND hLbl1 = CreateWindow("STATIC", "Boarding / Origin Stop:",
                              WS_CHILD | WS_VISIBLE | SS_LEFT,
                              20, 72, 360, 20, hWnd, NULL, hInstance, NULL);
    SendMessage(hLbl1, WM_SETFONT, (WPARAM)hFont, TRUE);

    hComboSrc = CreateWindow("COMBOBOX", "",
                             WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL,
                             20, 95, 360, 220, hWnd, (HMENU)ID_COMBO_SRC, hInstance, NULL);
    SendMessage(hComboSrc, WM_SETFONT, (WPARAM)hFont, TRUE);

    // 2. Destination Stop
    HWND hLbl2 = CreateWindow("STATIC", "Destination / Alighting Stop:",
                              WS_CHILD | WS_VISIBLE | SS_LEFT,
                              20, 130, 360, 20, hWnd, NULL, hInstance, NULL);
    SendMessage(hLbl2, WM_SETFONT, (WPARAM)hFont, TRUE);

    hComboDest = CreateWindow("COMBOBOX", "",
                              WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL,
                              20, 153, 360, 220, hWnd, (HMENU)ID_COMBO_DEST, hInstance, NULL);
    SendMessage(hComboDest, WM_SETFONT, (WPARAM)hFont, TRUE);

    // 3. Algorithm Mode
    HWND hLbl3 = CreateWindow("STATIC", "Algorithm / Exploration Mode:",
                              WS_CHILD | WS_VISIBLE | SS_LEFT,
                              20, 188, 360, 20, hWnd, NULL, hInstance, NULL);
    SendMessage(hLbl3, WM_SETFONT, (WPARAM)hFont, TRUE);

    hComboMode = CreateWindow("COMBOBOX", "",
                              WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL,
                              20, 211, 360, 220, hWnd, (HMENU)ID_COMBO_MODE, hInstance, NULL);
    SendMessage(hComboMode, WM_SETFONT, (WPARAM)hFont, TRUE);

    // 4. Simulate Road Block
    HWND hLbl4 = CreateWindow("STATIC", "Simulate Road Block / Closure:",
                              WS_CHILD | WS_VISIBLE | SS_LEFT,
                              20, 246, 360, 20, hWnd, NULL, hInstance, NULL);
    SendMessage(hLbl4, WM_SETFONT, (WPARAM)hFont, TRUE);

    hComboRoadBlock = CreateWindow("COMBOBOX", "",
                                  WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL,
                                  20, 269, 360, 220, hWnd, (HMENU)ID_COMBO_BLOCK, hInstance, NULL);
    SendMessage(hComboRoadBlock, WM_SETFONT, (WPARAM)hFont, TRUE);

    // 5. Buttons
    hBtnRun = CreateWindow("BUTTON", "🚀 Run & Show Visuals",
                           WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
                           20, 310, 175, 38, hWnd, (HMENU)ID_BTN_RUN, hInstance, NULL);
    SendMessage(hBtnRun, WM_SETFONT, (WPARAM)hFontBold, TRUE);

    hBtnAnimate = CreateWindow("BUTTON", "▶️ Animate Steps",
                               WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                               205, 310, 175, 38, hWnd, (HMENU)ID_BTN_ANIMATE, hInstance, NULL);
    SendMessage(hBtnAnimate, WM_SETFONT, (WPARAM)hFontBold, TRUE);

    hBtnReset = CreateWindow("BUTTON", "🔄 Reset Network & View",
                             WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                             20, 355, 360, 30, hWnd, (HMENU)ID_BTN_RESET, hInstance, NULL);
    SendMessage(hBtnReset, WM_SETFONT, (WPARAM)hFont, TRUE);

    // 6. Results Text Box
    HWND hLblRes = CreateWindow("STATIC", "Algorithm Trace & Journey Summary:",
                                WS_CHILD | WS_VISIBLE | SS_LEFT,
                                20, 395, 360, 20, hWnd, NULL, hInstance, NULL);
    SendMessage(hLblRes, WM_SETFONT, (WPARAM)hFontBold, TRUE);

    hEditResults = CreateWindowEx(
        WS_EX_CLIENTEDGE,
        "EDIT", "",
        WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY,
        20, 420, 360, 280, hWnd, (HMENU)ID_EDIT_RESULTS, hInstance, NULL
    );
    SendMessage(hEditResults, WM_SETFONT, (WPARAM)hFont, TRUE);
}

void GUIApp::populateDropdowns() {
    // Populate Stops in Source and Destination combos
    SendMessage(hComboSrc, CB_RESETCONTENT, 0, 0);
    SendMessage(hComboDest, CB_RESETCONTENT, 0, 0);

    for (const auto& pair : graph.getAllStops()) {
        std::string item = std::to_string(pair.first) + ". " + pair.second.name + " (" + pair.second.zone + ")";
        SendMessage(hComboSrc, CB_ADDSTRING, 0, (LPARAM)item.c_str());
        SendMessage(hComboDest, CB_ADDSTRING, 0, (LPARAM)item.c_str());
    }

    // Default: 1 (Uttara) and 22 (Motijheel)
    SendMessage(hComboSrc, CB_SETCURSEL, 0, 0);
    SendMessage(hComboDest, CB_SETCURSEL, 21, 0);

    // Populate Modes
    SendMessage(hComboMode, CB_RESETCONTENT, 0, 0);
    SendMessage(hComboMode, CB_ADDSTRING, 0, (LPARAM)"1. BFS Journey Finder (Shortest Connections)");
    SendMessage(hComboMode, CB_ADDSTRING, 0, (LPARAM)"2. BFS Bus Transfer Finder (Min Bus Switches)");
    SendMessage(hComboMode, CB_ADDSTRING, 0, (LPARAM)"3. BFS Reachability Explorer (Hop Levels)");
    SendMessage(hComboMode, CB_ADDSTRING, 0, (LPARAM)"4. DFS Network Explorer (Corridor Walk)");
    SendMessage(hComboMode, CB_ADDSTRING, 0, (LPARAM)"5. DFS Components & Isolated Stops");
    SendMessage(hComboMode, CB_ADDSTRING, 0, (LPARAM)"6. DFS Road Closure & Bottleneck Bridge Impact");
    SendMessage(hComboMode, CB_ADDSTRING, 0, (LPARAM)"7. Bus Recommendation (Top 3 Direct/Transfer)");
    SendMessage(hComboMode, CB_SETCURSEL, 0, 0);

    // Populate Road Block Options
    SendMessage(hComboRoadBlock, CB_RESETCONTENT, 0, 0);
    SendMessage(hComboRoadBlock, CB_ADDSTRING, 0, (LPARAM)"[None] All Roads Fully Open");
    SendMessage(hComboRoadBlock, CB_ADDSTRING, 0, (LPARAM)"[Block] Farmgate <-> Karwan-Bazar (14-15)");
    SendMessage(hComboRoadBlock, CB_ADDSTRING, 0, (LPARAM)"[Block] Airport <-> Khilkhet (2-3)");
    SendMessage(hComboRoadBlock, CB_ADDSTRING, 0, (LPARAM)"[Block] Kuril <-> Banani (4-6)");
    SendMessage(hComboRoadBlock, CB_ADDSTRING, 0, (LPARAM)"[Block] Mohakhali <-> Farmgate (9-14)");
    SendMessage(hComboRoadBlock, CB_ADDSTRING, 0, (LPARAM)"[Block] Mirpur-10 <-> Banani (19-6)");
    SendMessage(hComboRoadBlock, CB_ADDSTRING, 0, (LPARAM)"[Block] Malibagh <-> Sayedabad (12-23)");
    SendMessage(hComboRoadBlock, CB_ADDSTRING, 0, (LPARAM)"[Block] Dhanmondi <-> New-Market (17-18)");
    SendMessage(hComboRoadBlock, CB_SETCURSEL, 0, 0);
}

void GUIApp::resetState() {
    isAnimating = false;
    KillTimer(hWndMain, ID_TIMER_ANIM);

    currentPath.clear();
    currentJourney.clear();
    currentComponents.clear();
    currentReachable.clear();
    currentRecommendations.clear();
    animationVisitedOrder.clear();
    animationEdges.clear();
    animationStepLogs.clear();
    animationStep = 0;
    animationTotalSteps = 0;

    // Clear blocked roads in graph
    auto blocked = graph.getBlockedRoads();
    for (const auto& edge : blocked) {
        graph.unblockRoad(edge.first, edge.second);
    }

    SendMessage(hComboRoadBlock, CB_SETCURSEL, 0, 0);

    std::string initText =
        "Welcome to TransitIQ!\r\n\r\n"
        "• Network: 23 Dhaka Stops, 30 Roads, 8 Buses.\r\n"
        "• Select Origin and Destination from the dropdowns.\r\n"
        "• Choose an algorithm mode (BFS / DFS).\r\n"
        "• Click [Run & Show Visuals] to highlight paths,\r\n"
        "  or [Animate Steps] for step-by-step queue/stack trace.\r\n"
        "• Try blocking roads to watch BFS find live detours!";

    SetWindowText(hEditResults, initText.c_str());

    InvalidateRect(hWndMain, NULL, TRUE);
}

void GUIApp::runSelectedAlgorithm(bool startAnimation) {
    // 1. Get selected options
    int srcIdx = (int)SendMessage(hComboSrc, CB_GETCURSEL, 0, 0);
    int destIdx = (int)SendMessage(hComboDest, CB_GETCURSEL, 0, 0);
    selectedMode = (int)SendMessage(hComboMode, CB_GETCURSEL, 0, 0);
    blockedRoadOption = (int)SendMessage(hComboRoadBlock, CB_GETCURSEL, 0, 0);

    // Map combo index to stop ID (1-based IDs from graph stops)
    int i = 0;
    for (const auto& pair : graph.getAllStops()) {
        if (i == srcIdx) selectedSrc = pair.first;
        if (i == destIdx) selectedDest = pair.first;
        i++;
    }

    // 2. Clear previous road blocks and apply current selected block
    auto blocked = graph.getBlockedRoads();
    for (const auto& edge : blocked) {
        graph.unblockRoad(edge.first, edge.second);
    }

    switch (blockedRoadOption) {
        case 1: graph.blockRoad(14, 15); break; // Farmgate <-> Karwan-Bazar
        case 2: graph.blockRoad(2, 3);   break; // Airport <-> Khilkhet
        case 3: graph.blockRoad(4, 6);   break; // Kuril <-> Banani
        case 4: graph.blockRoad(9, 14);  break; // Mohakhali <-> Farmgate
        case 5: graph.blockRoad(19, 6);  break; // Mirpur-10 <-> Banani
        case 6: graph.blockRoad(12, 23); break; // Malibagh <-> Sayedabad
        case 7: graph.blockRoad(17, 18); break; // Dhanmondi <-> New-Market
        default: break;
    }

    currentPath.clear();
    currentJourney.clear();
    currentComponents.clear();
    currentReachable.clear();
    currentRecommendations.clear();
    animationVisitedOrder.clear();
    animationEdges.clear();
    animationStepLogs.clear();

    // 3. Execute selected algorithm
    if (selectedMode == 0) {
        // Mode 1: BFS Journey Finder
        graph.bfsShortestRoute(selectedSrc, selectedDest, currentPath, false);

        // Prepare animation steps (simulate BFS queue steps)
        std::queue<int> q;
        std::map<int, bool> vis;
        q.push(selectedSrc);
        vis[selectedSrc] = true;

        while (!q.empty()) {
            int curr = q.front();
            q.pop();
            animationVisitedOrder.push_back(curr);
            animationStepLogs.push_back("BFS Expanding: " + graph.getStop(curr)->name);

            if (curr == selectedDest) break;

            const auto* s = graph.getStop(curr);
            if (!s) continue;

            for (const auto& p : graph.getAllRoads()) {
                int u = p.second.u, v = p.second.v;
                if ((u == curr || v == curr) && !graph.isRoadBlocked(u, v)) {
                    int neighbor = (u == curr ? v : u);
                    if (!vis[neighbor]) {
                        vis[neighbor] = true;
                        q.push(neighbor);
                        animationEdges.push_back({curr, neighbor});
                    }
                }
            }
        }
    } else if (selectedMode == 1) {
        // Mode 2: BFS Bus Transfer Finder
        graph.bfsMinimumBusTransfers(selectedSrc, selectedDest, busMgr.getAllBuses(), currentJourney, false);
    } else if (selectedMode == 2) {
        // Mode 3: BFS Reachability
        graph.bfsAllReachable(selectedSrc, -1, currentReachable, false);
    } else if (selectedMode == 3) {
        // Mode 4: DFS Network Explorer
        graph.dfsExplore(selectedSrc, animationVisitedOrder, false);
    } else if (selectedMode == 4) {
        // Mode 5: DFS Components & Isolated Stops
        currentComponents = graph.dfsFindConnectedComponents();
    } else if (selectedMode == 5) {
        // Mode 6: DFS Road Closure Impact
        currentClosureReport = graph.dfsAnalyzeClosureImpact(selectedSrc, selectedDest, busMgr.getAllBuses());
    } else if (selectedMode == 6) {
        // Mode 7: Bus Recommendations
        currentRecommendations = busMgr.recommendBuses(selectedSrc, selectedDest, graph);
    }

    updateResultsText();

    if (startAnimation) {
        isAnimating = true;
        animationStep = 0;
        animationTotalSteps = (int)animationVisitedOrder.size();
        SetTimer(hWndMain, ID_TIMER_ANIM, 350, NULL);
    } else {
        isAnimating = false;
        KillTimer(hWndMain, ID_TIMER_ANIM);
    }

    InvalidateRect(hWndMain, NULL, TRUE);
}

void GUIApp::updateResultsText() {
    std::string text = "";
    std::string srcName = (graph.getStop(selectedSrc) ? graph.getStop(selectedSrc)->name : "Origin");
    std::string destName = (graph.getStop(selectedDest) ? graph.getStop(selectedDest)->name : "Destination");

    if (selectedMode == 0) {
        // BFS Journey Finder
        text += "── BFS JOURNEY FINDER ──\r\n";
        text += "Route from: " + srcName + " -> " + destName + "\r\n";
        if (graph.getBlockedRoads().size() > 0) {
            text += "[!] Active Road Block in place (BFS finding detour)\r\n";
        }
        text += "\r\n";

        if (currentPath.empty()) {
            text += "NO AVAILABLE ROUTE FOUND!\r\n(Road closure disconnected this corridor)\r\n";
        } else {
            double totalDist = 0;
            int totalTime = 0;
            graph.calculatePathMetrics(currentPath, totalDist, totalTime);

            text += "Status: Optimal Route Found!\r\n";
            text += "Total Stops: " + std::to_string(currentPath.size()) + "\r\n";
            text += "Transit Hops: " + std::to_string(currentPath.size() - 1) + "\r\n";
            text += "Est. Distance: " + std::to_string((int)totalDist) + " km\r\n";
            text += "Est. Travel Time: ~" + std::to_string(totalTime) + " mins\r\n\r\n";
            text += "Route Sequence:\r\n";
            for (size_t k = 0; k < currentPath.size(); ++k) {
                text += std::to_string(k + 1) + ". " + graph.getStop(currentPath[k])->name + "\r\n";
            }
        }
    } else if (selectedMode == 1) {
        // Bus Transfer Finder
        text += "── BFS MINIMUM BUS TRANSFERS ──\r\n";
        text += "From: " + srcName + " -> " + destName + "\r\n\r\n";

        if (currentJourney.empty()) {
            text += "No bus route combination connects these stops.\r\n";
        } else {
            text += "Transfers Required: " + std::to_string(currentJourney.size() - 1) + "\r\n";
            text += "Total Buses: " + std::to_string(currentJourney.size()) + "\r\n\r\n";
            for (size_t leg = 0; leg < currentJourney.size(); ++leg) {
                const auto& l = currentJourney[leg];
                text += "Leg " + std::to_string(leg + 1) + ": " + l.busName + " [" + l.busType + "]\r\n";
                text += "  Board:  " + graph.getStop(l.boardStopId)->name + "\r\n";
                text += "  Alight: " + graph.getStop(l.alightStopId)->name + "\r\n";
                if (leg + 1 < currentJourney.size()) {
                    text += "  >> Switch bus at " + graph.getStop(l.alightStopId)->name + "\r\n";
                }
                text += "\r\n";
            }
        }
    } else if (selectedMode == 2) {
        // Reachability
        text += "── BFS REACHABILITY EXPLORER ──\r\n";
        text += "Starting From: " + srcName + "\r\n";
        text += "Total Reachable Stops: " + std::to_string(currentReachable.size()) + "\r\n\r\n";
        for (const auto& item : currentReachable) {
            text += "• Level " + std::to_string(item.second) + " hop: "
                 + graph.getStop(item.first)->name + "\r\n";
        }
    } else if (selectedMode == 3) {
        // DFS Explorer
        text += "── DFS NETWORK EXPLORER ──\r\n";
        text += "Deep Corridor Walk from: " + srcName + "\r\n";
        text += "Stops Reached: " + std::to_string(animationVisitedOrder.size()) + "\r\n\r\n";
        text += "Traversal Order:\r\n";
        for (size_t k = 0; k < animationVisitedOrder.size(); ++k) {
            text += std::to_string(k + 1) + ". " + graph.getStop(animationVisitedOrder[k])->name + "\r\n";
        }
    } else if (selectedMode == 4) {
        // Connected Components
        text += "── DFS CONNECTED COMPONENTS ──\r\n";
        text += "Total Subgraphs: " + std::to_string(currentComponents.size()) + "\r\n\r\n";
        int iso = 0;
        for (size_t c = 0; c < currentComponents.size(); ++c) {
            if (currentComponents[c].size() == 1) {
                iso++;
                text += "⚠️ Isolated Stop: " + graph.getStop(currentComponents[c][0])->name + "\r\n";
            } else {
                text += "Component #" + std::to_string(c + 1) + " ("
                     + std::to_string(currentComponents[c].size()) + " stops)\r\n";
            }
        }
        if (iso == 0) text += "\r\n✓ Full Network is connected.\r\n";
    } else if (selectedMode == 5) {
        // DFS Closure Impact
        text += "── DFS CLOSURE IMPACT ANALYSIS ──\r\n";
        text += "Analyzed Road: " + currentClosureReport.nameU + " <-> " + currentClosureReport.nameV + "\r\n\r\n";
        text += "Components Before: " + std::to_string(currentClosureReport.componentsBefore) + "\r\n";
        text += "Components After:  " + std::to_string(currentClosureReport.componentsAfter) + "\r\n\r\n";
        if (currentClosureReport.isCriticalBridge) {
            text += "CRITICAL BOTTLENECK / BRIDGE!\r\n";
            text += "Closing this road splits Dhaka transit into "
                 + std::to_string(currentClosureReport.componentsAfter) + " disconnected parts!\r\n";
        } else {
            text += "Network remains connected.\r\nRedundant alternative routes exist.\r\n";
        }
    } else if (selectedMode == 6) {
        // Bus Recommendations
        text += "── BUS RECOMMENDATIONS (TOP 3) ──\r\n";
        text += "Trip: " + srcName + " -> " + destName + "\r\n\r\n";
        if (currentRecommendations.empty()) {
            text += "No suitable buses found for this trip.\r\n";
        } else {
            for (size_t r = 0; r < currentRecommendations.size(); ++r) {
                const auto& rec = currentRecommendations[r];
                std::string medal = (r == 0 ? "1. TOP CHOICE: " : (r == 1 ? "2. RUNNER UP: " : "3. OPTION: "));
                text += medal + rec.bus.name + " (" + rec.bus.type + ")\r\n";
                text += "   Highlight: " + rec.highlightReason + "\r\n";
                text += "   Time: ~" + std::to_string(rec.estimatedTimeMin) + "m | Dist: "
                     + std::to_string((int)rec.estimatedDistanceKm) + "km\r\n";
                text += "   Fare: " + std::to_string(rec.bus.baseFare) + " BDT | Freq: "
                     + std::to_string(rec.bus.frequencyMin) + "m\r\n\r\n";
            }
        }
    }

    SetWindowText(hEditResults, text.c_str());
}

// -------------------------------------------------------------
// Drawing Engine
// -------------------------------------------------------------
void GUIApp::drawMap(HDC hdc, const RECT& mapRect) {
    // 1. Double Buffer to prevent flickering
    int mapWidth = mapRect.right - mapRect.left;
    int mapHeight = mapRect.bottom - mapRect.top;

    HDC memDC = CreateCompatibleDC(hdc);
    HBITMAP memBitmap = CreateCompatibleBitmap(hdc, mapWidth, mapHeight);
    HBITMAP oldBitmap = (HBITMAP)SelectObject(memDC, memBitmap);

    // Dark Background (Modern Slate)
    HBRUSH bgBrush = CreateSolidBrush(RGB(18, 24, 38));
    RECT localRect = {0, 0, mapWidth, mapHeight};
    FillRect(memDC, &localRect, bgBrush);
    DeleteObject(bgBrush);

    // Subtle Map Title / Watermark
    HFONT hTitleFont = CreateFont(22, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                                  DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                  DEFAULT_QUALITY, DEFAULT_PITCH, "Segoe UI");
    HFONT oldFont = (HFONT)SelectObject(memDC, hTitleFont);
    SetBkMode(memDC, TRANSPARENT);
    SetTextColor(memDC, RGB(45, 55, 75));
    TextOut(memDC, 20, 15, "DHAKA METROPOLITAN TRANSIT NETWORK", 35);
    DeleteObject(hTitleFont);

    // 2. Draw all Roads
    for (const auto& pair : graph.getAllRoads()) {
        int u = pair.second.u;
        int v = pair.second.v;
        bool isBlocked = graph.isRoadBlocked(u, v);

        if (isBlocked) {
            // Draw thick red dashed line for blocked road
            drawRoad(memDC, u, v, RGB(239, 68, 68), 3, true);

            // Draw a small ⛔ block icon / red X at the midpoint
            if (stopCoords.find(u) != stopCoords.end() && stopCoords.find(v) != stopCoords.end()) {
                int midX = (stopCoords[u].x + stopCoords[v].x) / 2;
                int midY = (stopCoords[u].y + stopCoords[v].y) / 2;

                HBRUSH redB = CreateSolidBrush(RGB(220, 38, 38));
                HBRUSH oldB = (HBRUSH)SelectObject(memDC, redB);
                Ellipse(memDC, midX - 8, midY - 8, midX + 8, midY + 8);
                SelectObject(memDC, oldB);
                DeleteObject(redB);

                HPEN whiteP = CreatePen(PS_SOLID, 2, RGB(255, 255, 255));
                HPEN oldP = (HPEN)SelectObject(memDC, whiteP);
                MoveToEx(memDC, midX - 4, midY, NULL);
                LineTo(memDC, midX + 4, midY);
                SelectObject(memDC, oldP);
                DeleteObject(whiteP);
            }
        } else {
            // Regular open road
            drawRoad(memDC, u, v, RGB(55, 65, 85), 2, false);
        }
    }

    // 3. Highlight Journey Route or Traversal
    if (selectedMode == 0 && !currentPath.empty()) {
        // BFS Path Highlight (Glowing Emerald Green)
        for (size_t i = 0; i + 1 < currentPath.size(); ++i) {
            int u = currentPath[i];
            int v = currentPath[i + 1];
            drawRoad(memDC, u, v, RGB(16, 185, 129), 5, false);

            if (stopCoords.find(u) != stopCoords.end() && stopCoords.find(v) != stopCoords.end()) {
                drawArrow(memDC, stopCoords[u].x, stopCoords[u].y, stopCoords[v].x, stopCoords[v].y, RGB(16, 185, 129));
            }
        }
    } else if (selectedMode == 1 && !currentJourney.empty()) {
        // Multi-leg Bus Transfers (Vivid Colors for Legs)
        COLORREF legColors[] = { RGB(16, 185, 129), RGB(217, 70, 239), RGB(6, 182, 212) };
        for (size_t leg = 0; leg < currentJourney.size(); ++leg) {
            COLORREF c = legColors[leg % 3];
            const auto& stops = currentJourney[leg].stopsInLeg;
            for (size_t s = 0; s + 1 < stops.size(); ++s) {
                drawRoad(memDC, stops[s], stops[s + 1], c, 5, false);
            }
        }
    } else if (selectedMode == 5 && !currentClosureReport.alternativeRoute.empty()) {
        // DFS Road Closure Bypass Route (Bright Cyan)
        for (size_t i = 0; i + 1 < currentClosureReport.alternativeRoute.size(); ++i) {
            int u = currentClosureReport.alternativeRoute[i];
            int v = currentClosureReport.alternativeRoute[i + 1];
            drawRoad(memDC, u, v, RGB(6, 182, 212), 4, false);
        }
    }

    // 4. Highlight Animation Steps if running
    if (isAnimating) {
        for (int k = 0; k < animationStep && k < (int)animationEdges.size(); ++k) {
            int u = animationEdges[k].first;
            int v = animationEdges[k].second;
            drawRoad(memDC, u, v, RGB(245, 158, 11), 3, false);
        }
    }

    // 5. Draw Stop Nodes
    std::set<int> pathStops(currentPath.begin(), currentPath.end());
    std::set<int> visitedAnim;
    if (isAnimating) {
        for (int k = 0; k <= animationStep && k < (int)animationVisitedOrder.size(); ++k) {
            visitedAnim.insert(animationVisitedOrder[k]);
        }
    }

    for (const auto& pair : graph.getAllStops()) {
        int id = pair.first;
        bool isStart = (id == selectedSrc);
        bool isDest = (id == selectedDest);
        bool inPath = (pathStops.find(id) != pathStops.end());
        bool inAnim = (visitedAnim.find(id) != visitedAnim.end());

        COLORREF nodeBg = RGB(71, 85, 105);
        COLORREF borderColor = RGB(148, 163, 184);

        if (isStart) {
            nodeBg = RGB(245, 158, 11); // Golden Yellow
            borderColor = RGB(255, 255, 255);
        } else if (isDest) {
            nodeBg = RGB(6, 182, 212);  // Bright Cyan
            borderColor = RGB(255, 255, 255);
        } else if (inPath) {
            nodeBg = RGB(16, 185, 129); // Emerald Green
            borderColor = RGB(255, 255, 255);
        } else if (inAnim) {
            nodeBg = RGB(234, 88, 12);  // Orange (visited in animation)
            borderColor = RGB(255, 255, 255);
        }

        drawStopNode(memDC, id, nodeBg, borderColor, isStart || isDest || inPath);
    }

    // 6. Draw Legend at Bottom Right
    RECT legendRect = { mapWidth - 250, mapHeight - 110, mapWidth - 15, mapHeight - 15 };
    HBRUSH legBg = CreateSolidBrush(RGB(24, 32, 48));
    FillRect(memDC, &legendRect, legBg);
    DeleteObject(legBg);

    HFONT hLegFont = CreateFont(13, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                                DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                DEFAULT_QUALITY, DEFAULT_PITCH, "Segoe UI");
    SelectObject(memDC, hLegFont);
    SetTextColor(memDC, RGB(220, 225, 235));

    // Legend items
    auto drawLegendItem = [&](int x, int y, COLORREF color, const char* label) {
        HBRUSH b = CreateSolidBrush(color);
        HBRUSH old = (HBRUSH)SelectObject(memDC, b);
        Ellipse(memDC, x, y, x + 10, y + 10);
        SelectObject(memDC, old);
        DeleteObject(b);
        TextOut(memDC, x + 16, y - 2, label, (int)strlen(label));
    };

    drawLegendItem(mapWidth - 240, mapHeight - 100, RGB(245, 158, 11), "Origin / Boarding (Start)");
    drawLegendItem(mapWidth - 240, mapHeight - 80,  RGB(6, 182, 212),  "Destination (Target)");
    drawLegendItem(mapWidth - 240, mapHeight - 60,  RGB(16, 185, 129), "Optimal Journey Path");
    drawLegendItem(mapWidth - 240, mapHeight - 40,  RGB(239, 68, 68),  "Blocked Road Corridor");

    DeleteObject(hLegFont);

    // Blit to screen
    BitBlt(hdc, mapRect.left, mapRect.top, mapWidth, mapHeight, memDC, 0, 0, SRCCOPY);

    SelectObject(memDC, oldFont);
    SelectObject(memDC, oldBitmap);
    DeleteObject(memBitmap);
    DeleteDC(memDC);
}

void GUIApp::drawRoad(HDC hdc, int u, int v, COLORREF color, int width, bool isDashed) {
    if (stopCoords.find(u) == stopCoords.end() || stopCoords.find(v) == stopCoords.end()) return;

    int x1 = stopCoords[u].x;
    int y1 = stopCoords[u].y;
    int x2 = stopCoords[v].x;
    int y2 = stopCoords[v].y;

    HPEN pen = CreatePen(isDashed ? PS_DASH : PS_SOLID, width, color);
    HPEN oldPen = (HPEN)SelectObject(hdc, pen);

    MoveToEx(hdc, x1, y1, NULL);
    LineTo(hdc, x2, y2);

    SelectObject(hdc, oldPen);
    DeleteObject(pen);
}

void GUIApp::drawArrow(HDC hdc, int x1, int y1, int x2, int y2, COLORREF color) {
    int midX = (x1 + x2) / 2;
    int midY = (y1 + y2) / 2;

    double angle = atan2((double)(y2 - y1), (double)(x2 - x1));
    double arrowLen = 10.0;
    double arrowAngle = 0.5;

    int ax1 = (int)(midX - arrowLen * cos(angle - arrowAngle));
    int ay1 = (int)(midY - arrowLen * sin(angle - arrowAngle));
    int ax2 = (int)(midX - arrowLen * cos(angle + arrowAngle));
    int ay2 = (int)(midY - arrowLen * sin(angle + arrowAngle));

    HPEN pen = CreatePen(PS_SOLID, 2, color);
    HPEN oldPen = (HPEN)SelectObject(hdc, pen);

    MoveToEx(hdc, midX, midY, NULL);
    LineTo(hdc, ax1, ay1);
    MoveToEx(hdc, midX, midY, NULL);
    LineTo(hdc, ax2, ay2);

    SelectObject(hdc, oldPen);
    DeleteObject(pen);
}

void GUIApp::drawStopNode(HDC hdc, int stopId, COLORREF bgColor, COLORREF borderColor, bool isHighlight) {
    if (stopCoords.find(stopId) == stopCoords.end()) return;

    int cx = stopCoords[stopId].x;
    int cy = stopCoords[stopId].y;
    int radius = isHighlight ? 12 : 9;

    // Outer glow ring for highlighted nodes
    if (isHighlight) {
        HPEN glowPen = CreatePen(PS_SOLID, 2, borderColor);
        HPEN oldP = (HPEN)SelectObject(hdc, glowPen);
        HBRUSH nullB = (HBRUSH)GetStockObject(NULL_BRUSH);
        HBRUSH oldB = (HBRUSH)SelectObject(hdc, nullB);

        Ellipse(hdc, cx - radius - 4, cy - radius - 4, cx + radius + 4, cy + radius + 4);

        SelectObject(hdc, oldB);
        SelectObject(hdc, oldP);
        DeleteObject(glowPen);
    }

    // Node Circle
    HBRUSH brush = CreateSolidBrush(bgColor);
    HPEN pen = CreatePen(PS_SOLID, 2, borderColor);
    HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, brush);
    HPEN oldPen = (HPEN)SelectObject(hdc, pen);

    Ellipse(hdc, cx - radius, cy - radius, cx + radius, cy + radius);

    SelectObject(hdc, oldBrush);
    SelectObject(hdc, oldPen);
    DeleteObject(brush);
    DeleteObject(pen);

    // Stop Label Text
    const Stop* s = graph.getStop(stopId);
    if (!s) return;

    HFONT hLabelFont = CreateFont(isHighlight ? 13 : 11, 0, 0, 0,
                                  isHighlight ? FW_BOLD : FW_NORMAL,
                                  FALSE, FALSE, FALSE,
                                  DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                  DEFAULT_QUALITY, DEFAULT_PITCH, "Segoe UI");
    HFONT oldFont = (HFONT)SelectObject(hdc, hLabelFont);

    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, isHighlight ? RGB(255, 255, 255) : RGB(190, 200, 215));

    // Place label smartly (offset slightly)
    TextOut(hdc, cx + radius + 4, cy - 7, s->name.c_str(), (int)s->name.length());

    SelectObject(hdc, oldFont);
    DeleteObject(hLabelFont);
}

int GUIApp::run() {
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return (int)msg.wParam;
}

LRESULT CALLBACK GUIApp::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (!g_pApp) return DefWindowProc(hWnd, msg, wParam, lParam);

    switch (msg) {
        case WM_COMMAND: {
            int wmId = LOWORD(wParam);
            int wmEvent = HIWORD(wParam);

            if (wmId == ID_BTN_RUN) {
                g_pApp->runSelectedAlgorithm(false);
            } else if (wmId == ID_BTN_ANIMATE) {
                g_pApp->runSelectedAlgorithm(true);
            } else if (wmId == ID_BTN_RESET) {
                g_pApp->resetState();
            } else if (wmEvent == CBN_SELCHANGE) {
                // Instantly update when user picks an option
                g_pApp->runSelectedAlgorithm(false);
            }
            break;
        }

        case WM_TIMER: {
            if (wParam == ID_TIMER_ANIM && g_pApp->isAnimating) {
                g_pApp->animationStep++;
                if (g_pApp->animationStep >= g_pApp->animationTotalSteps) {
                    g_pApp->isAnimating = false;
                    KillTimer(hWnd, ID_TIMER_ANIM);
                }
                InvalidateRect(hWnd, NULL, FALSE);
            }
            break;
        }

        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);

            // Left panel background
            RECT leftRect = {0, 0, 400, 760};
            HBRUSH lBrush = CreateSolidBrush(RGB(245, 247, 250));
            FillRect(hdc, &leftRect, lBrush);
            DeleteObject(lBrush);

            // Divider Line
            HPEN divPen = CreatePen(PS_SOLID, 2, RGB(210, 215, 225));
            HPEN oldP = (HPEN)SelectObject(hdc, divPen);
            MoveToEx(hdc, 400, 0, NULL);
            LineTo(hdc, 400, 760);
            SelectObject(hdc, oldP);
            DeleteObject(divPen);

            // Right Map Canvas
            RECT mapRect = {402, 0, 1230, 730};
            g_pApp->drawMap(hdc, mapRect);

            EndPaint(hWnd, &ps);
            break;
        }

        case WM_DESTROY: {
            KillTimer(hWnd, ID_TIMER_ANIM);
            PostQuitMessage(0);
            break;
        }

        default:
            return DefWindowProc(hWnd, msg, wParam, lParam);
    }
    return 0;
}

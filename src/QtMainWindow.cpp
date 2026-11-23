#include "QtMainWindow.h"
#include <QApplication>
#include <QScreen>
#include <QScrollBar>

QtMainWindow::QtMainWindow(QWidget* parent)
    : QMainWindow(parent), dataMgr("data/stops.txt", "data/roads.txt", "data/buses.txt") {
    dataMgr.ensureDefaultFilesExist(graph, busMgr);
    dataMgr.loadAll(graph, busMgr);

    setupUI();
    applyDarkTheme();
    populateDropdowns();

    // Run default BFS route between Uttara and Motijheel on start
    executeAlgorithm(false);
}

QtMainWindow::~QtMainWindow() {}

void QtMainWindow::setupUI() {
    setWindowTitle("TransitIQ — Intelligent Public Transport & Journey Explorer (Dhaka Transit)");
    resize(1280, 820);
    setMinimumSize(1050, 700);

    // Center on screen
    if (auto screen = QGuiApplication::primaryScreen()) {
        setGeometry(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, size(), screen->availableGeometry()));
    }

    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QVBoxLayout* rootLayout = new QVBoxLayout(centralWidget);
    rootLayout->setContentsMargins(16, 12, 16, 16);
    rootLayout->setSpacing(12);

    // ─────────────────────────────────────────────────────────────
    // 1. TOP HEADER (Brand & Live Network Status)
    // ─────────────────────────────────────────────────────────────
    QFrame* headerFrame = new QFrame(this);
    headerFrame->setObjectName("headerFrame");
    QHBoxLayout* headerLayout = new QHBoxLayout(headerFrame);
    headerLayout->setContentsMargins(16, 10, 16, 10);

    // Logo icon + titles
    QLabel* logoIcon = new QLabel("🚌", headerFrame);
    logoIcon->setObjectName("logoIcon");

    QVBoxLayout* titleBox = new QVBoxLayout();
    titleBox->setSpacing(2);
    QLabel* titleLabel = new QLabel("TransitIQ", headerFrame);
    titleLabel->setObjectName("titleLabel");

    QLabel* subtitleLabel = new QLabel("Intelligent Public Transport Recommendations & Algorithmic Route Explorer", headerFrame);
    subtitleLabel->setObjectName("subtitleLabel");
    titleBox->addWidget(titleLabel);
    titleBox->addWidget(subtitleLabel);

    headerLayout->addWidget(logoIcon);
    headerLayout->addLayout(titleBox);
    headerLayout->addStretch();

    // Live status pill badge
    QLabel* statusBadge = new QLabel(
        QString("● LIVE NETWORK: %1 Stops  |  %2 Roads  |  %3 Bus Fleets")
            .arg(graph.getAllStops().size())
            .arg(graph.getAllRoads().size())
            .arg(busMgr.getAllBuses().size()),
        headerFrame
    );
    statusBadge->setObjectName("statusBadge");
    headerLayout->addWidget(statusBadge);

    rootLayout->addWidget(headerFrame);

    // ─────────────────────────────────────────────────────────────
    // 2. MAIN SPLIT BODY (Left Controls & Cards + Right Map)
    // ─────────────────────────────────────────────────────────────
    QHBoxLayout* bodyLayout = new QHBoxLayout();
    bodyLayout->setSpacing(14);

    // LEFT PANEL (Scrollable Controls & Information)
    QScrollArea* scrollArea = new QScrollArea(this);
    scrollArea->setObjectName("leftScrollArea");
    scrollArea->setWidgetResizable(true);
    scrollArea->setFixedWidth(440);
    scrollArea->setFrameShape(QFrame::NoFrame);

    QWidget* leftContainer = new QWidget(scrollArea);
    leftContainer->setObjectName("leftContainer");
    QVBoxLayout* leftLayout = new QVBoxLayout(leftContainer);
    leftLayout->setContentsMargins(0, 0, 8, 0);
    leftLayout->setSpacing(14);

    // Card 1: Journey Planner Controls
    QFrame* controlCard = new QFrame(leftContainer);
    controlCard->setObjectName("cardFrame");
    QVBoxLayout* cLayout = new QVBoxLayout(controlCard);
    cLayout->setContentsMargins(16, 16, 16, 16);
    cLayout->setSpacing(10);

    QLabel* cTitle = new QLabel("JOURNEY PLANNER & CONTROLS", controlCard);
    cTitle->setObjectName("cardTitle");
    cLayout->addWidget(cTitle);

    // Origin
    QLabel* lblFrom = new QLabel("Boarding / Origin Stop:", controlCard);
    lblFrom->setObjectName("fieldLabel");
    comboSrc = new QComboBox(controlCard);
    cLayout->addWidget(lblFrom);
    cLayout->addWidget(comboSrc);

    // Destination
    QLabel* lblTo = new QLabel("Destination / Alighting Stop:", controlCard);
    lblTo->setObjectName("fieldLabel");
    comboDest = new QComboBox(controlCard);
    cLayout->addWidget(lblTo);
    cLayout->addWidget(comboDest);

    // Algorithm Mode
    QLabel* lblMode = new QLabel("Algorithm / Exploration Mode:", controlCard);
    lblMode->setObjectName("fieldLabel");
    comboMode = new QComboBox(controlCard);
    cLayout->addWidget(lblMode);
    cLayout->addWidget(comboMode);

    // Road Closure Simulation
    QLabel* lblBlock = new QLabel("Road Closure Simulator (Dynamic Detour):", controlCard);
    lblBlock->setObjectName("fieldLabel");
    comboRoadBlock = new QComboBox(controlCard);
    cLayout->addWidget(lblBlock);
    cLayout->addWidget(comboRoadBlock);

    // Action Buttons
    QHBoxLayout* btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(8);

    btnRun = new QPushButton("Find Optimal Route", controlCard);
    btnRun->setObjectName("btnPrimary");
    btnRun->setCursor(Qt::PointingHandCursor);

    btnAnimate = new QPushButton("▶ Animate Steps", controlCard);
    btnAnimate->setObjectName("btnSecondary");
    btnAnimate->setCursor(Qt::PointingHandCursor);

    btnLayout->addWidget(btnRun);
    btnLayout->addWidget(btnAnimate);
    cLayout->addLayout(btnLayout);

    btnReset = new QPushButton("🔄 Reset Network & Detours", controlCard);
    btnReset->setObjectName("btnMuted");
    btnReset->setCursor(Qt::PointingHandCursor);
    cLayout->addWidget(btnReset);

    leftLayout->addWidget(controlCard);

    // Card 2: Route Metrics Stat Cards
    QFrame* statsCard = new QFrame(leftContainer);
    statsCard->setObjectName("cardFrame");
    QVBoxLayout* sLayout = new QVBoxLayout(statsCard);
    sLayout->setContentsMargins(16, 14, 16, 14);
    sLayout->setSpacing(8);

    QLabel* sTitle = new QLabel("ESTIMATED JOURNEY METRICS", statsCard);
    sTitle->setObjectName("cardTitle");
    sLayout->addWidget(sTitle);

    QHBoxLayout* gridStats = new QHBoxLayout();
    gridStats->setSpacing(8);

    auto makeStatBox = [this, &gridStats](const QString& title, QLabel*& valLabel) {
        QFrame* box = new QFrame();
        box->setObjectName("statBox");
        QVBoxLayout* bLayout = new QVBoxLayout(box);
        bLayout->setContentsMargins(8, 8, 8, 8);
        bLayout->setSpacing(2);

        QLabel* t = new QLabel(title, box);
        t->setObjectName("statTitle");
        valLabel = new QLabel("-", box);
        valLabel->setObjectName("statValue");

        bLayout->addWidget(t);
        bLayout->addWidget(valLabel);
        gridStats->addWidget(box);
    };

    makeStatBox("STOPS", lblStatStops);
    makeStatBox("HOPS", lblStatHops);
    makeStatBox("DIST", lblStatDist);
    makeStatBox("EST. TIME", lblStatTime);

    sLayout->addLayout(gridStats);
    leftLayout->addWidget(statsCard);

    // Card 3: Bus Recommendations (Website Style)
    QFrame* recCard = new QFrame(leftContainer);
    recCard->setObjectName("cardFrame");
    QVBoxLayout* rLayout = new QVBoxLayout(recCard);
    rLayout->setContentsMargins(16, 14, 16, 14);
    rLayout->setSpacing(10);

    QLabel* rTitle = new QLabel("BUS RECOMMENDATIONS & FLEETS", recCard);
    rTitle->setObjectName("cardTitle");
    rLayout->addWidget(rTitle);

    recContainer = new QWidget(recCard);
    recLayout = new QVBoxLayout(recContainer);
    recLayout->setContentsMargins(0, 0, 0, 0);
    recLayout->setSpacing(8);
    rLayout->addWidget(recContainer);

    leftLayout->addWidget(recCard);

    // Card 4: Algorithmic Execution Trace / Queue State
    QFrame* logCard = new QFrame(leftContainer);
    logCard->setObjectName("cardFrame");
    QVBoxLayout* lLayout = new QVBoxLayout(logCard);
    lLayout->setContentsMargins(16, 14, 16, 14);
    lLayout->setSpacing(8);

    QLabel* lTitle = new QLabel("ALGORITHM LOG & QUEUE TRACE", logCard);
    lTitle->setObjectName("cardTitle");
    lLayout->addWidget(lTitle);

    textLog = new QTextEdit(logCard);
    textLog->setObjectName("textLog");
    textLog->setReadOnly(true);
    textLog->setFixedHeight(140);
    lLayout->addWidget(textLog);

    leftLayout->addWidget(logCard);

    scrollArea->setWidget(leftContainer);
    bodyLayout->addWidget(scrollArea);

    // RIGHT PANEL (Vector Dhaka Transit Map)
    mapWidget = new QtMapWidget(&graph, this);
    bodyLayout->addWidget(mapWidget, 1);

    rootLayout->addLayout(bodyLayout);

    // Connections
    connect(btnRun, &QPushButton::clicked, this, &QtMainWindow::onRunClicked);
    connect(btnAnimate, &QPushButton::clicked, this, &QtMainWindow::onAnimateClicked);
    connect(btnReset, &QPushButton::clicked, this, &QtMainWindow::onResetClicked);

    connect(comboSrc, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &QtMainWindow::onSelectionChanged);
    connect(comboDest, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &QtMainWindow::onSelectionChanged);
    connect(comboMode, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &QtMainWindow::onSelectionChanged);
    connect(comboRoadBlock, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &QtMainWindow::onSelectionChanged);
}

void QtMainWindow::applyDarkTheme() {
    // Style matching https://transitiq.bolt.host
    QString qss = R"(
        QMainWindow, QWidget#leftContainer {
            background-color: #090d16;
            color: #f8fafc;
            font-family: 'Plus Jakarta Sans', 'Segoe UI', sans-serif;
        }

        QFrame#headerFrame {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #0f172a, stop:1 #1e293b);
            border: 1px solid rgba(255, 255, 255, 0.08);
            border-radius: 12px;
        }

        QLabel#logoIcon {
            font-size: 26px;
            padding-right: 8px;
        }

        QLabel#titleLabel {
            font-size: 22px;
            font-weight: 800;
            color: #5bdcff;
            letter-spacing: 0.5px;
        }

        QLabel#subtitleLabel {
            font-size: 11px;
            color: #94a3b8;
            font-weight: 500;
        }

        QLabel#statusBadge {
            background-color: rgba(34, 195, 240, 0.12);
            color: #38bdf8;
            border: 1px solid rgba(56, 189, 248, 0.35);
            padding: 6px 14px;
            border-radius: 20px;
            font-size: 11px;
            font-weight: 600;
        }

        QFrame#cardFrame {
            background-color: #111827;
            border: 1px solid rgba(255, 255, 255, 0.07);
            border-radius: 12px;
        }

        QLabel#cardTitle {
            font-size: 11px;
            font-weight: 800;
            color: #5bdcff;
            letter-spacing: 1px;
        }

        QLabel#fieldLabel {
            font-size: 11px;
            font-weight: 600;
            color: #cbd5e1;
        }

        QComboBox {
            background-color: #1e293b;
            color: #f8fafc;
            border: 1px solid #334155;
            border-radius: 8px;
            padding: 7px 12px;
            font-size: 12px;
            font-weight: 500;
        }

        QComboBox:hover {
            border: 1px solid #38bdf8;
        }

        QComboBox:focus {
            border: 1px solid #0ea5e9;
        }

        QComboBox::drop-down {
            subcontrol-origin: padding;
            subcontrol-position: top right;
            width: 24px;
            border-left: none;
        }

        QComboBox QAbstractItemView {
            background-color: #0f172a;
            color: #f8fafc;
            border: 1px solid #334155;
            selection-background-color: #0284c7;
            selection-color: white;
            padding: 4px;
        }

        QPushButton#btnPrimary {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #0284c7, stop:1 #0ea5e9);
            color: white;
            border: none;
            border-radius: 8px;
            padding: 10px 16px;
            font-size: 12px;
            font-weight: 700;
        }

        QPushButton#btnPrimary:hover {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #0369a1, stop:1 #38bdf8);
        }

        QPushButton#btnSecondary {
            background-color: rgba(56, 189, 248, 0.12);
            color: #38bdf8;
            border: 1px solid rgba(56, 189, 248, 0.4);
            border-radius: 8px;
            padding: 10px 14px;
            font-size: 12px;
            font-weight: 700;
        }

        QPushButton#btnSecondary:hover {
            background-color: rgba(56, 189, 248, 0.22);
        }

        QPushButton#btnMuted {
            background-color: #1e293b;
            color: #94a3b8;
            border: 1px solid #334155;
            border-radius: 8px;
            padding: 6px 12px;
            font-size: 11px;
            font-weight: 600;
        }

        QPushButton#btnMuted:hover {
            background-color: #334155;
            color: white;
        }

        QFrame#statBox {
            background-color: #1e293b;
            border: 1px solid rgba(255, 255, 255, 0.05);
            border-radius: 8px;
        }

        QLabel#statTitle {
            font-size: 9px;
            font-weight: 700;
            color: #94a3b8;
            letter-spacing: 0.5px;
        }

        QLabel#statValue {
            font-size: 15px;
            font-weight: 800;
            color: #f8fafc;
        }

        QFrame#busCard {
            background-color: #1a2234;
            border: 1px solid rgba(255, 255, 255, 0.08);
            border-radius: 10px;
        }

        QFrame#busCard:hover {
            border: 1px solid rgba(56, 189, 248, 0.4);
        }

        QTextEdit#textLog {
            background-color: #0b0f19;
            color: #38bdf8;
            border: 1px solid #1e293b;
            border-radius: 8px;
            font-family: 'JetBrains Mono', 'Consolas', monospace;
            font-size: 11px;
            padding: 6px;
        }

        QScrollBar:vertical {
            background: #0f172a;
            width: 8px;
            border-radius: 4px;
        }

        QScrollBar::handle:vertical {
            background: #334155;
            border-radius: 4px;
            min-height: 20px;
        }

        QScrollBar::handle:vertical:hover {
            background: #0ea5e9;
        }

        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
            height: 0px;
        }
    )";

    setStyleSheet(qss);
}

void QtMainWindow::populateDropdowns() {
    comboSrc->blockSignals(true);
    comboDest->blockSignals(true);
    comboMode->blockSignals(true);
    comboRoadBlock->blockSignals(true);

    comboSrc->clear();
    comboDest->clear();

    for (const auto& pair : graph.getAllStops()) {
        QString text = QString("%1. %2 (%3)")
            .arg(pair.first)
            .arg(QString::fromStdString(pair.second.name))
            .arg(QString::fromStdString(pair.second.zone));
        comboSrc->addItem(text, pair.first);
        comboDest->addItem(text, pair.first);
    }

    // Default: Uttara (1) to Motijheel (22)
    comboSrc->setCurrentIndex(0);
    comboDest->setCurrentIndex(21);

    // Modes
    comboMode->clear();
    comboMode->addItem("1. 🧭 BFS Journey Finder (Shortest Connections)", 0);
    comboMode->addItem("2. 🔄 BFS Bus Transfer Finder (Min Bus Switches)", 1);
    comboMode->addItem("3. 🌐 BFS Reachability Explorer (Hop Levels)", 2);
    comboMode->addItem("4. 🔍 DFS Network Explorer (Deep Corridor Walk)", 3);
    comboMode->addItem("5. 🏝️ DFS Connected Components & Isolated Stops", 4);
    comboMode->addItem("6. ⚠️ DFS Road Closure & Bottleneck Impact", 5);
    comboMode->addItem("7. 🚌 Bus Recommendation (Top 3 Direct/Transfer)", 6);

    // Road Block Simulator Options
    comboRoadBlock->clear();
    comboRoadBlock->addItem("None (All Roads Fully Open)", 0);
    comboRoadBlock->addItem("Farmgate <-> Karwan-Bazar (14 <-> 15)", 1);
    comboRoadBlock->addItem("Airport <-> Khilkhet (2 <-> 3)", 2);
    comboRoadBlock->addItem("Kuril <-> Banani (4 <-> 6)", 3);
    comboRoadBlock->addItem("Mohakhali <-> Farmgate (9 <-> 14)", 4);
    comboRoadBlock->addItem("Mirpur-10 <-> Banani (19 <-> 6)", 5);
    comboRoadBlock->addItem("Malibagh <-> Sayedabad (12 <-> 23)", 6);
    comboRoadBlock->addItem("Dhanmondi <-> New-Market (17 <-> 18)", 7);

    comboSrc->blockSignals(false);
    comboDest->blockSignals(false);
    comboMode->blockSignals(false);
    comboRoadBlock->blockSignals(false);
}

void QtMainWindow::onSelectionChanged() {
    executeAlgorithm(false);
}

void QtMainWindow::onRunClicked() {
    executeAlgorithm(false);
}

void QtMainWindow::onAnimateClicked() {
    executeAlgorithm(true);
}

void QtMainWindow::onResetClicked() {
    // Unblock all roads
    auto blocked = graph.getBlockedRoads();
    for (const auto& edge : blocked) {
        graph.unblockRoad(edge.first, edge.second);
    }
    comboRoadBlock->setCurrentIndex(0);
    mapWidget->resetView();
    executeAlgorithm(false);
}

void QtMainWindow::executeAlgorithm(bool startAnimation) {
    int srcId = comboSrc->currentData().toInt();
    int destId = comboDest->currentData().toInt();
    int mode = comboMode->currentData().toInt();
    int blockOpt = comboRoadBlock->currentData().toInt();

    // Reset roadblocks first
    auto blocked = graph.getBlockedRoads();
    for (const auto& edge : blocked) {
        graph.unblockRoad(edge.first, edge.second);
    }

    // Apply simulated roadblock
    switch (blockOpt) {
        case 1: graph.blockRoad(14, 15); break; // Farmgate <-> Karwan-Bazar
        case 2: graph.blockRoad(2, 3);   break; // Airport <-> Khilkhet
        case 3: graph.blockRoad(4, 6);   break; // Kuril <-> Banani
        case 4: graph.blockRoad(9, 14);  break; // Mohakhali <-> Farmgate
        case 5: graph.blockRoad(19, 6);  break; // Mirpur-10 <-> Banani
        case 6: graph.blockRoad(12, 23); break; // Malibagh <-> Sayedabad
        case 7: graph.blockRoad(17, 18); break; // Dhanmondi <-> New-Market
        default: break;
    }

    mapWidget->setSelectedStops(srcId, destId);
    mapWidget->setAlgorithmMode(mode);

    std::vector<int> path;
    std::vector<JourneyLeg> journey;
    std::vector<BusRecommendation> recs;
    std::string logText = "";

    std::vector<int> stepNodes;
    std::vector<std::pair<int, int>> stepEdges;
    std::vector<QColor> stepColors;

    if (mode == 0) {
        // Mode 1: BFS Journey Finder
        bool found = graph.bfsShortestRoute(srcId, destId, path, false);
        mapWidget->setPath(path);

        if (found) {
            logText += "BFS Shortest Route Found!\r\n";
            logText += "Guarantees minimum intermediate connections.\r\n";
            if (graph.getBlockedRoads().size() > 0) {
                logText += "[!] Detour active: bypassing closed road.\r\n";
            }
        } else {
            logText += "NO ROUTE FOUND!\r\nRoad closure disconnected this corridor.\r\n";
        }

        // Trace for animation (BFS queue expansion)
        std::queue<int> q;
        std::map<int, bool> vis;
        q.push(srcId);
        vis[srcId] = true;
        while (!q.empty()) {
            int curr = q.front();
            q.pop();
            stepNodes.push_back(curr);
            stepColors.push_back(QColor(245, 158, 11)); // Amber
            if (curr == destId) break;

            for (const auto& p : graph.getAllRoads()) {
                int u = p.second.u, v = p.second.v;
                if ((u == curr || v == curr) && !graph.isRoadBlocked(u, v)) {
                    int neighbor = (u == curr ? v : u);
                    if (!vis[neighbor]) {
                        vis[neighbor] = true;
                        q.push(neighbor);
                        stepEdges.push_back({curr, neighbor});
                    }
                }
            }
        }
    } else if (mode == 1) {
        // Mode 2: BFS Bus Transfers
        graph.bfsMinimumBusTransfers(srcId, destId, busMgr.getAllBuses(), journey, false);
        mapWidget->setJourney(journey);
        logText += "BFS Minimum Bus Transfers Route:\r\n";
        logText += "Minimizes vehicle changes across bus lines.\r\n";

        for (size_t leg = 0; leg < journey.size(); ++leg) {
            logText += QString("• Leg %1: %2 (%3 -> %4)\r\n")
                .arg(leg + 1)
                .arg(QString::fromStdString(journey[leg].busName))
                .arg(QString::fromStdString(graph.getStop(journey[leg].boardStopId)->name))
                .arg(QString::fromStdString(graph.getStop(journey[leg].alightStopId)->name))
                .toStdString();

            QColor legColor = (leg == 0) ? QColor(34, 195, 240) : QColor(217, 70, 239);
            for (size_t i = 0; i < journey[leg].stopsInLeg.size(); ++i) {
                int sId = journey[leg].stopsInLeg[i];
                stepNodes.push_back(sId);
                stepColors.push_back(legColor);
                if (path.empty() || path.back() != sId) {
                    path.push_back(sId);
                }
                if (i + 1 < journey[leg].stopsInLeg.size()) {
                    stepEdges.push_back({sId, journey[leg].stopsInLeg[i + 1]});
                }
            }
        }
        mapWidget->setPath(path);
    } else if (mode == 2) {
        // Mode 3: BFS Reachability Explorer
        std::vector<std::pair<int, int>> reachables;
        graph.bfsAllReachable(srcId, -1, reachables, false);
        mapWidget->setReachable(reachables);

        logText += QString("BFS Reachable Stops from %1:\r\n")
            .arg(QString::fromStdString(graph.getStop(srcId)->name))
            .toStdString();

        QColor hopColors[] = {
            QColor(245, 158, 11), // Hop 0 (Origin)
            QColor(16, 185, 129), // Hop 1 (Green)
            QColor(34, 195, 240), // Hop 2 (Cyan)
            QColor(168, 85, 247), // Hop 3 (Purple)
            QColor(236, 72, 153)  // Hop 4+ (Pink)
        };

        for (const auto& item : reachables) {
            logText += QString("• Level %1: %2\r\n")
                .arg(item.second)
                .arg(QString::fromStdString(graph.getStop(item.first)->name))
                .toStdString();

            stepNodes.push_back(item.first);
            stepColors.push_back(hopColors[std::min(item.second, 4)]);
        }

        // Add tree edges for animation
        for (size_t i = 1; i < stepNodes.size(); ++i) {
            stepEdges.push_back({srcId, stepNodes[i]});
        }
    } else if (mode == 3) {
        // Mode 4: DFS Network Explorer
        std::vector<int> dfsOrder;
        graph.dfsExplore(srcId, dfsOrder, false);
        logText += "DFS Deep Corridor Exploration:\r\n";
        logText += "Deeply walks corridors until dead ends before backtracking.\r\n";

        for (size_t i = 0; i < dfsOrder.size(); ++i) {
            int sId = dfsOrder[i];
            path.push_back(sId);
            stepNodes.push_back(sId);
            stepColors.push_back(QColor(249, 115, 22)); // Orange
            logText += QString("-> %1\r\n").arg(QString::fromStdString(graph.getStop(sId)->name)).toStdString();
            if (i + 1 < dfsOrder.size()) {
                stepEdges.push_back({sId, dfsOrder[i + 1]});
            }
        }
        mapWidget->setPath(path);
    } else if (mode == 4) {
        // Mode 5: DFS Components & Isolated Stops
        auto comps = graph.dfsFindConnectedComponents();
        mapWidget->setComponents(comps);
        logText += "DFS Connected Components Topology:\r\n";
        logText += QString("Total Subgraphs: %1\r\n\r\n").arg(comps.size()).toStdString();

        QColor compColors[] = {
            QColor(34, 195, 240), QColor(168, 85, 247),
            QColor(16, 185, 129), QColor(245, 158, 11)
        };

        for (size_t c = 0; c < comps.size(); ++c) {
            if (comps[c].size() == 1) {
                logText += QString("⚠️ Isolated Stop: %1\r\n")
                    .arg(QString::fromStdString(graph.getStop(comps[c][0])->name))
                    .toStdString();
                stepNodes.push_back(comps[c][0]);
                stepColors.push_back(QColor(239, 68, 68)); // Red for isolated
            } else {
                logText += QString("Component #%1 (%2 stops)\r\n")
                    .arg(c + 1)
                    .arg(comps[c].size())
                    .toStdString();
                for (int sId : comps[c]) {
                    stepNodes.push_back(sId);
                    stepColors.push_back(compColors[c % 4]);
                }
            }
        }
    } else if (mode == 5) {
        // Mode 6: DFS Closure Impact
        auto report = graph.dfsAnalyzeClosureImpact(srcId, destId, busMgr.getAllBuses());
        mapWidget->setClosureReport(report);
        path = report.alternativeRoute;

        logText += "DFS Road Closure & Bottleneck Report:\r\n";
        logText += QString("Closed Corridor: %1 <-> %2\r\n")
            .arg(QString::fromStdString(report.nameU))
            .arg(QString::fromStdString(report.nameV))
            .toStdString();

        if (report.isCriticalBridge) {
            logText += "⚠️ CRITICAL BOTTLENECK / BRIDGE DETECTED!\r\n";
            logText += QString("Closing splits network into %1 disconnected parts.\r\n").arg(report.componentsAfter).toStdString();
        } else {
            logText += "✓ Network remains connected.\r\n";
            logText += "Redundant alternative bypass route found!\r\n";
        }

        // Animation shows closed road, then bypass detour
        stepNodes.push_back(srcId);
        stepColors.push_back(QColor(239, 68, 68));
        stepNodes.push_back(destId);
        stepColors.push_back(QColor(239, 68, 68));
        stepEdges.push_back({srcId, destId});

        for (size_t i = 0; i < report.alternativeRoute.size(); ++i) {
            stepNodes.push_back(report.alternativeRoute[i]);
            stepColors.push_back(QColor(16, 185, 129));
            if (i + 1 < report.alternativeRoute.size()) {
                stepEdges.push_back({report.alternativeRoute[i], report.alternativeRoute[i + 1]});
            }
        }
        mapWidget->setPath(path);
    } else if (mode == 6) {
        // Mode 7: Bus Recommendations
        recs = busMgr.recommendBuses(srcId, destId, graph);
        logText += "Bus Fleet Timetable Recommendations:\r\n";
        logText += QString("Trip: %1 -> %2\r\n")
            .arg(QString::fromStdString(graph.getStop(srcId)->name))
            .arg(QString::fromStdString(graph.getStop(destId)->name))
            .toStdString();

        if (!recs.empty()) {
            path = recs[0].traversedStops;
            mapWidget->setPath(path);

            for (size_t i = 0; i < path.size(); ++i) {
                stepNodes.push_back(path[i]);
                stepColors.push_back(QColor(34, 195, 240));
                if (i + 1 < path.size()) {
                    stepEdges.push_back({path[i], path[i + 1]});
                }
            }
        }
    }

    if (mode != 6) {
        recs = busMgr.recommendBuses(srcId, destId, graph);
    }

    updateMetricsAndCards(path, recs, logText);

    if (startAnimation) {
        mapWidget->startStepAnimation(stepNodes, stepEdges, stepColors);
    }
}

void QtMainWindow::updateMetricsAndCards(const std::vector<int>& path,
                                        const std::vector<BusRecommendation>& recs,
                                        const std::string& logText) {
    // 1. Update Stat Badges
    if (!path.empty()) {
        double dist = 0.0;
        int time = 0;
        graph.calculatePathMetrics(path, dist, time);

        lblStatStops->setText(QString::number(path.size()));
        lblStatHops->setText(QString::number(path.size() - 1));
        lblStatDist->setText(QString("%1 km").arg(dist, 0, 'f', 1));
        lblStatTime->setText(QString("~%1 min").arg(time));
    } else {
        lblStatStops->setText("0");
        lblStatHops->setText("0");
        lblStatDist->setText("0 km");
        lblStatTime->setText("-");
    }

    // 2. Update Bus Recommendation Cards matching transitiq.bolt.host
    QLayoutItem* item;
    while ((item = recLayout->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }

    if (recs.empty()) {
        QLabel* noBus = new QLabel("No direct or 1-transfer buses found.", recContainer);
        noBus->setStyleSheet("color: #94a3b8; font-size: 11px; padding: 8px;");
        recLayout->addWidget(noBus);
    } else {
        for (size_t i = 0; i < recs.size(); ++i) {
            const auto& r = recs[i];
            QFrame* card = new QFrame(recContainer);
            card->setObjectName("busCard");
            QVBoxLayout* cardLayout = new QVBoxLayout(card);
            cardLayout->setContentsMargins(12, 10, 12, 10);
            cardLayout->setSpacing(6);

            // Badge / Tagline
            QHBoxLayout* tagRow = new QHBoxLayout();
            QLabel* tagBadge = new QLabel(QString::fromStdString(r.highlightReason), card);
            tagBadge->setStyleSheet(
                (i == 0)
                    ? "background: rgba(16, 185, 129, 0.15); color: #34d399; font-weight: 700; font-size: 10px; padding: 2px 8px; border-radius: 4px;"
                    : "background: rgba(56, 189, 248, 0.15); color: #38bdf8; font-weight: 700; font-size: 10px; padding: 2px 8px; border-radius: 4px;"
            );
            tagRow->addWidget(tagBadge);
            tagRow->addStretch();

            QLabel* fareLabel = new QLabel(QString("৳%1 BDT").arg(r.bus.baseFare), card);
            fareLabel->setStyleSheet("color: #f8fafc; font-weight: 800; font-size: 12px;");
            tagRow->addWidget(fareLabel);
            cardLayout->addLayout(tagRow);

            // Bus Name & Type
            QHBoxLayout* nameRow = new QHBoxLayout();
            QLabel* busName = new QLabel(QString::fromStdString(r.bus.name), card);
            busName->setStyleSheet("color: #f8fafc; font-size: 14px; font-weight: 700;");

            QLabel* busType = new QLabel(QString::fromStdString(r.bus.type), card);
            busType->setStyleSheet("color: #94a3b8; font-size: 10px; font-weight: 600; padding: 1px 6px; background: #0f172a; border-radius: 4px;");

            nameRow->addWidget(busName);
            nameRow->addSpacing(6);
            nameRow->addWidget(busType);
            nameRow->addStretch();
            cardLayout->addLayout(nameRow);

            // Route timeline / metrics
            QLabel* metaRow = new QLabel(
                QString("⏱ ~%1 mins  •  🛣 %2 km  •  🚏 %3 stops  •  🕒 Every %4 min")
                    .arg(r.estimatedTimeMin)
                    .arg(r.estimatedDistanceKm, 0, 'f', 1)
                    .arg(r.numStops)
                    .arg(r.bus.frequencyMin),
                card
            );
            metaRow->setStyleSheet("color: #94a3b8; font-size: 10px; font-weight: 500;");
            cardLayout->addWidget(metaRow);

            recLayout->addWidget(card);
        }
    }

    // 3. Update Log Text
    textLog->setText(QString::fromStdString(logText));
}


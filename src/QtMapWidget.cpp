#include "QtMapWidget.h"
#include <QPainterPath>
#include <QLinearGradient>
#include <cmath>

QtMapWidget::QtMapWidget(const Graph* g, QWidget* parent)
    : QWidget(parent), graph(g),
      selectedSrc(1), selectedDest(22), selectedMode(0),
      isAnimating(false), animationStep(0), animationTotalSteps(0),
      vehicleProgress(0.0) {
    initPositions();

    animTimer = new QTimer(this);
    connect(animTimer, &QTimer::timeout, this, &QtMapWidget::onAnimationTick);

    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setMinimumSize(550, 600);
}

QtMapWidget::~QtMapWidget() {}

void QtMapWidget::initPositions() {
    nodePositions[1]  = {0.45, 0.08}; // Uttara
    nodePositions[2]  = {0.45, 0.16}; // Airport
    nodePositions[3]  = {0.58, 0.20}; // Khilkhet
    nodePositions[4]  = {0.64, 0.28}; // Kuril
    nodePositions[5]  = {0.80, 0.29}; // Bashundhara
    nodePositions[6]  = {0.44, 0.31}; // Banani
    nodePositions[7]  = {0.56, 0.33}; // Gulshan-2
    nodePositions[8]  = {0.56, 0.43}; // Gulshan-1
    nodePositions[9]  = {0.43, 0.42}; // Mohakhali
    nodePositions[10] = {0.70, 0.43}; // Badda
    nodePositions[11] = {0.70, 0.54}; // Rampura
    nodePositions[12] = {0.62, 0.65}; // Malibagh
    nodePositions[13] = {0.50, 0.65}; // Moghbazar
    nodePositions[14] = {0.33, 0.56}; // Farmgate
    nodePositions[15] = {0.36, 0.65}; // Karwan-Bazar
    nodePositions[16] = {0.41, 0.75}; // Shahbagh
    nodePositions[17] = {0.22, 0.65}; // Dhanmondi
    nodePositions[18] = {0.27, 0.78}; // New-Market
    nodePositions[19] = {0.20, 0.28}; // Mirpur-10
    nodePositions[20] = {0.17, 0.39}; // Mirpur-1
    nodePositions[21] = {0.10, 0.51}; // Gabtoli
    nodePositions[22] = {0.54, 0.85}; // Motijheel
    nodePositions[23] = {0.70, 0.83}; // Sayedabad
}

QPointF QtMapWidget::getCanvasPos(int stopId, const QRectF& area) const {
    auto it = nodePositions.find(stopId);
    if (it == nodePositions.end()) return QPointF(0, 0);

    double x = area.left() + it->second.nx * area.width();
    double y = area.top() + it->second.ny * area.height();
    return QPointF(x, y);
}

void QtMapWidget::setSelectedStops(int src, int dest) {
    selectedSrc = src;
    selectedDest = dest;
    update();
}

void QtMapWidget::setAlgorithmMode(int mode) {
    selectedMode = mode;
    update();
}

void QtMapWidget::setPath(const std::vector<int>& path) {
    currentPath = path;
    update();
}

void QtMapWidget::setJourney(const std::vector<JourneyLeg>& journey) {
    currentJourney = journey;
    update();
}

void QtMapWidget::setComponents(const std::vector<std::vector<int>>& comps) {
    currentComponents = comps;
    update();
}

void QtMapWidget::setReachable(const std::vector<std::pair<int, int>>& reach) {
    currentReachable = reach;
    update();
}

void QtMapWidget::setClosureReport(const RoadClosureAnalysis& report) {
    currentClosureReport = report;
    update();
}

void QtMapWidget::startStepAnimation(const std::vector<int>& visitOrder, const std::vector<std::pair<int, int>>& edges) {
    animationVisitedOrder = visitOrder;
    animationEdges = edges;
    animationStep = 0;
    animationTotalSteps = static_cast<int>(visitOrder.size());
    isAnimating = true;
    vehicleProgress = 0.0;
    animTimer->start(250);
    update();
}

void QtMapWidget::stopAnimation() {
    isAnimating = false;
    animTimer->stop();
    update();
}

void QtMapWidget::resetView() {
    stopAnimation();
    currentPath.clear();
    currentJourney.clear();
    currentComponents.clear();
    currentReachable.clear();
    animationVisitedOrder.clear();
    animationEdges.clear();
    update();
}

void QtMapWidget::onAnimationTick() {
    if (!isAnimating) return;

    animationStep++;
    vehicleProgress += 0.08;
    if (vehicleProgress > 1.0) vehicleProgress = 0.0;

    if (animationStep >= animationTotalSteps + 5) {
        // Stop animation after full traversal
        stopAnimation();
    }
    update();
}

void QtMapWidget::paintEvent(QPaintEvent* /*event*/) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setRenderHint(QPainter::TextAntialiasing, true);

    QRectF mapRect = rect();

    // 1. Sleek Background (Dark Obsidian matching transitiq.bolt.host)
    QLinearGradient bgGrad(0, 0, width(), height());
    bgGrad.setColorAt(0.0, QColor(10, 14, 23));   // #0a0e17
    bgGrad.setColorAt(1.0, QColor(15, 23, 42));   // #0f172a
    p.fillRect(mapRect, bgGrad);

    // Subtle Map Grid Pattern
    p.setPen(QPen(QColor(255, 255, 255, 8), 1, Qt::DotLine));
    for (int x = 40; x < width(); x += 60) {
        p.drawLine(x, 0, x, height());
    }
    for (int y = 40; y < height(); y += 60) {
        p.drawLine(0, y, width(), y);
    }

    // Top map label
    p.setFont(QFont("Plus Jakarta Sans", 11, QFont::Bold));
    p.setPen(QColor(91, 220, 255, 140)); // #5bdcff
    p.drawText(24, 30, "DHAKA METROPOLITAN BUS TRANSIT NETWORK");

    p.setFont(QFont("Plus Jakarta Sans", 9));
    p.setPen(QColor(148, 163, 184, 180));
    p.drawText(24, 48, "Live Graph Layout • BFS & DFS Algorithmic Visualizer");

    if (!graph) return;

    // 2. Draw Road Corridors (Edges)
    for (const auto& pair : graph->getAllRoads()) {
        int u = pair.second.u;
        int v = pair.second.v;
        bool isBlocked = graph->isRoadBlocked(u, v);

        QPointF p1 = getCanvasPos(u, mapRect);
        QPointF p2 = getCanvasPos(v, mapRect);

        if (isBlocked) {
            // Blocked road: Red dashed line + glowing red outline
            p.setPen(QPen(QColor(239, 68, 68, 60), 6, Qt::SolidLine, Qt::RoundCap));
            p.drawLine(p1, p2);

            p.setPen(QPen(QColor(239, 68, 68, 240), 3, Qt::DashLine, Qt::RoundCap));
            p.drawLine(p1, p2);

            // ⛔ Roadblock Badge at midpoint
            QPointF mid = (p1 + p2) / 2.0;
            p.setBrush(QColor(220, 38, 38));
            p.setPen(QPen(Qt::white, 1.5));
            p.drawEllipse(mid, 7, 7);
            p.setPen(QPen(Qt::white, 2));
            p.drawLine(mid.x() - 4, mid.y(), mid.x() + 4, mid.y());
        } else {
            // Normal Road: Clean dark slate corridor
            p.setPen(QPen(QColor(30, 41, 59, 200), 5, Qt::SolidLine, Qt::RoundCap));
            p.drawLine(p1, p2);

            p.setPen(QPen(QColor(71, 85, 105, 180), 2, Qt::SolidLine, Qt::RoundCap));
            p.drawLine(p1, p2);
        }
    }

    // 3. Highlight Journey Route (BFS Path)
    if (selectedMode == 0 && currentPath.size() >= 2) {
        // Glowing Neon Cyan / Emerald Green Path
        for (size_t i = 0; i + 1 < currentPath.size(); ++i) {
            QPointF p1 = getCanvasPos(currentPath[i], mapRect);
            QPointF p2 = getCanvasPos(currentPath[i + 1], mapRect);

            // Wide ambient glow
            p.setPen(QPen(QColor(34, 195, 240, 70), 12, Qt::SolidLine, Qt::RoundCap));
            p.drawLine(p1, p2);

            // Medium vibrant glow
            p.setPen(QPen(QColor(91, 220, 255, 160), 6, Qt::SolidLine, Qt::RoundCap));
            p.drawLine(p1, p2);

            // Bright core line
            p.setPen(QPen(QColor(255, 255, 255, 230), 2.5, Qt::SolidLine, Qt::RoundCap));
            p.drawLine(p1, p2);

            // Direction arrow
            double angle = std::atan2(p2.y() - p1.y(), p2.x() - p1.x());
            QPointF mid = (p1 + p2) / 2.0;
            double arrowLen = 9.0;
            QPointF a1 = mid - QPointF(arrowLen * std::cos(angle - 0.5), arrowLen * std::sin(angle - 0.5));
            QPointF a2 = mid - QPointF(arrowLen * std::cos(angle + 0.5), arrowLen * std::sin(angle + 0.5));
            p.setPen(QPen(QColor(16, 185, 129), 2.5, Qt::SolidLine, Qt::RoundCap));
            p.drawLine(mid, a1);
            p.drawLine(mid, a2);
        }
    } else if (selectedMode == 1 && !currentJourney.empty()) {
        // Bus Transfers Journey with distinct bus colors
        QColor busColors[] = { QColor(34, 195, 240), QColor(217, 70, 239), QColor(16, 185, 129) };
        for (size_t leg = 0; leg < currentJourney.size(); ++leg) {
            QColor legColor = busColors[leg % 3];
            const auto& stops = currentJourney[leg].stopsInLeg;
            for (size_t s = 0; s + 1 < stops.size(); ++s) {
                QPointF p1 = getCanvasPos(stops[s], mapRect);
                QPointF p2 = getCanvasPos(stops[s + 1], mapRect);

                p.setPen(QPen(QColor(legColor.red(), legColor.green(), legColor.blue(), 70), 10, Qt::SolidLine, Qt::RoundCap));
                p.drawLine(p1, p2);
                p.setPen(QPen(legColor, 4, Qt::SolidLine, Qt::RoundCap));
                p.drawLine(p1, p2);
            }
        }
    } else if (selectedMode == 5 && !currentClosureReport.alternativeRoute.empty()) {
        // DFS Bypass Detour
        for (size_t i = 0; i + 1 < currentClosureReport.alternativeRoute.size(); ++i) {
            QPointF p1 = getCanvasPos(currentClosureReport.alternativeRoute[i], mapRect);
            QPointF p2 = getCanvasPos(currentClosureReport.alternativeRoute[i + 1], mapRect);

            p.setPen(QPen(QColor(16, 185, 129, 90), 8, Qt::SolidLine, Qt::RoundCap));
            p.drawLine(p1, p2);
            p.setPen(QPen(QColor(16, 185, 129), 3, Qt::SolidLine, Qt::RoundCap));
            p.drawLine(p1, p2);
        }
    }

    // 4. Animation Frontier Edges
    if (isAnimating) {
        for (int k = 0; k < animationStep && k < static_cast<int>(animationEdges.size()); ++k) {
            QPointF p1 = getCanvasPos(animationEdges[k].first, mapRect);
            QPointF p2 = getCanvasPos(animationEdges[k].second, mapRect);
            p.setPen(QPen(QColor(245, 158, 11, 180), 4, Qt::SolidLine, Qt::RoundCap));
            p.drawLine(p1, p2);
        }
    }

    // 5. Draw Bus Stops (Nodes)
    std::set<int> pathSet(currentPath.begin(), currentPath.end());
    std::set<int> visitedAnimSet;
    if (isAnimating) {
        for (int k = 0; k <= animationStep && k < static_cast<int>(animationVisitedOrder.size()); ++k) {
            visitedAnimSet.insert(animationVisitedOrder[k]);
        }
    }

    for (const auto& pair : graph->getAllStops()) {
        int id = pair.first;
        QPointF pt = getCanvasPos(id, mapRect);
        if (pt.isNull()) continue;

        bool isOrigin = (id == selectedSrc);
        bool isDest = (id == selectedDest);
        bool inPath = (pathSet.find(id) != pathSet.end());
        bool inAnim = (visitedAnimSet.find(id) != visitedAnimSet.end());

        double radius = (isOrigin || isDest || inPath) ? 10.0 : 7.0;

        // Outer halo / glow for special stops
        if (isOrigin) {
            p.setBrush(Qt::NoBrush);
            p.setPen(QPen(QColor(245, 158, 11, 100), 5));
            p.drawEllipse(pt, radius + 4, radius + 4);

            p.setBrush(QColor(245, 158, 11)); // Amber
            p.setPen(QPen(Qt::white, 2.5));
            p.drawEllipse(pt, radius, radius);
        } else if (isDest) {
            p.setBrush(Qt::NoBrush);
            p.setPen(QPen(QColor(34, 195, 240, 100), 5));
            p.drawEllipse(pt, radius + 4, radius + 4);

            p.setBrush(QColor(34, 195, 240)); // Cyan
            p.setPen(QPen(Qt::white, 2.5));
            p.drawEllipse(pt, radius, radius);
        } else if (inPath) {
            p.setBrush(QColor(16, 185, 129)); // Emerald Green
            p.setPen(QPen(Qt::white, 2));
            p.drawEllipse(pt, radius, radius);
        } else if (inAnim) {
            p.setBrush(QColor(249, 115, 22)); // Orange
            p.setPen(QPen(Qt::white, 1.8));
            p.drawEllipse(pt, radius, radius);
        } else {
            // Standard stop
            p.setBrush(QColor(51, 65, 85)); // Slate
            p.setPen(QPen(QColor(148, 163, 184), 1.5));
            p.drawEllipse(pt, radius, radius);
        }

        // Stop Label
        QString name = QString::fromStdString(pair.second.name);
        p.setFont(QFont("Plus Jakarta Sans", (isOrigin || isDest || inPath) ? 9 : 8,
                        (isOrigin || isDest || inPath) ? QFont::Bold : QFont::Normal));

        if (isOrigin) {
            p.setPen(QColor(254, 240, 138));
        } else if (isDest) {
            p.setPen(QColor(186, 230, 253));
        } else if (inPath) {
            p.setPen(QColor(167, 243, 208));
        } else {
            p.setPen(QColor(203, 213, 225));
        }

        p.drawText(QPointF(pt.x() + radius + 5, pt.y() + 4), name);
    }

    // 6. Interactive Floating Map Legend (Bottom-Right)
    int legW = 210, legH = 110;
    QRectF legRect(width() - legW - 20, height() - legH - 20, legW, legH);

    p.setBrush(QColor(15, 23, 42, 230)); // Glassmorphism dark
    p.setPen(QPen(QColor(255, 255, 255, 25), 1));
    p.drawRoundedRect(legRect, 8, 8);

    p.setFont(QFont("Plus Jakarta Sans", 8, QFont::Bold));
    p.setPen(QColor(148, 163, 184));
    p.drawText(legRect.left() + 14, legRect.top() + 18, "MAP LEGEND");

    auto drawLegendRow = [&](int yOff, const QColor& col, const QString& text) {
        p.setBrush(col);
        p.setPen(Qt::NoPen);
        p.drawEllipse(QPointF(legRect.left() + 18, legRect.top() + yOff), 5, 5);

        p.setFont(QFont("Plus Jakarta Sans", 8));
        p.setPen(QColor(226, 232, 240));
        p.drawText(QPointF(legRect.left() + 32, legRect.top() + yOff + 4), text);
    };

    drawLegendRow(38, QColor(245, 158, 11), "Origin / Boarding Point");
    drawLegendRow(56, QColor(34, 195, 240), "Target Destination");
    drawLegendRow(74, QColor(16, 185, 129), "Optimal BFS / Detour Path");
    drawLegendRow(92, QColor(239, 68, 68), "Closed Road Corridor");
}

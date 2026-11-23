#include "QtMapWidget.h"
#include <QPainterPath>
#include <QLinearGradient>
#include <QRadialGradient>
#include <cmath>

QtMapWidget::QtMapWidget(const Graph* g, QWidget* parent)
    : QWidget(parent), graph(g),
      selectedSrc(1), selectedDest(22), selectedMode(0),
      queryBusProgress(0.0), queryBusSpeed(0.025),
      queryBusAtTerminal(false), queryBusPauseTicks(0),
      streamDashOffset(0.0),
      isStepAnimating(false), stepIndex(0), stepMax(0), stepHopLevel(0) {

    initPositions();
    initAmbientFleet();

    // 30ms (~33 FPS) frame timer for smooth continuous motion
    frameTimer = new QTimer(this);
    connect(frameTimer, &QTimer::timeout, this, &QtMapWidget::onFrameTick);
    frameTimer->start(30);

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

void QtMapWidget::initAmbientFleet() {
    ambientBuses.clear();

    // 1. BRTC (Electric Cyan)
    ambientBuses.push_back({
        1, "BRTC", QColor(34, 195, 240),
        {1, 2, 3, 4, 6, 9, 14, 15, 16, 22},
        2, 0.3, 0.015, true
    });

    // 2. Raida (Emerald Green)
    ambientBuses.push_back({
        2, "Raida", QColor(16, 185, 129),
        {1, 2, 3, 4, 10, 11, 12, 23},
        4, 0.6, 0.018, true
    });

    // 3. Bikolpo (Amber Yellow)
    ambientBuses.push_back({
        3, "Bikolpo", QColor(245, 158, 11),
        {19, 20, 14, 15, 16, 22},
        1, 0.1, 0.020, true
    });

    // 4. Turag (Hot Pink)
    ambientBuses.push_back({
        4, "Turag", QColor(236, 72, 153),
        {1, 2, 6, 9, 14, 17, 18},
        3, 0.8, 0.016, true
    });

    // 5. Alif (Purple)
    ambientBuses.push_back({
        6, "Alif", QColor(168, 85, 247),
        {19, 6, 7, 8, 10, 11},
        0, 0.5, 0.019, true
    });
}

QPointF QtMapWidget::getCanvasPos(int stopId, const QRectF& area) const {
    auto it = nodePositions.find(stopId);
    if (it == nodePositions.end()) return QPointF(0, 0);

    double x = area.left() + it->second.nx * area.width();
    double y = area.top() + it->second.ny * area.height();
    return QPointF(x, y);
}

QPointF QtMapWidget::interpolatePos(int u, int v, double t, const QRectF& area, double& outAngle) const {
    QPointF p1 = getCanvasPos(u, area);
    QPointF p2 = getCanvasPos(v, area);

    double dx = p2.x() - p1.x();
    double dy = p2.y() - p1.y();
    outAngle = std::atan2(dy, dx);

    return QPointF(p1.x() + t * dx, p1.y() + t * dy);
}

void QtMapWidget::setSelectedStops(int src, int dest) {
    selectedSrc = src;
    selectedDest = dest;
    queryBusProgress = 0.0;
    queryBusPauseTicks = 0;
    update();
}

void QtMapWidget::setAlgorithmMode(int mode) {
    selectedMode = mode;
    queryBusProgress = 0.0;
    queryBusPauseTicks = 0;
    update();
}

void QtMapWidget::setPath(const std::vector<int>& path) {
    currentPath = path;
    queryBusProgress = 0.0;
    queryBusPauseTicks = 0;
    update();
}

void QtMapWidget::setJourney(const std::vector<JourneyLeg>& journey) {
    currentJourney = journey;
    currentPath.clear();
    for (const auto& leg : journey) {
        for (size_t i = 0; i < leg.stopsInLeg.size(); ++i) {
            if (currentPath.empty() || currentPath.back() != leg.stopsInLeg[i]) {
                currentPath.push_back(leg.stopsInLeg[i]);
            }
        }
    }
    queryBusProgress = 0.0;
    queryBusPauseTicks = 0;
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
    currentPath = report.alternativeRoute;
    queryBusProgress = 0.0;
    queryBusPauseTicks = 0;
    update();
}

void QtMapWidget::startStepAnimation(const std::vector<int>& nodes,
                                     const std::vector<std::pair<int, int>>& edges,
                                     const std::vector<QColor>& colors) {
    stepNodes = nodes;
    stepEdges = edges;
    stepColors = colors;
    stepIndex = 0;
    stepMax = static_cast<int>(nodes.size());
    isStepAnimating = true;
    update();
}

void QtMapWidget::stopStepAnimation() {
    isStepAnimating = false;
    stepIndex = 0;
    update();
}

void QtMapWidget::resetView() {
    stopStepAnimation();
    currentPath.clear();
    currentJourney.clear();
    currentComponents.clear();
    currentReachable.clear();
    queryBusProgress = 0.0;
    queryBusPauseTicks = 0;
    update();
}

void QtMapWidget::onFrameTick() {
    // 1. Advance marching dash offset for stream effect
    streamDashOffset -= 0.9;
    if (streamDashOffset < -100.0) streamDashOffset = 0.0;

    // 2. Advance Ambient Background Fleet
    for (auto& bus : ambientBuses) {
        if (bus.route.size() < 2) continue;

        bus.progress += bus.speed;
        if (bus.progress >= 1.0) {
            bus.progress = 0.0;
            if (bus.forward) {
                bus.currentSegment++;
                if (bus.currentSegment >= static_cast<int>(bus.route.size()) - 1) {
                    bus.forward = false;
                    bus.currentSegment = static_cast<int>(bus.route.size()) - 2;
                }
            } else {
                bus.currentSegment--;
                if (bus.currentSegment < 0) {
                    bus.forward = true;
                    bus.currentSegment = 0;
                }
            }
        }
    }

    // 3. Advance Queried Active Route Bus
    if (currentPath.size() >= 2) {
        if (queryBusPauseTicks > 0) {
            queryBusPauseTicks--;
            if (queryBusPauseTicks == 0) {
                queryBusProgress = 0.0; // Loop again from origin!
            }
        } else {
            queryBusProgress += queryBusSpeed;
            if (queryBusProgress >= static_cast<double>(currentPath.size() - 1)) {
                queryBusProgress = static_cast<double>(currentPath.size() - 1);
                queryBusPauseTicks = 30; // Pause ~1s at destination terminal
            }
        }
    }

    // 4. Advance Step-by-Step Animation (if active)
    if (isStepAnimating) {
        static int stepTickCounter = 0;
        stepTickCounter++;
        if (stepTickCounter >= 6) { // Every ~180ms
            stepTickCounter = 0;
            stepIndex++;
            if (stepIndex >= stepMax + 12) {
                stopStepAnimation();
            }
        }
    }

    update();
}

void QtMapWidget::drawVehicle(QPainter& p, const QPointF& pos, double angle,
                             const QColor& bodyColor, const QString& label, bool isLarge) {
    p.save();
    p.translate(pos);
    p.rotate(angle * 180.0 / 3.14159265358979323846);

    double w = isLarge ? 28.0 : 18.0;
    double h = isLarge ? 14.0 : 10.0;

    // Headlight Beams (casting forward)
    if (isLarge) {
        QRadialGradient beamGrad(w / 2.0 + 8.0, 0, 30);
        beamGrad.setColorAt(0.0, QColor(254, 240, 138, 120));
        beamGrad.setColorAt(1.0, QColor(254, 240, 138, 0));
        p.setBrush(beamGrad);
        p.setPen(Qt::NoPen);

        QPainterPath beamPath;
        beamPath.moveTo(w / 2.0, -h / 2.0 + 2);
        beamPath.lineTo(w / 2.0 + 35.0, -h - 8.0);
        beamPath.lineTo(w / 2.0 + 35.0, h + 8.0);
        beamPath.lineTo(w / 2.0, h / 2.0 - 2);
        beamPath.closeSubpath();
        p.drawPath(beamPath);
    }

    // Ambient vehicle glow
    p.setBrush(QColor(bodyColor.red(), bodyColor.green(), bodyColor.blue(), isLarge ? 80 : 40));
    p.setPen(Qt::NoPen);
    p.drawRoundedRect(QRectF(-w / 2.0 - 3, -h / 2.0 - 3, w + 6, h + 6), 5, 5);

    // Vehicle Body
    p.setBrush(bodyColor);
    p.setPen(QPen(QColor(255, 255, 255, isLarge ? 240 : 180), 1.5));
    p.drawRoundedRect(QRectF(-w / 2.0, -h / 2.0, w, h), 3, 3);

    // Windshield (Front Glass)
    p.setBrush(QColor(15, 23, 42, 220));
    p.setPen(Qt::NoPen);
    p.drawRect(QRectF(w / 2.0 - (isLarge ? 6.0 : 4.0), -h / 2.0 + 2.0, (isLarge ? 4.0 : 3.0), h - 4.0));

    // Vehicle Label / Number
    if (isLarge) {
        p.setFont(QFont("Plus Jakarta Sans", 7, QFont::Bold));
        p.setPen(Qt::white);
        p.drawText(QRectF(-w / 2.0, -h / 2.0, w - 4, h), Qt::AlignCenter, label);
    }

    p.restore();
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
    for (int x = 40; x < width(); x += 60) p.drawLine(x, 0, x, height());
    for (int y = 40; y < height(); y += 60) p.drawLine(0, y, width(), y);

    // Header Title Overlay
    p.setFont(QFont("Plus Jakarta Sans", 11, QFont::Bold));
    p.setPen(QColor(91, 220, 255, 140)); // #5bdcff
    p.drawText(24, 30, "DHAKA METROPOLITAN BUS TRANSIT NETWORK");

    p.setFont(QFont("Plus Jakarta Sans", 9));
    p.setPen(QColor(148, 163, 184, 180));
    p.drawText(24, 48, "Live Vehicle Simulation • Continuous Transit Motion • BFS & DFS Visualizer");

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
            p.setPen(QPen(QColor(239, 68, 68, 60), 7, Qt::SolidLine, Qt::RoundCap));
            p.drawLine(p1, p2);

            p.setPen(QPen(QColor(239, 68, 68, 240), 3, Qt::DashLine, Qt::RoundCap));
            p.drawLine(p1, p2);

            // ⛔ Roadblock Badge at midpoint
            QPointF mid = (p1 + p2) / 2.0;
            p.setBrush(QColor(220, 38, 38));
            p.setPen(QPen(Qt::white, 1.5));
            p.drawEllipse(mid, 8, 8);
            p.setPen(QPen(Qt::white, 2));
            p.drawLine(mid.x() - 4, mid.y(), mid.x() + 4, mid.y());
        } else {
            // Normal Road: Clean dark slate corridor
            p.setPen(QPen(QColor(30, 41, 59, 220), 5, Qt::SolidLine, Qt::RoundCap));
            p.drawLine(p1, p2);

            p.setPen(QPen(QColor(71, 85, 105, 190), 2, Qt::SolidLine, Qt::RoundCap));
            p.drawLine(p1, p2);
        }
    }

    // 3. Highlight Queried Journey Route (with animated marching stream)
    if (currentPath.size() >= 2) {
        for (size_t i = 0; i + 1 < currentPath.size(); ++i) {
            QPointF p1 = getCanvasPos(currentPath[i], mapRect);
            QPointF p2 = getCanvasPos(currentPath[i + 1], mapRect);

            // Ambient cyan glow
            p.setPen(QPen(QColor(34, 195, 240, 70), 14, Qt::SolidLine, Qt::RoundCap));
            p.drawLine(p1, p2);

            // Core solid path
            p.setPen(QPen(QColor(14, 165, 233, 200), 5, Qt::SolidLine, Qt::RoundCap));
            p.drawLine(p1, p2);

            // Marching dashed electric cyan stream (like website stroke-dashoffset)
            QPen streamPen(QColor(255, 255, 255, 230), 2.5);
            QVector<qreal> dashes;
            dashes << 8 << 8;
            streamPen.setDashPattern(dashes);
            streamPen.setDashOffset(streamDashOffset);
            p.setPen(streamPen);
            p.drawLine(p1, p2);
        }
    }

    // 4. Highlight Step Animation Edges (for all modes)
    if (isStepAnimating) {
        for (int k = 0; k < stepIndex && k < static_cast<int>(stepEdges.size()); ++k) {
            QPointF p1 = getCanvasPos(stepEdges[k].first, mapRect);
            QPointF p2 = getCanvasPos(stepEdges[k].second, mapRect);
            QColor c = (k < static_cast<int>(stepColors.size())) ? stepColors[k] : QColor(245, 158, 11);
            p.setPen(QPen(c, 4, Qt::SolidLine, Qt::RoundCap));
            p.drawLine(p1, p2);
        }
    }

    // 5. Draw Ambient Background Buses (Continuous City Simulation)
    for (const auto& bus : ambientBuses) {
        if (bus.route.size() < 2) continue;
        int seg = bus.currentSegment;
        if (seg < 0 || seg >= static_cast<int>(bus.route.size()) - 1) continue;

        int u = bus.forward ? bus.route[seg] : bus.route[seg + 1];
        int v = bus.forward ? bus.route[seg + 1] : bus.route[seg];

        double angle = 0.0;
        QPointF vPos = interpolatePos(u, v, bus.progress, mapRect, angle);
        drawVehicle(p, vPos, angle, bus.color, bus.name, false);
    }

    // 6. Draw Continuous Active Route Bus (Queried Route)
    if (currentPath.size() >= 2) {
        int segIdx = static_cast<int>(std::floor(queryBusProgress));
        if (segIdx >= static_cast<int>(currentPath.size()) - 1) {
            segIdx = static_cast<int>(currentPath.size()) - 2;
        }
        double segT = queryBusProgress - segIdx;
        if (segT < 0.0) segT = 0.0;
        if (segT > 1.0) segT = 1.0;

        int u = currentPath[segIdx];
        int v = currentPath[segIdx + 1];

        double angle = 0.0;
        QPointF busPos = interpolatePos(u, v, segT, mapRect, angle);

        // Prominent glowing active bus (Electric Cyan)
        drawVehicle(p, busPos, angle, QColor(34, 195, 240), "BUS", true);
    }

    // 7. Draw Bus Stops (Nodes)
    std::set<int> pathSet(currentPath.begin(), currentPath.end());
    std::set<int> stepActiveNodes;
    if (isStepAnimating) {
        for (int k = 0; k <= stepIndex && k < static_cast<int>(stepNodes.size()); ++k) {
            stepActiveNodes.insert(stepNodes[k]);
        }
    }

    for (const auto& pair : graph->getAllStops()) {
        int id = pair.first;
        QPointF pt = getCanvasPos(id, mapRect);
        if (pt.isNull()) continue;

        bool isOrigin = (id == selectedSrc);
        bool isDest = (id == selectedDest);
        bool inPath = (pathSet.find(id) != pathSet.end());
        bool inStepAnim = (stepActiveNodes.find(id) != stepActiveNodes.end());

        double radius = (isOrigin || isDest || inPath) ? 10.0 : 7.0;

        if (isOrigin) {
            // Golden-Yellow Origin Pin
            p.setBrush(Qt::NoBrush);
            p.setPen(QPen(QColor(245, 158, 11, 120), 6));
            p.drawEllipse(pt, radius + 4, radius + 4);

            p.setBrush(QColor(245, 158, 11));
            p.setPen(QPen(Qt::white, 2.5));
            p.drawEllipse(pt, radius, radius);
        } else if (isDest) {
            // Bright Cyan Target Pin
            p.setBrush(Qt::NoBrush);
            p.setPen(QPen(QColor(34, 195, 240, 120), 6));
            p.drawEllipse(pt, radius + 4, radius + 4);

            p.setBrush(QColor(34, 195, 240));
            p.setPen(QPen(Qt::white, 2.5));
            p.drawEllipse(pt, radius, radius);
        } else if (inPath) {
            // Emerald Green Route Pin
            p.setBrush(QColor(16, 185, 129));
            p.setPen(QPen(Qt::white, 2));
            p.drawEllipse(pt, radius, radius);
        } else if (inStepAnim) {
            // Active animation step pin
            p.setBrush(QColor(249, 115, 22)); // Orange
            p.setPen(QPen(Qt::white, 2));
            p.drawEllipse(pt, radius + 2, radius + 2);
        } else {
            // Standard stop pin
            p.setBrush(QColor(51, 65, 85));
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

    // 8. Floating Map HUD / Legend (Bottom-Right)
    int legW = 230, legH = 125;
    QRectF legRect(width() - legW - 20, height() - legH - 20, legW, legH);

    p.setBrush(QColor(15, 23, 42, 235));
    p.setPen(QPen(QColor(255, 255, 255, 25), 1));
    p.drawRoundedRect(legRect, 8, 8);

    p.setFont(QFont("Plus Jakarta Sans", 8, QFont::Bold));
    p.setPen(QColor(148, 163, 184));
    p.drawText(legRect.left() + 14, legRect.top() + 18, "LIVE TRANSIT SIMULATION");

    auto drawLegendRow = [&](int yOff, const QColor& col, const QString& text) {
        p.setBrush(col);
        p.setPen(Qt::NoPen);
        p.drawEllipse(QPointF(legRect.left() + 18, legRect.top() + yOff), 5, 5);

        p.setFont(QFont("Plus Jakarta Sans", 8));
        p.setPen(QColor(226, 232, 240));
        p.drawText(QPointF(legRect.left() + 32, legRect.top() + yOff + 4), text);
    };

    drawLegendRow(36, QColor(245, 158, 11), "Origin Stop (Boarding)");
    drawLegendRow(54, QColor(34, 195, 240), "Target Destination");
    drawLegendRow(72, QColor(16, 185, 129), "Active Continuous Route");
    drawLegendRow(90, QColor(239, 68, 68),  "Closed Road Corridor");
    drawLegendRow(108, QColor(236, 72, 153), "Fleet Buses (Live Motion)");
}

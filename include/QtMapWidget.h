#ifndef QT_MAP_WIDGET_H
#define QT_MAP_WIDGET_H

#include <QWidget>
#include <QTimer>
#include <QPainter>
#include <QPaintEvent>
#include <map>
#include <vector>
#include <string>
#include "Graph.h"
#include "Bus.h"

struct MapNodePos {
    double nx; // Normalized X (0.0 to 1.0)
    double ny; // Normalized Y (0.0 to 1.0)
};

// Represents an ambient background bus moving continuously
struct AmbientBus {
    int busId;
    QString name;
    QColor color;
    std::vector<int> route;
    int currentSegment;
    double progress; // 0.0 to 1.0 on current segment
    double speed;
    bool forward;
};

class QtMapWidget : public QWidget {
    Q_OBJECT

private:
    const Graph* graph;
    std::map<int, MapNodePos> nodePositions;

    int selectedSrc;
    int selectedDest;
    int selectedMode;

    std::vector<int> currentPath;
    std::vector<JourneyLeg> currentJourney;
    std::vector<std::vector<int>> currentComponents;
    std::vector<std::pair<int, int>> currentReachable;
    RoadClosureAnalysis currentClosureReport;

    // Ambient background traffic
    std::vector<AmbientBus> ambientBuses;

    // Queried active bus running continuously on current route
    double queryBusProgress;   // 0.0 to path.size() - 1
    double queryBusSpeed;
    bool queryBusAtTerminal;
    int queryBusPauseTicks;

    // Marching dash stream offset
    double streamDashOffset;

    // Step-by-step exploration animation
    bool isStepAnimating;
    int stepIndex;
    int stepMax;
    std::vector<int> stepNodes;
    std::vector<std::pair<int, int>> stepEdges;
    std::vector<QColor> stepColors;
    int stepHopLevel;

    // Timers
    QTimer* frameTimer; // 30ms (~33fps) continuous vehicle & stream animation

    void initPositions();
    void initAmbientFleet();
    QPointF getCanvasPos(int stopId, const QRectF& area) const;
    QPointF interpolatePos(int u, int v, double t, const QRectF& area, double& outAngle) const;

    void drawRoadCorridor(QPainter& p, int u, int v, const QRectF& area,
                          const QColor& color, int width, bool isBlocked);
    void drawVehicle(QPainter& p, const QPointF& pos, double angle,
                     const QColor& bodyColor, const QString& label, bool isLarge);

protected:
    void paintEvent(QPaintEvent* event) override;

private slots:
    void onFrameTick();

public:
    explicit QtMapWidget(const Graph* g, QWidget* parent = nullptr);
    ~QtMapWidget();

    void setSelectedStops(int src, int dest);
    void setAlgorithmMode(int mode);
    void setPath(const std::vector<int>& path);
    void setJourney(const std::vector<JourneyLeg>& journey);
    void setComponents(const std::vector<std::vector<int>>& comps);
    void setReachable(const std::vector<std::pair<int, int>>& reach);
    void setClosureReport(const RoadClosureAnalysis& report);

    void startStepAnimation(const std::vector<int>& nodes,
                            const std::vector<std::pair<int, int>>& edges,
                            const std::vector<QColor>& colors = {});
    void stopStepAnimation();
    void resetView();
};

#endif // QT_MAP_WIDGET_H

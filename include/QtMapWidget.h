#ifndef QT_MAP_WIDGET_H
#define QT_MAP_WIDGET_H

#include <QWidget>
#include <QTimer>
#include <QPainter>
#include <QPaintEvent>
#include <map>
#include <vector>
#include "Graph.h"
#include "Bus.h"

struct MapNodePos {
    double nx; // Normalized X (0.0 to 1.0)
    double ny; // Normalized Y (0.0 to 1.0)
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

    // Animation state
    bool isAnimating;
    int animationStep;
    int animationTotalSteps;
    std::vector<int> animationVisitedOrder;
    std::vector<std::pair<int, int>> animationEdges;
    QTimer* animTimer;
    double vehicleProgress; // 0.0 to 1.0 for moving bus along path

    void initPositions();
    QPointF getCanvasPos(int stopId, const QRectF& area) const;

protected:
    void paintEvent(QPaintEvent* event) override;

private slots:
    void onAnimationTick();

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

    void startStepAnimation(const std::vector<int>& visitOrder, const std::vector<std::pair<int, int>>& edges);
    void stopAnimation();
    void resetView();
};

#endif // QT_MAP_WIDGET_H

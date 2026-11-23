#ifndef QT_MAIN_WINDOW_H
#define QT_MAIN_WINDOW_H

#include <QMainWindow>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QGroupBox>
#include <QFrame>
#include "Graph.h"
#include "BusManager.h"
#include "DataManager.h"
#include "QtMapWidget.h"

class QtMainWindow : public QMainWindow {
    Q_OBJECT

private:
    Graph graph;
    BusManager busMgr;
    DataManager dataMgr;

    // Controls
    QComboBox* comboSrc;
    QComboBox* comboDest;
    QComboBox* comboMode;
    QComboBox* comboRoadBlock;

    QPushButton* btnRun;
    QPushButton* btnAnimate;
    QPushButton* btnReset;

    // Metrics Labels
    QLabel* lblStatStops;
    QLabel* lblStatHops;
    QLabel* lblStatDist;
    QLabel* lblStatTime;

    // Output containers
    QWidget* recContainer;
    QVBoxLayout* recLayout;
    QTextEdit* textLog;

    // Map Widget
    QtMapWidget* mapWidget;

    void setupUI();
    void applyDarkTheme();
    void populateDropdowns();

    void executeAlgorithm(bool startAnimation = false);
    void updateMetricsAndCards(const std::vector<int>& path,
                               const std::vector<BusRecommendation>& recs,
                               const std::string& logText);

private slots:
    void onRunClicked();
    void onAnimateClicked();
    void onResetClicked();
    void onSelectionChanged();

public:
    explicit QtMainWindow(QWidget* parent = nullptr);
    ~QtMainWindow();
};

#endif // QT_MAIN_WINDOW_H

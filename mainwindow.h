#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QSlider>
#include <QRadioButton>
#include <QButtonGroup>
#include <QPushButton>
#include <QListWidget>
#include <QInputDialog>
#include <QComboBox>
#include <QDockWidget>
#include <QTableWidget>
#include <QTabWidget>
#include <QHeaderView>
#include "engine/engine.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

enum class DeformViewMode { Undeformed, Deformed, Both };
enum class DiagramMode    { None, SFD, BMD, EnvelopeMoment, EnvelopeShear };

struct GlobalMomentResult
{
    bool          valid  = false;
    const Member* member = nullptr;
    double        x      = 0.0;
    double        value  = 0.0;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onViewClicked(const QPointF& pos);
    void onMouseMoved(const QPointF& pos);
    void onLeftPressed(const QPointF& pos);
    void onMouseReleased();

    void addNodalLoad();
    void addUDL();
    void addPartialUDL();
    void deleteLoad();

private:
    Ui::MainWindow *ui;
    QGraphicsScene *scene;
    Engine engine;

    // ── Core helpers ─────────────────────────────────────
    void redrawScene();
    void handleRightClick(const QPointF& pos);
    int  findNodeAtPosition(const QPointF& scenePos);
    int  findMemberAtPosition(const QPointF& pos);
    QPointF snapToGrid(const QPointF& scenePos) const;

    // ── Diagram drawing ──────────────────────────────────
    void drawSFD(const Member& m, const Node& ni, const Node& nj, double scale);
    void drawBMD(const Member& m, const Node& ni, const Node& nj, double scale);
    void drawEnvelopeMoment(const Member& m, const Node& ni, const Node& nj,
                            int memberIndex, double scale);
    void drawEnvelopeShear(const Member& m, const Node& ni, const Node& nj,
                           int memberIndex, double scale);

    // ── Labels ───────────────────────────────────────────
    void addSFDLabels(size_t index, const QPointF& pi, const QPointF& pj,
                      const QPointF& normal, double scale);
    void addBMDLabels(size_t index, const QPointF& pi, const QPointF& pj,
                      const QPointF& normal, double scale);

    // ── Scale helpers ─────────────────────────────────────
    double computeDiagramScale() const;
    void   computeGlobalMaxMoment();

    // ── Results tables ───────────────────────────────────
    void setupMemberTable();
    void updateMemberTable();
    void setupEnvelopeTable();
    void updateEnvelopeTable();

    // ── Load UI helpers ──────────────────────────────────
    void refreshLoadList();
    void saveModel();
    void loadModel();
    void updateAnalysis();

    // ── State ────────────────────────────────────────────
    GlobalMomentResult globalMaxMoment;
    int    selectedNode     = -1;
    double deformationScale = 1.0;
    int    draggedNodeId    = -1;
    bool   isDragging       = false;

    DeformViewMode deformViewMode = DeformViewMode::Both;
    DiagramMode    diagramMode    = DiagramMode::None;

    // ── Widgets ──────────────────────────────────────────
    QSlider* scaleSlider;

    QRadioButton* rbUndeformed;
    QRadioButton* rbDeformed;
    QRadioButton* rbBoth;
    QButtonGroup* deformGroup;

    QRadioButton* rbNone;
    QRadioButton* rbSFD;
    QRadioButton* rbBMD;
    QRadioButton* rbEnvelopeMoment;
    QRadioButton* rbEnvelopeShear;
    QButtonGroup* diagramGroup;

    QListWidget*  loadList;
    QPushButton*  btnAddNodal;
    QPushButton*  btnAddUDL;
    QPushButton*  btnAddPartialUDL;
    QPushButton*  btnDeleteLoad;
    QPushButton*  btnSave;
    QPushButton*  btnLoad;

    QComboBox*    comboLoadCases;
    QPushButton*  btnAddLoadCase;
    QComboBox*    comboLoadCombinations;
    QPushButton*  btnAddCombination;
    QRadioButton* rbUseCase;
    QRadioButton* rbUseCombo;

    // Results dock
    QDockWidget*  resultsDock;
    QTabWidget*   resultTabs;
    QTableWidget* memberTable;
    QTableWidget* envelopeTable;   // ← NEW
};

#endif // MAINWINDOW_H

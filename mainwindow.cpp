#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <algorithm>
#include <cmath>
#include <QMenu>
#include "ui/dialogs/loaddialog.h"
#include <QGraphicsTextItem>
#include <QLabel>
#include <QPushButton>
#include <QListWidget>
#include <QInputDialog>
#include <QFileDialog>
#include <QComboBox>
#include <QPainterPath>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QColor>
#include <QFont>

constexpr double LOAD_SCALE = 5.0;

// ─────────────────────────────────────────────────────────────────────────────
//  Constructor
// ─────────────────────────────────────────────────────────────────────────────
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    scene = new QGraphicsScene(this);
    ui->graphicsView->setScene(scene);
    ui->graphicsView->setRenderHint(QPainter::Antialiasing);

    // ── Deformation scale slider ───────────────────────────
    scaleSlider = new QSlider(Qt::Horizontal, this);
    scaleSlider->setRange(10, 300);
    scaleSlider->setValue(100);
    scaleSlider->setGeometry(10, 10, 200, 25);

    QLabel* scaleLabel = new QLabel("Scale: 1.0x", this);
    scaleLabel->setGeometry(220, 10, 80, 25);

    connect(scaleSlider, &QSlider::valueChanged, this,
            [this, scaleLabel](int value)
            {
                deformationScale = value / 100.0;
                scaleLabel->setText(
                    QString("Scale: %1x").arg(deformationScale, 0, 'f', 2));
                redrawScene();
            });

    // ── Deformed / Undeformed radio buttons ───────────────
    rbUndeformed = new QRadioButton("Undeformed", this);
    rbDeformed   = new QRadioButton("Deformed",   this);
    rbBoth       = new QRadioButton("Both",       this);

    rbUndeformed->setGeometry(10, 45, 100, 20);
    rbDeformed->setGeometry(10,   65, 100, 20);
    rbBoth->setGeometry(10,       85, 100, 20);
    rbBoth->setChecked(true);

    deformGroup = new QButtonGroup(this);
    deformGroup->addButton(rbUndeformed);
    deformGroup->addButton(rbDeformed);
    deformGroup->addButton(rbBoth);

    connect(rbUndeformed, &QRadioButton::toggled, this, [this](bool c){
        if (c) { deformViewMode = DeformViewMode::Undeformed; redrawScene(); }
    });
    connect(rbDeformed, &QRadioButton::toggled, this, [this](bool c){
        if (c) { deformViewMode = DeformViewMode::Deformed; redrawScene(); }
    });
    connect(rbBoth, &QRadioButton::toggled, this, [this](bool c){
        if (c) { deformViewMode = DeformViewMode::Both; redrawScene(); }
    });

    // ── Diagram mode radio buttons ─────────────────────────
    rbNone           = new QRadioButton("Structure",   this);
    rbSFD            = new QRadioButton("SFD",         this);
    rbBMD            = new QRadioButton("BMD",         this);
    rbEnvelopeMoment = new QRadioButton("Env. Moment", this);
    rbEnvelopeShear  = new QRadioButton("Env. Shear",  this);

    rbNone->setGeometry(10,           120, 120, 20);
    rbSFD->setGeometry(10,            140, 120, 20);
    rbBMD->setGeometry(10,            160, 120, 20);
    rbEnvelopeMoment->setGeometry(10, 180, 120, 20);
    rbEnvelopeShear->setGeometry(10,  200, 120, 20);

    rbNone->setChecked(true);

    diagramGroup = new QButtonGroup(this);
    diagramGroup->addButton(rbNone);
    diagramGroup->addButton(rbSFD);
    diagramGroup->addButton(rbBMD);
    diagramGroup->addButton(rbEnvelopeMoment);
    diagramGroup->addButton(rbEnvelopeShear);

    connect(rbNone, &QRadioButton::toggled, this, [this](bool c){
        if (c) { diagramMode = DiagramMode::None; redrawScene(); }
    });
    connect(rbSFD, &QRadioButton::toggled, this, [this](bool c){
        if (c) { diagramMode = DiagramMode::SFD; redrawScene(); }
    });
    connect(rbBMD, &QRadioButton::toggled, this, [this](bool c){
        if (c) { diagramMode = DiagramMode::BMD; redrawScene(); }
    });
    connect(rbEnvelopeMoment, &QRadioButton::toggled, this, [this](bool c){
        if (c) {
            diagramMode = DiagramMode::EnvelopeMoment;
            if (!engine.isEnvelopeComputed()) engine.computeEnvelope();
            redrawScene();
        }
    });
    connect(rbEnvelopeShear, &QRadioButton::toggled, this, [this](bool c){
        if (c) {
            diagramMode = DiagramMode::EnvelopeShear;
            if (!engine.isEnvelopeComputed()) engine.computeEnvelope();
            redrawScene();
        }
    });

    // ── Load panel ────────────────────────────────────────
    btnAddNodal      = new QPushButton("Add Nodal Load", this);
    btnAddUDL        = new QPushButton("Add UDL",        this);
    btnAddPartialUDL = new QPushButton("Add Partial UDL",this);
    btnDeleteLoad    = new QPushButton("Delete Load",    this);
    loadList         = new QListWidget(this);

    btnAddNodal->setGeometry(10,      230, 150, 25);
    btnAddUDL->setGeometry(10,        260, 150, 25);
    btnAddPartialUDL->setGeometry(10, 290, 150, 25);
    loadList->setGeometry(10,         320, 200, 130);
    btnDeleteLoad->setGeometry(10,    460, 150, 25);

    connect(btnAddNodal,      &QPushButton::clicked, this, &MainWindow::addNodalLoad);
    connect(btnAddUDL,        &QPushButton::clicked, this, &MainWindow::addUDL);
    connect(btnAddPartialUDL, &QPushButton::clicked, this, &MainWindow::addPartialUDL);
    connect(btnDeleteLoad,    &QPushButton::clicked, this, &MainWindow::deleteLoad);

    // ── Save / Load ────────────────────────────────────────
    btnSave = new QPushButton("Save Model", this);
    btnLoad = new QPushButton("Load Model", this);
    btnSave->setGeometry(10, 495, 150, 25);
    btnLoad->setGeometry(10, 525, 150, 25);
    connect(btnSave, &QPushButton::clicked, this, &MainWindow::saveModel);
    connect(btnLoad, &QPushButton::clicked, this, &MainWindow::loadModel);

    // ── Load case combo ────────────────────────────────────
    comboLoadCases = new QComboBox(this);
    btnAddLoadCase = new QPushButton("Add Load Case", this);
    comboLoadCases->setGeometry(10, 560, 200, 25);
    btnAddLoadCase->setGeometry(10, 590, 150, 25);

    for (int i = 0; i < engine.getLoadCaseCount(); ++i)
        comboLoadCases->addItem(
            QString::fromStdString(engine.getLoadCases()[i].name));

    connect(btnAddLoadCase, &QPushButton::clicked, this, [this]()
            {
                int newIndex = engine.getLoadCaseCount() + 1;
                std::string name = "Load Case " + std::to_string(newIndex);
                engine.addLoadCase(name);
                comboLoadCases->addItem(QString::fromStdString(name));
            });

    connect(comboLoadCases,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this,
            [this](int index)
            {
                engine.setActiveLoadCase(index);
                refreshLoadList();
                if (rbUseCase->isChecked()) {
                    engine.solve();
                    computeGlobalMaxMoment();
                    updateMemberTable();
                    updateEnvelopeTable();
                    redrawScene();
                }
            });

    // ── Load combination combo ─────────────────────────────
    comboLoadCombinations = new QComboBox(this);
    btnAddCombination     = new QPushButton("Add Combination", this);
    rbUseCase             = new QRadioButton("Use Load Case",  this);
    rbUseCombo            = new QRadioButton("Use Combination",this);

    rbUseCase->setGeometry(220,             560, 150, 20);
    rbUseCombo->setGeometry(220,            580, 150, 20);
    comboLoadCombinations->setGeometry(220, 610, 200, 25);
    btnAddCombination->setGeometry(220,     640, 150, 25);

    rbUseCase->setChecked(true);

    connect(rbUseCase, &QRadioButton::toggled, this, [this](bool checked)
            {
                if (checked) {
                    engine.setActiveLoadCase(comboLoadCases->currentIndex());
                    computeGlobalMaxMoment();
                    updateMemberTable();
                    updateEnvelopeTable();
                    redrawScene();
                }
            });

    connect(rbUseCombo, &QRadioButton::toggled, this, [this](bool checked)
            {
                if (checked) {
                    int idx = comboLoadCombinations->currentIndex();
                    engine.setActiveLoadCombination(idx);
                    computeGlobalMaxMoment();
                    updateMemberTable();
                    updateEnvelopeTable();
                    redrawScene();
                }
            });

    connect(btnAddCombination, &QPushButton::clicked, this, [this]()
            {
                int newIndex = engine.getLoadCombinationCount() + 1;
                std::string name = "Combo " + std::to_string(newIndex);
                engine.addLoadCombination(name);
                comboLoadCombinations->addItem(QString::fromStdString(name));
            });

    connect(comboLoadCombinations,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this,
            [this](int index)
            {
                if (rbUseCombo->isChecked()) {
                    engine.setActiveLoadCombination(index);
                    computeGlobalMaxMoment();
                    updateMemberTable();
                    updateEnvelopeTable();
                    redrawScene();
                }
            });

    // ── View signals ───────────────────────────────────────
    connect(ui->graphicsView, &GraphicsView::leftClicked,
            this, &MainWindow::onViewClicked);
    connect(ui->graphicsView, &GraphicsView::leftPressed,
            this, &MainWindow::onLeftPressed);
    connect(ui->graphicsView, &GraphicsView::rightClicked,
            this, &MainWindow::handleRightClick);
    connect(ui->graphicsView, &GraphicsView::mouseMoved,
            this, &MainWindow::onMouseMoved);
    connect(ui->graphicsView, &GraphicsView::mouseReleased,
            this, &MainWindow::onMouseReleased);

    // ── Results dock ───────────────────────────────────────
    resultsDock = new QDockWidget("Results", this);
    resultsDock->setAllowedAreas(Qt::BottomDockWidgetArea | Qt::RightDockWidgetArea);
    addDockWidget(Qt::BottomDockWidgetArea, resultsDock);

    resultTabs = new QTabWidget(resultsDock);
    resultsDock->setWidget(resultTabs);

    memberTable   = new QTableWidget(this);
    envelopeTable = new QTableWidget(this);

    resultTabs->addTab(memberTable,   "Member Forces");
    resultTabs->addTab(envelopeTable, "Envelope");

    setupMemberTable();
    setupEnvelopeTable();

    redrawScene();
}

MainWindow::~MainWindow()
{
    delete ui;
}

// ─────────────────────────────────────────────────────────────────────────────
//  Member Forces Table
// ─────────────────────────────────────────────────────────────────────────────
void MainWindow::setupMemberTable()
{
    memberTable->setColumnCount(7);

    QStringList headers;
    headers << "Member" << "N1 (kN)" << "V1 (kN)" << "M1 (kNm)"
            << "N2 (kN)" << "V2 (kN)" << "M2 (kNm)";
    memberTable->setHorizontalHeaderLabels(headers);
    memberTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    memberTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    memberTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    memberTable->setAlternatingRowColors(true);
    memberTable->horizontalHeader()->setStyleSheet(
        "QHeaderView::section {"
        "  background-color: #2c3e50; color: white;"
        "  font-weight: bold; padding: 4px; border: 1px solid #1a252f; }");
}

void MainWindow::updateMemberTable()
{
    const auto& members = engine.getMembers();
    const auto& result  = engine.getActiveResult();

    memberTable->setRowCount((int)members.size());

    if (!result.valid || result.memberForces.empty())
    {
        for (int i = 0; i < (int)members.size(); ++i)
        {
            memberTable->setItem(i, 0,
                                 new QTableWidgetItem(QString::number(members[i].id)));
            for (int col = 1; col <= 6; ++col)
                memberTable->setItem(i, col, new QTableWidgetItem("—"));
        }
        return;
    }

    double maxN = 0, maxV = 0, maxM = 0;
    for (int i = 0; i < (int)members.size(); ++i)
    {
        const auto& mf = result.memberForces[i];
        maxN = std::max(maxN, std::max(std::abs(mf.N1), std::abs(mf.N2)));
        maxV = std::max(maxV, std::max(std::abs(mf.V1), std::abs(mf.V2)));
        maxM = std::max(maxM, std::max(std::abs(mf.M1), std::abs(mf.M2)));
    }

    auto makeCell = [](double val, double absMax, bool isPeak) -> QTableWidgetItem*
    {
        auto* item = new QTableWidgetItem(QString::number(val, 'f', 3));
        item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        if (isPeak && absMax > 1e-9)
        {
            item->setForeground(QColor(180, 0, 0));
            QFont f = item->font(); f.setBold(true); item->setFont(f);
        }
        return item;
    };

    for (int i = 0; i < (int)members.size(); ++i)
    {
        const auto& m  = members[i];
        const auto& mf = result.memberForces[i];

        auto* idItem = new QTableWidgetItem(QString::number(m.id));
        idItem->setTextAlignment(Qt::AlignCenter);
        QFont bf = idItem->font(); bf.setBold(true); idItem->setFont(bf);
        memberTable->setItem(i, 0, idItem);

        memberTable->setItem(i, 1, makeCell(mf.N1, maxN, maxN > 1e-9 && std::abs(mf.N1) == maxN));
        memberTable->setItem(i, 2, makeCell(mf.V1, maxV, maxV > 1e-9 && std::abs(mf.V1) == maxV));
        memberTable->setItem(i, 3, makeCell(mf.M1, maxM, maxM > 1e-9 && std::abs(mf.M1) == maxM));
        memberTable->setItem(i, 4, makeCell(mf.N2, maxN, maxN > 1e-9 && std::abs(mf.N2) == maxN));
        memberTable->setItem(i, 5, makeCell(mf.V2, maxV, maxV > 1e-9 && std::abs(mf.V2) == maxV));
        memberTable->setItem(i, 6, makeCell(mf.M2, maxM, maxM > 1e-9 && std::abs(mf.M2) == maxM));
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  Envelope Table
// ─────────────────────────────────────────────────────────────────────────────
void MainWindow::setupEnvelopeTable()
{
    envelopeTable->setColumnCount(9);

    QStringList headers;
    headers << "Member"
            << "Max M" << "Source"
            << "Min M" << "Source"
            << "Max V" << "Source"
            << "Min V" << "Source";
    envelopeTable->setHorizontalHeaderLabels(headers);

    envelopeTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    envelopeTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    envelopeTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    envelopeTable->setAlternatingRowColors(true);

    envelopeTable->horizontalHeader()->setStyleSheet(
        "QHeaderView::section {"
        "  background-color: #1a3a5c; color: white;"
        "  font-weight: bold; padding: 4px; border: 1px solid #112844; }");
}

void MainWindow::updateEnvelopeTable()
{
    const auto& members = engine.getMembers();
    const auto& combos  = engine.getLoadCombinations();
    const auto& cases   = engine.getLoadCases();

    envelopeTable->setRowCount((int)members.size());

    auto envelope = engine.computeEnvelopeTable();

    // Helper: convert encoded source index to a display name
    // combo index >= 0  → combo name
    // combo index < 0   → -(lcIndex+1) → load case name
    auto sourceName = [&](int src) -> QString
    {
        if (src < 0)
        {
            int lcIdx = -(src + 1);
            if (lcIdx >= 0 && lcIdx < (int)cases.size())
                return QString::fromStdString(cases[lcIdx].name);
            return "—";
        }
        if (src < (int)combos.size())
            return QString::fromStdString(combos[src].name);
        return "—";
    };

    // Find global peaks for highlight
    double absMaxM = 0, absMinM = 0, absMaxV = 0, absMinV = 0;
    for (const auto& e : envelope)
    {
        absMaxM = std::max(absMaxM, std::abs(e.maxM));
        absMinM = std::max(absMinM, std::abs(e.minM));
        absMaxV = std::max(absMaxV, std::abs(e.maxV));
        absMinV = std::max(absMinV, std::abs(e.minV));
    }

    for (int i = 0; i < (int)members.size(); ++i)
    {
        const auto& e = envelope[i];

        // ── Col 0: Member ID ──
        auto* idItem = new QTableWidgetItem(QString::number(members[i].id));
        idItem->setTextAlignment(Qt::AlignCenter);
        QFont bf = idItem->font(); bf.setBold(true); idItem->setFont(bf);
        envelopeTable->setItem(i, 0, idItem);

        // Helper lambda: styled value cell
        auto valCell = [](double val, bool isPeak, QColor peakColor) -> QTableWidgetItem*
        {
            auto* item = new QTableWidgetItem(QString::number(val, 'f', 3));
            item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
            if (isPeak)
            {
                item->setForeground(peakColor);
                QFont f = item->font(); f.setBold(true); item->setFont(f);
            }
            return item;
        };

        // Helper lambda: source cell (italic, grey)
        auto srcCell = [&sourceName](int src) -> QTableWidgetItem*
        {
            auto* item = new QTableWidgetItem(sourceName(src));
            item->setTextAlignment(Qt::AlignCenter);
            item->setForeground(QColor(80, 80, 80));
            QFont f = item->font(); f.setItalic(true); item->setFont(f);
            return item;
        };

        bool maxMPeak = (absMaxM > 1e-9 && std::abs(e.maxM) == absMaxM);
        bool minMPeak = (absMinM > 1e-9 && std::abs(e.minM) == absMinM);
        bool maxVPeak = (absMaxV > 1e-9 && std::abs(e.maxV) == absMaxV);
        bool minVPeak = (absMinV > 1e-9 && std::abs(e.minV) == absMinV);

        envelopeTable->setItem(i, 1, valCell(e.maxM, maxMPeak, QColor(180, 0,   0)));
        envelopeTable->setItem(i, 2, srcCell(e.maxMCombo));

        envelopeTable->setItem(i, 3, valCell(e.minM, minMPeak, QColor(0,   0, 180)));
        envelopeTable->setItem(i, 4, srcCell(e.minMCombo));

        envelopeTable->setItem(i, 5, valCell(e.maxV, maxVPeak, QColor(0, 140,   0)));
        envelopeTable->setItem(i, 6, srcCell(e.maxVCombo));

        envelopeTable->setItem(i, 7, valCell(e.minV, minVPeak, QColor(140, 0, 140)));
        envelopeTable->setItem(i, 8, srcCell(e.minVCombo));
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  Helpers
// ─────────────────────────────────────────────────────────────────────────────
static QPointF localToGlobal(double x, double v, double cosT, double sinT,
                             const QPointF& origin)
{
    return QPointF(origin.x() + x * cosT - v * sinT,
                   origin.y() + x * sinT - v * cosT);
}

static void drawArrow(QGraphicsScene* scene, const QPointF& start,
                      const QPointF& end, const QPen& pen)
{
    scene->addLine(QLineF(start, end), pen);
    QLineF line(end, start); line.setLength(10);
    QLineF left = line; left.setAngle(line.angle() + 30);
    QLineF right = line; right.setAngle(line.angle() - 30);
    QPolygonF head; head << end << left.p2() << right.p2();
    scene->addPolygon(head, pen, QBrush(pen.color()));
}

// ─────────────────────────────────────────────────────────────────────────────
//  Scale helpers
// ─────────────────────────────────────────────────────────────────────────────
double MainWindow::computeDiagramScale() const
{
    const auto& members = engine.getMembers();
    const auto& result  = engine.getActiveResult();
    if (!result.valid) return 1.0;

    double maxValue = 0.0, maxLength = 0.0;
    for (size_t i = 0; i < members.size(); ++i)
    {
        const Member& m = members[i];
        const Node& ni  = engine.getNodeById(m.startNode);
        const Node& nj  = engine.getNodeById(m.endNode);
        double L = std::hypot(nj.x - ni.x, nj.y - ni.y);
        maxLength = std::max(maxLength, L);

        if (diagramMode == DiagramMode::SFD) {
            const auto& f = result.memberForces[i];
            maxValue = std::max(maxValue, std::max(std::abs(f.V1), std::abs(f.V2)));
        } else if (diagramMode == DiagramMode::BMD) {
            maxValue = std::max(maxValue, engine.getMaxMoment(m));
        }
    }
    if (maxValue < 1e-9) return 1.0;
    return (maxLength * 50.0 * 0.25) / maxValue;
}

void MainWindow::computeGlobalMaxMoment()
{
    globalMaxMoment.valid = false;
    const auto& result = engine.getActiveResult();
    if (!result.valid) return;

    const auto& members = engine.getMembers();
    for (size_t i = 0; i < members.size(); ++i)
    {
        const auto& m      = members[i];
        const auto& forces = result.memberForces[i];
        const Node& ni     = engine.getNodeById(m.startNode);
        const Node& nj     = engine.getNodeById(m.endNode);
        double L = std::hypot(nj.x - ni.x, nj.y - ni.y);

        if (!globalMaxMoment.valid || std::abs(forces.M1) > std::abs(globalMaxMoment.value))
            globalMaxMoment = {true, &m, 0.0, forces.M1};
        if (std::abs(forces.M2) > std::abs(globalMaxMoment.value))
            globalMaxMoment = {true, &m, L, forces.M2};

        double xc = engine.getCriticalX(m);
        if (xc >= 0) {
            double Mc = engine.momentAt(m, xc);
            if (std::abs(Mc) > std::abs(globalMaxMoment.value))
                globalMaxMoment = {true, &m, xc, Mc};
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  SFD drawing
// ─────────────────────────────────────────────────────────────────────────────
void MainWindow::drawSFD(const Member& m, const Node& ni, const Node& nj, double scale)
{
    QPointF pi(ni.x*50, ni.y*50), pj(nj.x*50, nj.y*50);
    QPointF dir = pj - pi;
    double L = std::hypot(dir.x(), dir.y());
    if (L < 1e-6) return;
    dir /= L;
    QPointF normal(-dir.y(), dir.x());

    QPolygonF poly; poly << pi;
    for (int i = 0; i <= 70; ++i) {
        double x = L * i / 70;
        poly << (pi + dir * x + normal * (engine.shearAt(m, x) * scale));
    }
    poly << pj;
    QPen pen(Qt::blue); pen.setWidth(2);
    scene->addPolygon(poly, pen, QBrush(QColor(0, 0, 255, 100)));
    scene->addLine(QLineF(pi, pj), QPen(Qt::black, 1));
}

void MainWindow::addSFDLabels(size_t index, const QPointF& pi, const QPointF& pj,
                              const QPointF& normal, double scale)
{
    const auto& result = engine.getActiveResult();
    if (!result.valid) return;
    const auto& f = result.memberForces[index];
    auto* t1 = scene->addText(QString::number(f.V1, 'f', 2));
    auto* t2 = scene->addText(QString::number(f.V2, 'f', 2));
    t1->setDefaultTextColor(Qt::blue); t2->setDefaultTextColor(Qt::blue);
    t1->setZValue(20); t2->setZValue(20);
    t1->setPos(pi + normal * (f.V1 * scale) + normal * 5);
    t2->setPos(pj + normal * (f.V2 * scale) + normal * 5);
}

// ─────────────────────────────────────────────────────────────────────────────
//  BMD drawing
// ─────────────────────────────────────────────────────────────────────────────
void MainWindow::drawBMD(const Member& m, const Node& ni, const Node& nj, double scale)
{
    QPointF pi(ni.x*50, ni.y*50), pj(nj.x*50, nj.y*50);
    QPointF dir = pj - pi;
    double L = std::hypot(dir.x(), dir.y());
    if (L < 1e-6) return;
    dir /= L;
    QPointF normal(-dir.y(), dir.x());

    QPolygonF poly; poly << pi;
    for (int i = 0; i <= 70; ++i) {
        double x = L * i / 70;
        poly << (pi + dir * x + normal * (engine.momentAt(m, x) * scale));
    }
    poly << pj;
    QPen pen(Qt::darkRed); pen.setWidth(2);
    scene->addPolygon(poly, pen, QBrush(QColor(200, 0, 0, 100)));
    scene->addLine(QLineF(pi, pj), QPen(Qt::black, 1));

    double xc = engine.getCriticalX(m);
    if (xc >= 0.0 && xc <= L) {
        double Mc = engine.momentAt(m, xc);
        QPointF point = pi + dir * xc + normal * (Mc * scale);
        auto* text = scene->addText(QString::number(Mc, 'f', 2));
        text->setDefaultTextColor(Qt::red);
        text->setZValue(20);
        text->setPos(point);
    }
}

void MainWindow::addBMDLabels(size_t index, const QPointF& pi, const QPointF& pj,
                              const QPointF& normal, double scale)
{
    const auto& result = engine.getActiveResult();
    if (!result.valid) return;
    const auto& f = result.memberForces[index];
    auto* t1 = scene->addText(QString::number(f.M1, 'f', 2));
    auto* t2 = scene->addText(QString::number(f.M2, 'f', 2));
    t1->setDefaultTextColor(Qt::darkRed); t2->setDefaultTextColor(Qt::darkRed);
    t1->setZValue(20); t2->setZValue(20);
    t1->setPos(pi + normal * (f.M1 * scale) + normal * 5);
    t2->setPos(pj + normal * (f.M2 * scale) + normal * 5);
}

// ─────────────────────────────────────────────────────────────────────────────
//  Envelope Moment drawing
// ─────────────────────────────────────────────────────────────────────────────
void MainWindow::drawEnvelopeMoment(const Member& m, const Node& ni, const Node& nj,
                                    int memberIndex, double scale)
{
    QPointF pi(ni.x*50, ni.y*50), pj(nj.x*50, nj.y*50);
    QPointF dir = pj - pi;
    double L = std::hypot(dir.x(), dir.y());
    if (L < 1e-6) return;
    dir /= L;
    QPointF normal(-dir.y(), dir.x());

    const auto& curves = engine.getEnvelopeCurves();
    if (memberIndex >= (int)curves.size()) return;
    const EnvelopeCurve& curve = curves[memberIndex];
    int samples = (int)curve.x.size();
    if (samples < 2) return;

    QPainterPath posPath, negPath;
    for (int i = 0; i < samples; ++i)
    {
        double xPix = curve.x[i] * L;
        QPointF base = pi + dir * xPix;
        QPointF ptMax = base + normal * (curve.maxMoment[i] * scale);
        QPointF ptMin = base + normal * (curve.minMoment[i] * scale);
        if (i == 0) { posPath.moveTo(ptMax); negPath.moveTo(ptMin); }
        else        { posPath.lineTo(ptMax); negPath.lineTo(ptMin); }
    }
    posPath.lineTo(pj); posPath.lineTo(pi); posPath.closeSubpath();
    negPath.lineTo(pj); negPath.lineTo(pi); negPath.closeSubpath();

    scene->addLine(QLineF(pi, pj), QPen(Qt::black, 2));
    QPen noPen(Qt::NoPen);
    scene->addPath(posPath, noPen, QBrush(QColor(200, 0,   0, 120)));
    scene->addPath(negPath, noPen, QBrush(QColor(0,   0, 200, 120)));
    scene->addPath(posPath, QPen(QColor(180, 0,   0), 1), Qt::NoBrush);
    scene->addPath(negPath, QPen(QColor(0,   0, 180), 1), Qt::NoBrush);
}

// ─────────────────────────────────────────────────────────────────────────────
//  Envelope Shear drawing
// ─────────────────────────────────────────────────────────────────────────────
void MainWindow::drawEnvelopeShear(const Member& m, const Node& ni, const Node& nj,
                                   int memberIndex, double scale)
{
    QPointF pi(ni.x*50, ni.y*50), pj(nj.x*50, nj.y*50);
    QPointF dir = pj - pi;
    double L = std::hypot(dir.x(), dir.y());
    if (L < 1e-6) return;
    dir /= L;
    QPointF normal(-dir.y(), dir.x());

    const auto& curves = engine.getEnvelopeCurves();
    if (memberIndex >= (int)curves.size()) return;
    const EnvelopeCurve& curve = curves[memberIndex];
    int samples = (int)curve.x.size();
    if (samples < 2) return;

    QPainterPath posPath, negPath;
    for (int i = 0; i < samples; ++i)
    {
        double xPix = curve.x[i] * L;
        QPointF base = pi + dir * xPix;
        QPointF ptMax = base + normal * (curve.maxShear[i] * scale);
        QPointF ptMin = base + normal * (curve.minShear[i] * scale);
        if (i == 0) { posPath.moveTo(ptMax); negPath.moveTo(ptMin); }
        else        { posPath.lineTo(ptMax); negPath.lineTo(ptMin); }
    }
    posPath.lineTo(pj); posPath.lineTo(pi); posPath.closeSubpath();
    negPath.lineTo(pj); negPath.lineTo(pi); negPath.closeSubpath();

    scene->addLine(QLineF(pi, pj), QPen(Qt::black, 2));
    QPen noPen(Qt::NoPen);
    scene->addPath(posPath, noPen, QBrush(QColor(0,   160, 0,   120)));
    scene->addPath(negPath, noPen, QBrush(QColor(180, 0,   180, 120)));
    scene->addPath(posPath, QPen(QColor(0,   140, 0),   1), Qt::NoBrush);
    scene->addPath(negPath, QPen(QColor(160, 0,   160), 1), Qt::NoBrush);
}

// ─────────────────────────────────────────────────────────────────────────────
//  redrawScene
// ─────────────────────────────────────────────────────────────────────────────
void MainWindow::redrawScene()
{
    scene->clear();

    const int gridSize = 50, gridCount = 20;
    QPen gridPen(QColor(220, 220, 220));
    for (int i = -gridCount; i <= gridCount; ++i) {
        scene->addLine(i*gridSize, -gridCount*gridSize, i*gridSize,  gridCount*gridSize, gridPen);
        scene->addLine(-gridCount*gridSize, i*gridSize, gridCount*gridSize, i*gridSize, gridPen);
    }

    if (diagramMode == DiagramMode::EnvelopeMoment ||
        diagramMode == DiagramMode::EnvelopeShear)
    {
        if (!engine.isEnvelopeComputed()) engine.computeEnvelope();
    }

    double diagramScale   = computeDiagramScale();
    double envelopeMaxM   = engine.getEnvelopeScaleMoment();
    double envelopeMaxV   = engine.getEnvelopeScaleShear();
    double envelopeScaleM = (envelopeMaxM > 1e-9) ? (80.0 / envelopeMaxM) : 1.0;
    double envelopeScaleV = (envelopeMaxV > 1e-9) ? (80.0 / envelopeMaxV) : 1.0;

    if (deformViewMode == DeformViewMode::Undeformed ||
        deformViewMode == DeformViewMode::Both)
    {
        const auto& members = engine.getMembers();
        for (int mIdx = 0; mIdx < (int)members.size(); ++mIdx)
        {
            const Member& member = members[mIdx];
            const Node& ni = engine.getNodeById(member.startNode);
            const Node& nj = engine.getNodeById(member.endNode);

            switch (diagramMode)
            {
            case DiagramMode::None:
                scene->addLine(ni.x*50, ni.y*50, nj.x*50, nj.y*50, QPen(Qt::black, 2));
                break;
            case DiagramMode::SFD:
                drawSFD(member, ni, nj, diagramScale); break;
            case DiagramMode::BMD:
                drawBMD(member, ni, nj, diagramScale); break;
            case DiagramMode::EnvelopeMoment:
                drawEnvelopeMoment(member, ni, nj, mIdx, envelopeScaleM); break;
            case DiagramMode::EnvelopeShear:
                drawEnvelopeShear(member, ni, nj, mIdx, envelopeScaleV); break;
            }
        }

        if (diagramMode == DiagramMode::BMD && globalMaxMoment.valid)
        {
            const Member* mp = globalMaxMoment.member;
            const Node& ni = engine.getNodeById(mp->startNode);
            const Node& nj = engine.getNodeById(mp->endNode);
            QPointF pi(ni.x*50, ni.y*50), pj(nj.x*50, nj.y*50);
            QPointF dir = pj - pi;
            double L = std::hypot(dir.x(), dir.y());
            if (L > 1e-9) {
                dir /= L;
                QPointF normal(-dir.y(), dir.x());
                QPointF point = pi + dir * globalMaxMoment.x
                                + normal * (globalMaxMoment.value * diagramScale);
                scene->addEllipse(point.x()-5, point.y()-5, 10, 10,
                                  QPen(Qt::black), QBrush(Qt::yellow));
                auto* text = scene->addText(
                    "GLOBAL MAX = " + QString::number(globalMaxMoment.value, 'f', 2));
                text->setDefaultTextColor(Qt::darkMagenta);
                text->setZValue(30);
                text->setPos(point + QPointF(12, -12));
            }
        }
    }

    // ── Deformed shape ────────────────────────────────────
    QPen defPen(QColor(30, 144, 255)); defPen.setWidth(2); defPen.setStyle(Qt::DashLine);
    if ((deformViewMode == DeformViewMode::Deformed ||
         deformViewMode == DeformViewMode::Both) &&
        engine.getDisplacements().size() > 0)
    {
        for (const auto& member : engine.getMembers())
        {
            const Node& ni = engine.getNodeById(member.startNode);
            const Node& nj = engine.getNodeById(member.endNode);
            double x1=ni.x*50, y1=ni.y*50, x2=nj.x*50, y2=nj.y*50;
            double dx=x2-x1, dy=y2-y1, L=std::sqrt(dx*dx+dy*dy);
            if (L < 1e-6) continue;
            double cosT=dx/L, sinT=dy/L;
            auto di=engine.getNodeDisplacement(ni), dj=engine.getNodeDisplacement(nj);
            double th1=di.z(), th2=dj.z();
            double v1=(-di.x()*sinT+di.y()*cosT)*deformationScale;
            double v2=(-dj.x()*sinT+dj.y()*cosT)*deformationScale;
            QPainterPath path;
            path.moveTo(localToGlobal(0, v1, cosT, sinT, QPointF(x1,y1)));
            for (int i = 1; i <= 20; ++i) {
                double xi=double(i)/20, x=xi*L;
                double N1=1-3*xi*xi+2*xi*xi*xi, N2=L*(xi-2*xi*xi+xi*xi*xi);
                double N3=3*xi*xi-2*xi*xi*xi,   N4=L*(-xi*xi+xi*xi*xi);
                path.lineTo(localToGlobal(x, N1*v1+N2*th1+N3*v2+N4*th2, cosT, sinT, QPointF(x1,y1)));
            }
            scene->addPath(path, defPen);
        }
    }

    // ── Nodes ─────────────────────────────────────────────
    for (const auto& node : engine.getNodes())
    {
        scene->addEllipse(node.x*50-3, node.y*50-3, 6, 6,
                          QPen(Qt::black),
                          QBrush(node.id == selectedNode ? Qt::red : Qt::blue));
    }

    // ── Supports & reactions ──────────────────────────────
    for (const auto& node : engine.getNodes())
    {
        QPointF p(node.x*50, node.y*50);
        if (node.support == SupportType::Fixed)
            scene->addRect(p.x()-8, p.y()+3, 16, 8, QPen(Qt::black), QBrush(Qt::darkGray));
        else if (node.support == SupportType::Pinned) {
            QPolygonF tri;
            tri << QPointF(p.x()-8,p.y()+3) << QPointF(p.x()+8,p.y()+3) << QPointF(p.x(),p.y()+15);
            scene->addPolygon(tri, QPen(Qt::black), QBrush(Qt::darkGray));
        } else if (node.support == SupportType::Roller) {
            QPolygonF tri;
            tri << QPointF(p.x()-8,p.y()+3) << QPointF(p.x()+8,p.y()+3) << QPointF(p.x(),p.y()+15);
            scene->addPolygon(tri, QPen(Qt::black), QBrush(Qt::gray));
            scene->addLine(p.x()-10,p.y()+15, p.x()+10,p.y()+15, QPen(Qt::black));
        }

        if (node.support != SupportType::None) {
            const auto& result = engine.getActiveResult();
            if (!result.valid) continue;
            int nodeIndex = &node - &engine.getNodes()[0];
            QString text;
            if (nodeIndex < (int)result.Rx.size()) {
                if (std::abs(result.Rx[nodeIndex]) > 1e-6)
                    text += QString("Rx = %1\n").arg(result.Rx[nodeIndex], 0, 'f', 2);
                if (std::abs(result.Ry[nodeIndex]) > 1e-6)
                    text += QString("Ry = %1\n").arg(result.Ry[nodeIndex], 0, 'f', 2);
                if (std::abs(result.Mz[nodeIndex]) > 1e-6)
                    text += QString("Mz = %1").arg(result.Mz[nodeIndex], 0, 'f', 2);
            }
            if (!text.isEmpty()) {
                auto* t = scene->addText(text);
                t->setDefaultTextColor(Qt::darkMagenta);
                t->setPos(p.x()+10, p.y()-40);
                t->setZValue(10);
            }
        }
    }

    // ── Load arrows ───────────────────────────────────────
    for (const auto& load : engine.getLoads())
    {
        if (load.type == LoadType::Nodal) {
            const auto& node = engine.getNodeById(load.nodeId);
            QPointF pos(node.x*50, node.y*50);
            if (std::abs(load.fy) > 1e-6) {
                double len = std::clamp(std::abs(load.fy) * LOAD_SCALE, 15.0, 80.0);
                QPointF end = pos + QPointF(0, load.fy > 0 ? -len : len);
                drawArrow(scene, pos, end, QPen(Qt::blue, 2));
            }
        }
    }

    QRectF rect = scene->itemsBoundingRect();
    if (rect.isNull()) rect = QRectF(-500, -500, 1000, 1000);
    scene->setSceneRect(rect.adjusted(-50, -50, 50, 50));
}

// ─────────────────────────────────────────────────────────────────────────────
//  Mouse / interaction
// ─────────────────────────────────────────────────────────────────────────────
void MainWindow::onViewClicked(const QPointF& pos)
{
    int clickedNode = findNodeAtPosition(pos);
    if (clickedNode == -1) {
        QPointF snapped = snapToGrid(pos);
        engine.addNode(snapped.x()/50.0, snapped.y()/50.0);
        selectedNode = -1; redrawScene(); return;
    }
    if (selectedNode == -1) { selectedNode = clickedNode; redrawScene(); }
    else if (selectedNode != clickedNode) {
        engine.addMember(selectedNode, clickedNode);
        selectedNode = -1; updateAnalysis();
    }
}

void MainWindow::onLeftPressed(const QPointF& pos)
{
    int nodeId = findNodeAtPosition(pos);
    if (nodeId != -1) { draggedNodeId = nodeId; isDragging = true; }
}

void MainWindow::onMouseMoved(const QPointF& pos)
{
    if (!isDragging || draggedNodeId == -1) return;
    QPointF snapped = snapToGrid(pos);
    engine.moveNode(draggedNodeId, snapped.x()/50.0, snapped.y()/50.0);
    redrawScene();
}

void MainWindow::onMouseReleased()
{
    draggedNodeId = -1; isDragging = false; updateAnalysis();
}

void MainWindow::handleRightClick(const QPointF& pos)
{
    int nodeId = findNodeAtPosition(pos);
    if (nodeId != -1)
    {
        QMenu menu(this);
        QAction* fixedAct  = menu.addAction("Fixed Support");
        QAction* pinnedAct = menu.addAction("Pinned Support");
        QAction* rollerAct = menu.addAction("Roller Support");
        QAction* removeAct = menu.addAction("Remove Support");
        QAction* loadAct   = menu.addAction("Add Load");
        QAction* clearAct  = menu.addAction("Remove Load");
        QAction* chosen = menu.exec(QCursor::pos());

        if      (chosen == fixedAct)  engine.setSupport(nodeId, SupportType::Fixed);
        else if (chosen == pinnedAct) engine.setSupport(nodeId, SupportType::Pinned);
        else if (chosen == rollerAct) engine.setSupport(nodeId, SupportType::Roller);
        else if (chosen == removeAct) engine.setSupport(nodeId, SupportType::None);
        else if (chosen == loadAct) {
            LoadDialog dlg(this);
            if (dlg.exec() == QDialog::Accepted)
                engine.addNodalLoad(nodeId, dlg.fx(), dlg.fy(), dlg.mz());
        }
        else if (chosen == clearAct)
            engine.removeLoadsAtNode(nodeId);

        updateAnalysis(); return;
    }

    int memberIndex = findMemberAtPosition(pos);
    if (memberIndex != -1) { engine.removeMember(memberIndex); updateAnalysis(); }
}

int MainWindow::findNodeAtPosition(const QPointF& scenePos)
{
    for (const auto& node : engine.getNodes()) {
        if (QLineF(scenePos, QPointF(node.x*50, node.y*50)).length() <= 8.0)
            return node.id;
    }
    return -1;
}

int MainWindow::findMemberAtPosition(const QPointF& pos)
{
    const double tol = 6.0;
    const auto& members = engine.getMembers();
    for (int i = 0; i < (int)members.size(); ++i)
    {
        const auto& m  = members[i];
        const auto& n1 = engine.getNodeById(m.startNode);
        const auto& n2 = engine.getNodeById(m.endNode);
        QPointF a(n1.x*50, n1.y*50), b(n2.x*50, n2.y*50);
        QPointF ap = pos - a, ab = b - a;
        double dot = QPointF::dotProduct(ap, ab);
        double len2 = QPointF::dotProduct(ab, ab);
        if (len2 == 0) continue;
        double t = std::clamp(dot/len2, 0.0, 1.0);
        if (QLineF(pos, a + t * ab).length() <= tol) return i;
    }
    return -1;
}

QPointF MainWindow::snapToGrid(const QPointF& scenePos) const
{
    constexpr double grid = 50.0;
    return QPointF(std::round(scenePos.x()/grid)*grid,
                   std::round(scenePos.y()/grid)*grid);
}

// ─────────────────────────────────────────────────────────────────────────────
//  Load panel
// ─────────────────────────────────────────────────────────────────────────────
void MainWindow::addNodalLoad()
{
    bool ok;
    int    n  = QInputDialog::getInt(this,    "Node", "Node ID:",         0, 0, 1000, 1,    &ok); if (!ok) return;
    double fx = QInputDialog::getDouble(this, "Fx",   "Horizontal Load:", 0,-10000,10000, 2, &ok); if (!ok) return;
    double fy = QInputDialog::getDouble(this, "Fy",   "Vertical Load:",   0,-10000,10000, 2, &ok); if (!ok) return;
    double mz = QInputDialog::getDouble(this, "Mz",   "Moment:",          0,-10000,10000, 2, &ok); if (!ok) return;
    engine.addNodalLoad(n, fx, fy, mz);
    refreshLoadList(); updateAnalysis();
}

void MainWindow::addUDL()
{
    bool ok;
    int    id = QInputDialog::getInt(this,    "Member", "Member ID:", 0, 0, 1000,   1, &ok); if (!ok) return;
    double w  = QInputDialog::getDouble(this, "UDL",    "Intensity:", 0,-10000,10000, 2, &ok); if (!ok) return;
    engine.addUDL(id, w);
    refreshLoadList(); updateAnalysis();
}

void MainWindow::addPartialUDL()
{
    bool ok;
    int    id    = QInputDialog::getInt(this,    "Member",    "Member ID:",     0, 0,   1000, 1, &ok); if (!ok) return;
    double w     = QInputDialog::getDouble(this, "Intensity", "w:",             0,-10000,10000,2,&ok); if (!ok) return;
    double start = QInputDialog::getDouble(this, "Start",     "Start Position:",0, 0,   1000, 2, &ok); if (!ok) return;
    double end   = QInputDialog::getDouble(this, "End",       "End Position:",  0, 0,   1000, 2, &ok); if (!ok) return;
    engine.addPartialUDL(id, w, start, end);
    refreshLoadList(); updateAnalysis();
}

void MainWindow::deleteLoad()
{
    int row = loadList->currentRow();
    if (row < 0) return;
    engine.removeLoadAtIndex(row);
    refreshLoadList(); updateAnalysis();
}

void MainWindow::refreshLoadList()
{
    loadList->clear();
    for (const auto& L : engine.getLoads())
    {
        QString text;
        if (L.type == LoadType::Nodal)
            text = QString("Node %1 Fx=%2 Fy=%3 Mz=%4")
                       .arg(L.nodeId).arg(L.fx).arg(L.fy).arg(L.mz);
        else if (L.type == LoadType::UDL)
            text = QString("Member %1 UDL=%2").arg(L.memberId).arg(L.w);
        else if (L.type == LoadType::PartialUDL)
            text = QString("Member %1 Partial %2–%3 w=%4")
                       .arg(L.memberId).arg(L.startPos).arg(L.endPos).arg(L.w);
        loadList->addItem(text);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  File I/O
// ─────────────────────────────────────────────────────────────────────────────
void MainWindow::saveModel()
{
    QString fn = QFileDialog::getSaveFileName(this, "Save Model", "", "JSON Files (*.json)");
    if (fn.isEmpty()) return;
    QFile file(fn);
    if (!file.open(QIODevice::WriteOnly)) return;
    file.write(QJsonDocument(engine.toJson()).toJson());
}

void MainWindow::loadModel()
{
    QString fn = QFileDialog::getOpenFileName(this, "Load Model", "", "JSON Files (*.json)");
    if (fn.isEmpty()) return;
    QFile file(fn);
    if (!file.open(QIODevice::ReadOnly)) return;
    engine.fromJson(QJsonDocument::fromJson(file.readAll()).object());
    file.close();

    comboLoadCases->clear();
    for (int i = 0; i < engine.getLoadCaseCount(); ++i)
        comboLoadCases->addItem(QString::fromStdString(engine.getLoadCases()[i].name));
    comboLoadCases->setCurrentIndex(engine.getActiveLoadCase());

    refreshLoadList();
    updateAnalysis();
}

// ─────────────────────────────────────────────────────────────────────────────
//  Analysis update — single source of truth
// ─────────────────────────────────────────────────────────────────────────────
void MainWindow::updateAnalysis()
{
    engine.solve();
    computeGlobalMaxMoment();
    updateMemberTable();
    updateEnvelopeTable();   // ← always refresh both tables
    redrawScene();
}

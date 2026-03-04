#pragma once
#include <vector>
#include <string>
#include "model/Node.h"
#include "model/Member.h"
#include "model/Load.h"
#include <Eigen/Dense>
#include <QJsonObject>

struct EnvelopeCurve
{
    std::vector<double> x;          // normalized 0..1
    std::vector<double> maxMoment;
    std::vector<double> minMoment;
    std::vector<double> maxShear;
    std::vector<double> minShear;
};

// ── Per-member governing envelope values with source combo index ──────────────
struct EnvelopeResult
{
    double maxM      =  0.0;   int maxMCombo = -1;
    double minM      =  0.0;   int minMCombo = -1;
    double maxV      =  0.0;   int maxVCombo = -1;
    double minV      =  0.0;   int minVCombo = -1;
};

struct AnalysisResult
{
    bool valid = false;

    Eigen::VectorXd displacements;   // reduced DOF

    std::vector<double> Rx;
    std::vector<double> Ry;
    std::vector<double> Mz;

    struct MemberForce
    {
        double N1, V1, M1;
        double N2, V2, M2;
    };

    std::vector<MemberForce> memberForces;
};

struct LoadCase
{
    std::string       name;
    std::vector<Load> loads;
    AnalysisResult    result;
    bool              isSolved = false;
};

struct LoadCombination
{
    std::string                        name;
    std::vector<std::pair<int,double>> factors;
    AnalysisResult                     result;
    bool                               isComputed = false;
};

class Engine
{
public:
    Engine();

    // ── Model management ─────────────────────────────────
    void addNode(double x, double y, double z = 0.0);
    void addMember(int startNodeId, int endNodeId);
    void removeNode(int nodeId);
    void removeMember(int memberIndex);
    void moveNode(int nodeId, double x, double y);

    void setSupport(int nodeId, SupportType type);
    void applySupportConditions();
    SupportType getSupport(int nodeId) const;
    bool hasAnySupports() const;
    bool isStable() const;

    // ── Loads ────────────────────────────────────────────
    void addNodalLoad(int nodeId, double fx, double fy, double mz = 0.0);
    void removeLoadsAtNode(int nodeId);
    void addUDL(int memberId, double w);
    void addPartialUDL(int memberId, double w, double start, double end);
    void removeLoadAtIndex(int index);
    const std::vector<Load>& getLoads() const;

    // ── Accessors ────────────────────────────────────────
    const std::vector<Node>&   getNodes()   const;
    const std::vector<Member>& getMembers() const;
    const Node& getNodeById(int id) const;

    const Eigen::VectorXd& getDisplacements() const;
    void setDisplacements(const Eigen::VectorXd& U);
    Eigen::Vector3d getNodeDisplacement(const Node& n) const;

    // ── Diagram helpers ──────────────────────────────────
    double shearAt(const Member& m, double x) const;
    double momentAt(const Member& m, double x) const;
    double getCriticalX(const Member& m) const;
    double getMaxMoment(const Member& m) const;

    // ── Serialisation ────────────────────────────────────
    QJsonObject toJson() const;
    void fromJson(const QJsonObject& obj);
    void clear();

    // ── Solver ───────────────────────────────────────────
    void solve();

    // ── Load cases ───────────────────────────────────────
    void addLoadCase(const std::string& name);
    void setActiveLoadCase(int index);
    int  getActiveLoadCase() const;
    int  getLoadCaseCount()  const;
    const std::vector<LoadCase>& getLoadCases() const;

    // ── Load combinations ────────────────────────────────
    void addLoadCombination(const std::string& name);
    void addCaseToCombination(int comboIndex, int caseIndex, double factor);
    void computeLoadCombination(int comboIndex);
    void setActiveLoadCombination(int index);
    int  getLoadCombinationCount() const;
    const std::vector<LoadCombination>& getLoadCombinations() const;
    void applyResultToModel(const AnalysisResult& result);

    // ── Results ──────────────────────────────────────────
    const AnalysisResult& getActiveResult() const;

    // ── Envelope (curves for drawing) ────────────────────
    void computeEnvelope();
    bool isEnvelopeComputed() const;
    const std::vector<EnvelopeCurve>& getEnvelopeCurves() const;
    double getEnvelopeScaleMoment() const;
    double getEnvelopeScaleShear()  const;

    // ── Envelope (table — governing values + combo IDs) ──
    std::vector<EnvelopeResult> computeEnvelopeTable() const;

private:
    std::vector<Node>   nodes;
    std::vector<Member> members;

    int nextNodeId   = 1;
    int nextMemberId = 1;

    std::vector<LoadCase>       loadCases;
    int                         activeLoadCase = 0;

    std::vector<LoadCombination> loadCombinations;
    int                          activeLoadCombination = -1;

    enum class ResultMode { LoadCase, LoadCombination };
    ResultMode resultMode = ResultMode::LoadCase;

    std::vector<AnalysisResult> results;

    std::vector<EnvelopeCurve> envelopeCurves;
    bool envelopeComputed = false;

    void evaluateEnvelopeForResult(const AnalysisResult& result);
};

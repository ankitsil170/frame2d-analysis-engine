#include "engine.h"
#include "solver/frame_solver.h"
#include <algorithm>
#include <QJsonArray>
#include <QJsonObject>

Engine::Engine()
{
    LoadCase defaultCase;
    defaultCase.name = "Load Case 1";
    loadCases.push_back(defaultCase);
}

void Engine::clear()
{
    nodes.clear();
    members.clear();
    loadCases.clear();

    LoadCase defaultCase;
    defaultCase.name = "Load Case 1";
    loadCases.push_back(defaultCase);

    activeLoadCase = 0;

    nextNodeId = 1;
    nextMemberId = 1;
}


QJsonObject Engine::toJson() const
{
    QJsonObject root;

    // ---- Nodes ----
    QJsonArray nodeArray;
    for (const auto& n : nodes)
    {
        QJsonObject obj;
        obj["id"] = n.id;
        obj["x"] = n.x;
        obj["y"] = n.y;
        obj["z"] = n.z;
        obj["support"] = static_cast<int>(n.support);
        nodeArray.append(obj);
    }
    root["nodes"] = nodeArray;

    // ---- Members ----
    QJsonArray memberArray;
    for (const auto& m : members)
    {
        QJsonObject obj;
        obj["id"] = m.id;
        obj["start"] = m.startNode;
        obj["end"] = m.endNode;
        obj["E"] = m.E;
        obj["A"] = m.A;
        obj["I"] = m.I;
        memberArray.append(obj);
    }
    root["members"] = memberArray;

    // ---- Load Cases ----
    QJsonArray caseArray;

    for (const auto& lc : loadCases)
    {
        QJsonObject caseObj;
        caseObj["name"] = QString::fromStdString(lc.name);

        QJsonArray loadArray;

        for (const auto& L : lc.loads)
        {
            QJsonObject obj;
            obj["type"]     = static_cast<int>(L.type);
            obj["nodeId"]   = L.nodeId;
            obj["memberId"] = L.memberId;
            obj["fx"]       = L.fx;
            obj["fy"]       = L.fy;
            obj["mz"]       = L.mz;
            obj["w"]        = L.w;
            obj["startPos"] = L.startPos;
            obj["endPos"]   = L.endPos;

            loadArray.append(obj);
        }

        caseObj["loads"] = loadArray;
        caseArray.append(caseObj);
    }

    root["loadCases"]      = caseArray;
    root["activeLoadCase"] = activeLoadCase;

    return root;
}

void Engine::fromJson(const QJsonObject& root)
{
    clear();

    // ---- Nodes ----
    QJsonArray nodeArray = root["nodes"].toArray();
    for (const auto& val : nodeArray)
    {
        QJsonObject obj = val.toObject();

        int    id = obj["id"].toInt();
        double x  = obj["x"].toDouble();
        double y  = obj["y"].toDouble();
        double z  = obj["z"].toDouble();

        Node n(id, x, y, z);
        n.support = static_cast<SupportType>(obj["support"].toInt());

        nodes.push_back(n);

        if (id >= nextNodeId)
            nextNodeId = id + 1;
    }

    // ---- Members ----
    QJsonArray memberArray = root["members"].toArray();
    for (const auto& val : memberArray)
    {
        QJsonObject obj = val.toObject();

        int    id    = obj["id"].toInt();
        int    start = obj["start"].toInt();
        int    end   = obj["end"].toInt();
        double E     = obj["E"].toDouble();
        double A     = obj["A"].toDouble();
        double I     = obj["I"].toDouble();

        Member m(id, start, end, E, A, I);
        members.push_back(m);

        if (id >= nextMemberId)
            nextMemberId = id + 1;
    }

    // ---- Load Cases ----
    loadCases.clear();

    QJsonArray caseArray = root["loadCases"].toArray();

    for (const auto& caseVal : caseArray)
    {
        QJsonObject caseObj = caseVal.toObject();

        LoadCase lc;
        lc.name = caseObj["name"].toString().toStdString();

        QJsonArray loadArray = caseObj["loads"].toArray();

        for (const auto& val : loadArray)
        {
            QJsonObject obj = val.toObject();

            Load L;
            L.type     = static_cast<LoadType>(obj["type"].toInt());
            L.nodeId   = obj["nodeId"].toInt();
            L.memberId = obj["memberId"].toInt();
            L.fx       = obj["fx"].toDouble();
            L.fy       = obj["fy"].toDouble();
            L.mz       = obj["mz"].toDouble();
            L.w        = obj["w"].toDouble();
            L.startPos = obj["startPos"].toDouble();
            L.endPos   = obj["endPos"].toDouble();

            lc.loads.push_back(L);
        }

        loadCases.push_back(lc);
    }

    activeLoadCase = root["activeLoadCase"].toInt();
}


/*-------------------------------------------------------
  BASIC MODEL MANAGEMENT
-------------------------------------------------------*/
const AnalysisResult& Engine::getActiveResult() const
{
    if (resultMode == ResultMode::LoadCombination &&
        activeLoadCombination >= 0)
    {
        return loadCombinations[activeLoadCombination].result;
    }

    return loadCases[activeLoadCase].result;
}

const Eigen::VectorXd& Engine::getDisplacements() const
{
    if (resultMode == ResultMode::LoadCombination && activeLoadCombination >= 0)
        return loadCombinations[activeLoadCombination].result.displacements;

    return loadCases[activeLoadCase].result.displacements;
}

Eigen::Vector3d Engine::getNodeDisplacement(const Node& n) const
{
    const auto& result       = getActiveResult();
    const auto& displacements = result.displacements;

    double ux = 0.0;
    double uy = 0.0;
    double rz = 0.0;

    if (!n.fixUx && n.dofUx >= 0 && n.dofUx < displacements.size())
        ux = displacements(n.dofUx);

    if (!n.fixUy && n.dofUy >= 0 && n.dofUy < displacements.size())
        uy = displacements(n.dofUy);

    if (!n.fixRz && n.dofRz >= 0 && n.dofRz < displacements.size())
        rz = displacements(n.dofRz);

    return Eigen::Vector3d(ux, uy, rz);
}

void Engine::addNode(double x, double y, double z)
{
    nodes.emplace_back(nextNodeId++, x, y, z);
    envelopeComputed = false;
}

void Engine::addMember(int startNodeId, int endNodeId)
{
    double E = 2.0e11;
    double A = 0.01;
    double I = 8.333e-6;

    members.emplace_back(nextMemberId++, startNodeId, endNodeId, E, A, I);
}

const std::vector<Node>&   Engine::getNodes()   const { return nodes; }
const std::vector<Member>& Engine::getMembers() const { return members; }

void Engine::removeNode(int nodeId)
{
    members.erase(
        std::remove_if(members.begin(), members.end(),
                       [nodeId](const Member& m) {
                           return m.startNode == nodeId || m.endNode == nodeId;
                       }),
        members.end());

    nodes.erase(
        std::remove_if(nodes.begin(), nodes.end(),
                       [nodeId](const Node& n) {
                           return n.id == nodeId;
                       }),
        nodes.end());

    envelopeComputed = false;
}

void Engine::removeMember(int memberIndex)
{
    if (memberIndex < 0 || memberIndex >= (int)members.size())
        return;

    members.erase(members.begin() + memberIndex);

    envelopeComputed = false;
}

void Engine::moveNode(int nodeId, double x, double y)
{
    for (auto& n : nodes) {
        if (n.id == nodeId) {
            n.x = x;
            n.y = y;
            return;
        }
    }
}

const Node& Engine::getNodeById(int id) const
{
    for (const auto& n : nodes)
    {
        if (n.id == id)
            return n;
    }

    throw std::runtime_error("Node ID not found");
}

/*-------------------------------------------------------
  SUPPORTS
-------------------------------------------------------*/
void Engine::setSupport(int nodeId, SupportType type)
{
    for (auto& n : nodes) {
        if (n.id == nodeId) {
            n.support = type;
            return;
        }
    }
}

void Engine::applySupportConditions()
{
    for (auto& n : nodes)
    {
        n.fixUx = false;
        n.fixUy = false;
        n.fixRz = false;

        switch (n.support)
        {
        case SupportType::Fixed:
            n.fixUx = true;
            n.fixUy = true;
            n.fixRz = true;
            break;

        case SupportType::Pinned:
            n.fixUx = true;
            n.fixUy = true;
            break;

        case SupportType::Roller:
            n.fixUy = true;
            break;

        case SupportType::None:
        default:
            break;
        }
    }
}

SupportType Engine::getSupport(int nodeId) const
{
    for (const auto& n : nodes)
        if (n.id == nodeId) return n.support;

    return SupportType::None;
}

bool Engine::hasAnySupports() const
{
    for (const auto& n : nodes)
        if (n.support != SupportType::None)
            return true;
    return false;
}

/*-------------------------------------------------------
  LOADS
-------------------------------------------------------*/
void Engine::addNodalLoad(int nodeId, double fx, double fy, double mz)
{
    Load L;
    L.type   = LoadType::Nodal;
    L.nodeId = nodeId;
    L.fx     = fx;
    L.fy     = fy;
    L.mz     = mz;

    loadCases[activeLoadCase].loads.push_back(L);
    envelopeComputed = false;
}

void Engine::removeLoadAtIndex(int index)
{
    auto& loads = loadCases[activeLoadCase].loads;

    if (index >= 0 && index < static_cast<int>(loads.size()))
        loads.erase(loads.begin() + index);
}

void Engine::removeLoadsAtNode(int nodeId)
{
    auto& loads = loadCases[activeLoadCase].loads;

    loads.erase(
        std::remove_if(loads.begin(), loads.end(),
                       [nodeId](const Load& l) {
                           return l.nodeId == nodeId;
                       }),
        loads.end());
}

/*-------------------------------------------------------
  UDL
-------------------------------------------------------*/
void Engine::addUDL(int memberId, double w)
{
    Load L;
    L.type     = LoadType::UDL;
    L.memberId = memberId;
    L.w        = w;

    loadCases[activeLoadCase].loads.push_back(L);
    envelopeComputed = false;
}

void Engine::addPartialUDL(int memberId, double w, double start, double end)
{
    Load L;
    L.type     = LoadType::PartialUDL;
    L.memberId = memberId;
    L.w        = w;
    L.startPos = start;
    L.endPos   = end;

    loadCases[activeLoadCase].loads.push_back(L);
    envelopeComputed = false;
}

/*-------------------------------------------------------
  SHEAR & MOMENT (with full UDL contribution)
-------------------------------------------------------*/
double Engine::shearAt(const Member& m, double x) const
{
    const auto& result = getActiveResult();

    int memberIndex = &m - &members[0];

    double V = result.memberForces[memberIndex].V1;

    const Node& ni = getNodeById(m.startNode);
    const Node& nj = getNodeById(m.endNode);
    double L = std::hypot(nj.x - ni.x, nj.y - ni.y);

    bool isCombo = (resultMode == ResultMode::LoadCombination &&
                    activeLoadCombination >= 0);

    if (!isCombo)
    {
        for (const auto& load : loadCases[activeLoadCase].loads)
        {
            if (load.memberId != m.id) continue;

            if (load.type == LoadType::UDL ||
                load.type == LoadType::PartialUDL)
            {
                double a = 0.0;
                double b = L;

                if (load.type == LoadType::PartialUDL)
                {
                    a = load.startPos;
                    b = load.endPos;
                }

                if (x > a)
                {
                    double effective = std::min(x, b) - a;
                    if (effective > 0)
                        V -= load.w * effective;
                }
            }
        }
    }
    else
    {
        const auto& combo = loadCombinations[activeLoadCombination];

        for (const auto& pair : combo.factors)
        {
            int    caseIndex = pair.first;
            double factor    = pair.second;

            if (caseIndex < 0 || caseIndex >= (int)loadCases.size())
                continue;

            for (const auto& load : loadCases[caseIndex].loads)
            {
                if (load.memberId != m.id) continue;

                if (load.type == LoadType::UDL ||
                    load.type == LoadType::PartialUDL)
                {
                    double a = 0.0;
                    double b = L;

                    if (load.type == LoadType::PartialUDL)
                    {
                        a = load.startPos;
                        b = load.endPos;
                    }

                    if (x > a)
                    {
                        double effective = std::min(x, b) - a;
                        if (effective > 0)
                            V -= factor * load.w * effective;
                    }
                }
            }
        }
    }

    return V;
}

double Engine::momentAt(const Member& m, double x) const
{
    const auto& result = getActiveResult();

    int memberIndex = &m - &members[0];

    double M = result.memberForces[memberIndex].M1 +
               result.memberForces[memberIndex].V1 * x;

    const Node& ni = getNodeById(m.startNode);
    const Node& nj = getNodeById(m.endNode);
    double L = std::hypot(nj.x - ni.x, nj.y - ni.y);

    bool isCombo = (resultMode == ResultMode::LoadCombination &&
                    activeLoadCombination >= 0);

    if (!isCombo)
    {
        for (const auto& load : loadCases[activeLoadCase].loads)
        {
            if (load.memberId != m.id) continue;

            if (load.type == LoadType::UDL ||
                load.type == LoadType::PartialUDL)
            {
                double a = 0.0;
                double b = L;

                if (load.type == LoadType::PartialUDL)
                {
                    a = load.startPos;
                    b = load.endPos;
                }

                if (x > a)
                {
                    double effective = std::min(x, b) - a;
                    if (effective > 0)
                        M -= load.w * effective * effective / 2.0;
                }
            }
        }
    }
    else
    {
        const auto& combo = loadCombinations[activeLoadCombination];

        for (const auto& pair : combo.factors)
        {
            int    caseIndex = pair.first;
            double factor    = pair.second;

            if (caseIndex < 0 || caseIndex >= (int)loadCases.size())
                continue;

            for (const auto& load : loadCases[caseIndex].loads)
            {
                if (load.memberId != m.id) continue;

                if (load.type == LoadType::UDL ||
                    load.type == LoadType::PartialUDL)
                {
                    double a = 0.0;
                    double b = L;

                    if (load.type == LoadType::PartialUDL)
                    {
                        a = load.startPos;
                        b = load.endPos;
                    }

                    if (x > a)
                    {
                        double effective = std::min(x, b) - a;
                        if (effective > 0)
                            M -= factor * load.w * effective * effective / 2.0;
                    }
                }
            }
        }
    }

    return M;
}

double Engine::getCriticalX(const Member& m) const
{
    const Node& ni = getNodeById(m.startNode);
    const Node& nj = getNodeById(m.endNode);

    double L = std::hypot(nj.x - ni.x, nj.y - ni.y);

    if (resultMode != ResultMode::LoadCase)
        return -1.0;

    const auto& loads  = loadCases[activeLoadCase].loads;
    const auto& result = getActiveResult();
    int memberIndex    = &m - &members[0];

    double V1 = result.memberForces[memberIndex].V1;

    for (const auto& load : loads)
    {
        if (load.memberId != m.id)
            continue;

        if (load.type == LoadType::UDL ||
            load.type == LoadType::PartialUDL)
        {
            if (std::abs(load.w) < 1e-12)
                continue;

            double a = 0.0;
            double b = L;

            if (load.type == LoadType::PartialUDL)
            {
                a = load.startPos;
                b = load.endPos;
            }

            double x = a + V1 / load.w;

            if (x >= a && x <= b)
                return x;
        }
    }

    return -1.0;
}

double Engine::getMaxMoment(const Member& m) const
{
    const auto& result = getActiveResult();
    int memberIndex    = &m - &members[0];

    double M1 = result.memberForces[memberIndex].M1;
    double M2 = result.memberForces[memberIndex].M2;

    double maxM = std::max(std::abs(M1), std::abs(M2));

    const Node& ni = getNodeById(m.startNode);
    const Node& nj = getNodeById(m.endNode);

    double L  = std::hypot(nj.x - ni.x, nj.y - ni.y);
    double xc = getCriticalX(m);

    if (xc >= 0.0 && xc <= L)
    {
        double Mc = momentAt(m, xc);
        maxM = std::max(maxM, std::abs(Mc));
    }

    return maxM;
}

const std::vector<Load>& Engine::getLoads() const
{
    return loadCases[activeLoadCase].loads;
}

/*-------------------------------------------------------
  SOLUTION
-------------------------------------------------------*/
void Engine::solve()
{
    if (activeLoadCase >= (int)loadCases.size())
        return;

    auto& currentCase = loadCases[activeLoadCase];
    auto& loads       = currentCase.loads;

    if (nodes.empty() || members.empty())
        return;

    if (!hasAnySupports())
        return;

    applySupportConditions();

    FrameSolver solver;

    // 0. ASSIGN FULL DOFs
    int fullCounter = 0;
    for (auto& n : nodes)
    {
        n.fullDofUx = fullCounter++;
        n.fullDofUy = fullCounter++;
        n.fullDofRz = fullCounter++;
    }
    int fullDOF = fullCounter;

    // 1. ASSIGN REDUCED DOFs
    int reducedDOF = solver.assignDOFs(nodes);
    if (reducedDOF == 0)
        return;

    // 2. ASSEMBLE REDUCED LOAD VECTOR
    Eigen::VectorXd F = Eigen::VectorXd::Zero(reducedDOF);

    for (const auto& L : loads)
    {
        if (L.type == LoadType::Nodal)
        {
            const Node& n = getNodeById(L.nodeId);

            if (!n.fixUx && n.dofUx >= 0) F(n.dofUx) += L.fx;
            if (!n.fixUy && n.dofUy >= 0) F(n.dofUy) += L.fy;
            if (!n.fixRz && n.dofRz >= 0) F(n.dofRz) += L.mz;
        }
    }

    // ADD MEMBER UDL EFFECTS
    for (const auto& L : loads)
    {
        if (L.type == LoadType::UDL || L.type == LoadType::PartialUDL)
        {
            const Member* memberPtr = nullptr;

            for (const auto& m : members)
                if (m.id == L.memberId)
                    memberPtr = &m;

            if (!memberPtr) continue;

            const Member& m = *memberPtr;

            const Node& ni = getNodeById(m.startNode);
            const Node& nj = getNodeById(m.endNode);

            double length = std::hypot(nj.x - ni.x, nj.y - ni.y);

            double start = 0.0;
            double end   = length;

            if (L.type == LoadType::PartialUDL)
            {
                start = L.startPos;
                end   = L.endPos;
            }

            Eigen::Matrix<double,6,1> feLocal =
                solver.fixedEndForcesPartialUDL(L.w, length, start, end);

            Eigen::Matrix<double,6,6> T =
                solver.transformation(ni.x, ni.y, nj.x, nj.y);

            Eigen::Matrix<double,6,1> feGlobal = T.transpose() * feLocal;

            int dofMap[6] = {
                ni.dofUx, ni.dofUy, ni.dofRz,
                nj.dofUx, nj.dofUy, nj.dofRz
            };

            for (int i = 0; i < 6; ++i)
                if (dofMap[i] >= 0)
                    F(dofMap[i]) -= feGlobal(i);
        }
    }

    // 3. ASSEMBLE REDUCED STIFFNESS
    Eigen::MatrixXd K =
        solver.assembleGlobalStiffness(nodes, members, reducedDOF);

    // 4. SOLVE FOR DISPLACEMENTS
    Eigen::VectorXd U = solver.solveDisplacements(K, F);

    AnalysisResult result;
    result.displacements = U;

    // 5. BUILD FULL DISPLACEMENT VECTOR
    Eigen::VectorXd Ufull = Eigen::VectorXd::Zero(fullDOF);

    for (const auto& n : nodes)
    {
        if (!n.fixUx && n.dofUx >= 0) Ufull(n.fullDofUx) = U(n.dofUx);
        if (!n.fixUy && n.dofUy >= 0) Ufull(n.fullDofUy) = U(n.dofUy);
        if (!n.fixRz && n.dofRz >= 0) Ufull(n.fullDofRz) = U(n.dofRz);
    }

    // 6. BUILD FULL LOAD VECTOR
    Eigen::VectorXd Ffull = Eigen::VectorXd::Zero(fullDOF);

    for (const auto& L : loads)
    {
        if (L.type == LoadType::Nodal)
        {
            const Node& n = getNodeById(L.nodeId);

            Ffull(n.fullDofUx) += L.fx;
            Ffull(n.fullDofUy) += L.fy;
            Ffull(n.fullDofRz) += L.mz;
        }
    }

    // 7. FULL STIFFNESS MATRIX
    Eigen::MatrixXd Kfull = solver.assembleFullStiffness(nodes, members);

    // 8. COMPUTE REACTIONS
    Eigen::VectorXd Rfull = Kfull * Ufull - Ffull;

    result.Rx.resize(nodes.size());
    result.Ry.resize(nodes.size());
    result.Mz.resize(nodes.size());

    for (int i = 0; i < (int)nodes.size(); ++i)
    {
        const auto& n = nodes[i];

        result.Rx[i] = n.fixUx ? Rfull(n.fullDofUx) : 0.0;
        result.Ry[i] = n.fixUy ? Rfull(n.fullDofUy) : 0.0;
        result.Mz[i] = n.fixRz ? Rfull(n.fullDofRz) : 0.0;
    }

    // 9. MEMBER END FORCES (LOCAL)
    result.memberForces.resize(members.size());

    for (int i = 0; i < (int)members.size(); ++i)
    {
        auto& m = members[i];

        const Node& ni = getNodeById(m.startNode);
        const Node& nj = getNodeById(m.endNode);

        Eigen::VectorXd f =
            solver.computeMemberEndForces(m, ni, nj, U);

        AnalysisResult::MemberForce mf;
        mf.N1 = f(0);
        mf.V1 = f(1);
        mf.M1 = f(2);
        mf.N2 = f(3);
        mf.V2 = f(4);
        mf.M2 = f(5);

        result.memberForces[i] = mf;
    }

    currentCase.result   = result;
    currentCase.isSolved = true;

    envelopeComputed = false;
}

/*-------------------------------------------------------
  LOAD CASES
-------------------------------------------------------*/
void Engine::addLoadCase(const std::string& name)
{
    LoadCase lc;
    lc.name = name;
    loadCases.push_back(lc);
}

void Engine::setActiveLoadCase(int index)
{
    if (index >= 0 && index < (int)loadCases.size())
    {
        activeLoadCase = index;
        resultMode     = ResultMode::LoadCase;
    }
}

int Engine::getActiveLoadCase() const { return activeLoadCase; }
int Engine::getLoadCaseCount()  const { return static_cast<int>(loadCases.size()); }

const std::vector<LoadCase>& Engine::getLoadCases() const { return loadCases; }

/*-------------------------------------------------------
  LOAD COMBINATIONS
-------------------------------------------------------*/
void Engine::addLoadCombination(const std::string& name)
{
    LoadCombination combo;
    combo.name = name;
    loadCombinations.push_back(combo);
}

void Engine::addCaseToCombination(int comboIndex, int caseIndex, double factor)
{
    loadCombinations[comboIndex].factors.push_back({ caseIndex, factor });
}

int Engine::getLoadCombinationCount() const
{
    return (int)loadCombinations.size();
}

void Engine::applyResultToModel(const AnalysisResult& result)
{
    for (int i = 0; i < (int)nodes.size(); ++i)
    {
        nodes[i].displayRx = result.Rx[i];
        nodes[i].displayRy = result.Ry[i];
        nodes[i].displayMz = result.Mz[i];
    }

    for (int i = 0; i < (int)members.size(); ++i)
    {
        members[i].displayN1 = result.memberForces[i].N1;
        members[i].displayV1 = result.memberForces[i].V1;
        members[i].displayM1 = result.memberForces[i].M1;
        members[i].displayN2 = result.memberForces[i].N2;
        members[i].displayV2 = result.memberForces[i].V2;
        members[i].displayM2 = result.memberForces[i].M2;
    }
}

void Engine::setActiveLoadCombination(int index)
{
    activeLoadCombination = index;
    resultMode            = ResultMode::LoadCombination;

    if (!loadCombinations[index].isComputed)
        computeLoadCombination(index);

    applyResultToModel(loadCombinations[index].result);
}

void Engine::computeLoadCombination(int comboIndex)
{
    auto& combo = loadCombinations[comboIndex];

    AnalysisResult combined;

    const auto& base = loadCases[combo.factors[0].first].result;

    combined.displacements =
        Eigen::VectorXd::Zero(base.displacements.size());

    combined.Rx = std::vector<double>(nodes.size(), 0.0);
    combined.Ry = std::vector<double>(nodes.size(), 0.0);
    combined.Mz = std::vector<double>(nodes.size(), 0.0);

    combined.memberForces.resize(members.size());

    for (auto& mf : combined.memberForces)
        mf = {0, 0, 0, 0, 0, 0};

    for (auto& pair : combo.factors)
    {
        int    caseIndex = pair.first;
        double factor    = pair.second;

        if (!loadCases[caseIndex].isSolved)
        {
            int oldActive = activeLoadCase;
            setActiveLoadCase(caseIndex);
            solve();
            activeLoadCase = oldActive;
        }

        const auto& r = loadCases[caseIndex].result;

        combined.displacements += factor * r.displacements;

        for (int i = 0; i < (int)nodes.size(); ++i)
        {
            combined.Rx[i] += factor * r.Rx[i];
            combined.Ry[i] += factor * r.Ry[i];
            combined.Mz[i] += factor * r.Mz[i];
        }

        for (int i = 0; i < (int)members.size(); ++i)
        {
            combined.memberForces[i].N1 += factor * r.memberForces[i].N1;
            combined.memberForces[i].V1 += factor * r.memberForces[i].V1;
            combined.memberForces[i].M1 += factor * r.memberForces[i].M1;
            combined.memberForces[i].N2 += factor * r.memberForces[i].N2;
            combined.memberForces[i].V2 += factor * r.memberForces[i].V2;
            combined.memberForces[i].M2 += factor * r.memberForces[i].M2;
        }
    }

    combo.result     = combined;
    combo.isComputed = true;
}

/*-------------------------------------------------------
  ENVELOPE
-------------------------------------------------------*/
void Engine::computeEnvelope()
{
    envelopeCurves.clear();
    envelopeCurves.resize(members.size());

    const int samples = 40;

    for (int mIndex = 0; mIndex < (int)members.size(); ++mIndex)
    {
        EnvelopeCurve& curve = envelopeCurves[mIndex];

        curve.x.resize(samples);
        curve.maxMoment.assign(samples, -1e20);
        curve.minMoment.assign(samples,  1e20);
        curve.maxShear.assign(samples,  -1e20);
        curve.minShear.assign(samples,   1e20);

        const Member& m = members[mIndex];

        const Node& ni = getNodeById(m.startNode);
        const Node& nj = getNodeById(m.endNode);

        double L = std::hypot(nj.x - ni.x, nj.y - ni.y);

        for (int i = 0; i < samples; ++i)
        {
            double x    = L * i / (samples - 1);
            curve.x[i]  = x / L;   // normalised 0..1

            // --- Load cases ---
            for (int lcIndex = 0; lcIndex < (int)loadCases.size(); ++lcIndex)
            {
                if (!loadCases[lcIndex].isSolved)
                    continue;

                int        oldActive = activeLoadCase;
                ResultMode oldMode   = resultMode;

                activeLoadCase = lcIndex;
                resultMode     = ResultMode::LoadCase;

                double V = shearAt(m, x);
                double M = momentAt(m, x);

                curve.maxMoment[i] = std::max(curve.maxMoment[i], M);
                curve.minMoment[i] = std::min(curve.minMoment[i], M);
                curve.maxShear[i]  = std::max(curve.maxShear[i],  V);
                curve.minShear[i]  = std::min(curve.minShear[i],  V);

                activeLoadCase = oldActive;
                resultMode     = oldMode;
            }

            // --- Load combinations ---
            for (int comboIndex = 0; comboIndex < (int)loadCombinations.size(); ++comboIndex)
            {
                if (!loadCombinations[comboIndex].isComputed)
                    continue;

                int        oldCombo = activeLoadCombination;
                ResultMode oldMode  = resultMode;

                activeLoadCombination = comboIndex;
                resultMode            = ResultMode::LoadCombination;

                double V = shearAt(m, x);
                double M = momentAt(m, x);

                curve.maxMoment[i] = std::max(curve.maxMoment[i], M);
                curve.minMoment[i] = std::min(curve.minMoment[i], M);
                curve.maxShear[i]  = std::max(curve.maxShear[i],  V);
                curve.minShear[i]  = std::min(curve.minShear[i],  V);

                activeLoadCombination = oldCombo;
                resultMode            = oldMode;
            }
        }
    }

    envelopeComputed = true;
}

bool Engine::isEnvelopeComputed() const
{
    return envelopeComputed;
}

double Engine::getEnvelopeScaleMoment() const
{
    double maxVal = 0.0;

    for (const auto& curve : envelopeCurves)
    {
        for (double v : curve.maxMoment)
            maxVal = std::max(maxVal, std::abs(v));
        for (double v : curve.minMoment)
            maxVal = std::max(maxVal, std::abs(v));
    }

    return maxVal;
}

double Engine::getEnvelopeScaleShear() const
{
    double maxVal = 0.0;

    for (const auto& curve : envelopeCurves)
    {
        for (double v : curve.maxShear)
            maxVal = std::max(maxVal, std::abs(v));
        for (double v : curve.minShear)
            maxVal = std::max(maxVal, std::abs(v));
    }

    return maxVal;
}

const std::vector<EnvelopeCurve>& Engine::getEnvelopeCurves() const
{
    return envelopeCurves;
}
// ─────────────────────────────────────────────────────────────────────────────
//  computeEnvelopeTable  — governing values per member with source combo index
//  Call after all combinations have been computed.
// ─────────────────────────────────────────────────────────────────────────────
std::vector<EnvelopeResult> Engine::computeEnvelopeTable() const
{
    std::vector<EnvelopeResult> envelope(members.size());

    // ── Load Cases ─────────────────────────────────────────────────────────
    for (int lcIndex = 0; lcIndex < (int)loadCases.size(); ++lcIndex)
    {
        const auto& lc = loadCases[lcIndex];
        if (!lc.isSolved) continue;

        for (int m = 0; m < (int)members.size(); ++m)
        {
            const auto& mf = lc.result.memberForces[m];

            double maxM = std::max(mf.M1, mf.M2);
            double minM = std::min(mf.M1, mf.M2);
            double maxV = std::max(mf.V1, mf.V2);
            double minV = std::min(mf.V1, mf.V2);

            // Use negative combo index to encode load-case source:
            // combo index = -(lcIndex + 1)  → displayed as "LC <n>"
            int src = -(lcIndex + 1);

            if (lcIndex == 0 && loadCombinations.empty())
            {
                // First and only source — just assign
                envelope[m].maxM = maxM; envelope[m].maxMCombo = src;
                envelope[m].minM = minM; envelope[m].minMCombo = src;
                envelope[m].maxV = maxV; envelope[m].maxVCombo = src;
                envelope[m].minV = minV; envelope[m].minVCombo = src;
            }
            else
            {
                if (maxM > envelope[m].maxM) { envelope[m].maxM = maxM; envelope[m].maxMCombo = src; }
                if (minM < envelope[m].minM) { envelope[m].minM = minM; envelope[m].minMCombo = src; }
                if (maxV > envelope[m].maxV) { envelope[m].maxV = maxV; envelope[m].maxVCombo = src; }
                if (minV < envelope[m].minV) { envelope[m].minV = minV; envelope[m].minVCombo = src; }
            }
        }
    }

    // ── Load Combinations ──────────────────────────────────────────────────
    for (int c = 0; c < (int)loadCombinations.size(); ++c)
    {
        const auto& combo = loadCombinations[c];
        if (!combo.isComputed) continue;

        for (int m = 0; m < (int)members.size(); ++m)
        {
            const auto& mf = combo.result.memberForces[m];

            double maxM = std::max(mf.M1, mf.M2);
            double minM = std::min(mf.M1, mf.M2);
            double maxV = std::max(mf.V1, mf.V2);
            double minV = std::min(mf.V1, mf.V2);

            bool isFirst = (c == 0 &&
                            std::none_of(loadCases.begin(), loadCases.end(),
                                         [](const LoadCase& lc){ return lc.isSolved; }));

            if (isFirst)
            {
                envelope[m].maxM = maxM; envelope[m].maxMCombo = c;
                envelope[m].minM = minM; envelope[m].minMCombo = c;
                envelope[m].maxV = maxV; envelope[m].maxVCombo = c;
                envelope[m].minV = minV; envelope[m].minVCombo = c;
            }
            else
            {
                if (maxM > envelope[m].maxM) { envelope[m].maxM = maxM; envelope[m].maxMCombo = c; }
                if (minM < envelope[m].minM) { envelope[m].minM = minM; envelope[m].minMCombo = c; }
                if (maxV > envelope[m].maxV) { envelope[m].maxV = maxV; envelope[m].maxVCombo = c; }
                if (minV < envelope[m].minV) { envelope[m].minV = minV; envelope[m].minVCombo = c; }
            }
        }
    }

    return envelope;
}

// Helper: access load combinations from outside
const std::vector<LoadCombination>& Engine::getLoadCombinations() const
{
    return loadCombinations;
}

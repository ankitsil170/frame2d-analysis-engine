#include "frame_solver.h"
#include <cmath>
#include <stdexcept>

/*-------------------------------------------------------
  DOF ASSIGNMENT (REDUCED SYSTEM ONLY)
-------------------------------------------------------*/
int FrameSolver::assignDOFs(std::vector<Node>& nodes)
{
    int counter = 0;

    for (auto& n : nodes)
    {
        n.dofUx = n.fixUx ? -1 : counter++;
        n.dofUy = n.fixUy ? -1 : counter++;
        n.dofRz = n.fixRz ? -1 : counter++;
    }

    return counter; // total FREE DOFs
}

/*-------------------------------------------------------
  LOCAL STIFFNESS MATRIX (2D FRAME)
-------------------------------------------------------*/
Eigen::Matrix<double, 6, 6>
FrameSolver::localStiffness(double E, double A, double I, double L)
{
    Eigen::Matrix<double, 6, 6> k;
    k.setZero();

    double EA_L  = E * A / L;
    double EI_L3 = E * I / (L * L * L);
    double EI_L2 = E * I / (L * L);
    double EI_L  = E * I / L;

    k <<
        EA_L,        0,           0,    -EA_L,        0,           0,
        0,   12*EI_L3,   6*EI_L2,      0, -12*EI_L3,   6*EI_L2,
        0,    6*EI_L2,    4*EI_L,      0,  -6*EI_L2,    2*EI_L,
        -EA_L,        0,           0,     EA_L,        0,           0,
        0, -12*EI_L3,  -6*EI_L2,      0,  12*EI_L3,  -6*EI_L2,
        0,    6*EI_L2,    2*EI_L,      0,  -6*EI_L2,    4*EI_L;

    return k;
}

/*-------------------------------------------------------
  TRANSFORMATION MATRIX
-------------------------------------------------------*/
Eigen::Matrix<double, 6, 6>
FrameSolver::transformation(double xi, double yi,
                            double xj, double yj)
{
    double dx = xj - xi;
    double dy = yj - yi;
    double L  = std::sqrt(dx*dx + dy*dy);

    double c = dx / L;
    double s = dy / L;

    Eigen::Matrix<double, 6, 6> T;
    T.setZero();

    T <<
        c,  s, 0,  0,  0, 0,
        -s,  c, 0,  0,  0, 0,
        0,  0, 1,  0,  0, 0,
        0,  0, 0,  c,  s, 0,
        0,  0, 0, -s,  c, 0,
        0,  0, 0,  0,  0, 1;

    return T;
}

/*-------------------------------------------------------
  ELEMENT GLOBAL STIFFNESS
-------------------------------------------------------*/
Eigen::Matrix<double, 6, 6>
FrameSolver::globalStiffness(double E, double A, double I,
                             double xi, double yi,
                             double xj, double yj)
{
    double L = std::hypot(xj - xi, yj - yi);

    auto k_local = localStiffness(E, A, I, L);
    auto T       = transformation(xi, yi, xj, yj);

    return T.transpose() * k_local * T;
}

/*-------------------------------------------------------
  NODE LOOKUP
-------------------------------------------------------*/
const Node& getNodeById(const std::vector<Node>& nodes, int id)
{
    for (const auto& n : nodes)
        if (n.id == id) return n;

    throw std::runtime_error("Node not found");
}

/*-------------------------------------------------------
  GLOBAL STIFFNESS ASSEMBLY (REDUCED SYSTEM)
-------------------------------------------------------*/
Eigen::MatrixXd FrameSolver::assembleGlobalStiffness(
    const std::vector<Node>& nodes,
    const std::vector<Member>& members,
    int totalDOF)
{
    Eigen::MatrixXd K = Eigen::MatrixXd::Zero(totalDOF, totalDOF);

    for (const Member& m : members)
    {
        const Node& ni = getNodeById(nodes, m.startNode);
        const Node& nj = getNodeById(nodes, m.endNode);

        Eigen::Matrix<double, 6, 6> ke =
            globalStiffness(m.E, m.A, m.I,
                            ni.x, ni.y,
                            nj.x, nj.y);

        int dofMap[6] = {
            ni.dofUx, ni.dofUy, ni.dofRz,
            nj.dofUx, nj.dofUy, nj.dofRz
        };

        for (int i = 0; i < 6; ++i)
        {
            if (dofMap[i] < 0) continue;

            for (int j = 0; j < 6; ++j)
            {
                if (dofMap[j] < 0) continue;

                K(dofMap[i], dofMap[j]) += ke(i, j);
            }
        }
    }

    return K;
}

/*-------------------------------------------------------
  APPLY BOUNDARY CONDITIONS
-------------------------------------------------------*/
void FrameSolver::applyBoundaryConditions(
    Eigen::MatrixXd& K,
    Eigen::VectorXd& F,
    const std::vector<Node>& nodes)
{

    for (const Node& n : nodes)
    {
        if (n.fixUx) constrainDOF(K, F, n.dofUx);
        if (n.fixUy) constrainDOF(K, F, n.dofUy);
        if (n.fixRz) constrainDOF(K, F, n.dofRz);
    }
}

void FrameSolver::constrainDOF(
    Eigen::MatrixXd& K,
    Eigen::VectorXd& F,
    int dof)
{
    if (dof < 0) return;

    K.row(dof).setZero();
    K.col(dof).setZero();
    K(dof, dof) = 1.0;
    F(dof) = 0.0;
}

/*-------------------------------------------------------
  SOLVER
-------------------------------------------------------*/
Eigen::VectorXd FrameSolver::solveDisplacements(
    const Eigen::MatrixXd& K,
    const Eigen::VectorXd& F)
{
    Eigen::LDLT<Eigen::MatrixXd> solver;
    solver.compute(K);

    if (solver.info() != Eigen::Success)
        throw std::runtime_error("LDLT decomposition failed");

    Eigen::VectorXd U = solver.solve(F);

    if (solver.info() != Eigen::Success)
        throw std::runtime_error("Solve failed");

    return U;
}

/*-------------------------------------------------------
  REACTIONS (USING ORIGINAL K & F)
-------------------------------------------------------*/
Eigen::VectorXd FrameSolver::computeReactions(
    const Eigen::MatrixXd& K_original,
    const Eigen::VectorXd& F_original,
    const Eigen::VectorXd& U)
{
    return K_original * U - F_original;
}

/*-------------------------------------------------------
  MEMBER END FORCES (FINAL, SINGLE SOURCE OF TRUTH)
-------------------------------------------------------*/
Eigen::VectorXd FrameSolver::computeMemberEndForces(
    const Member& m,
    const Node& ni,
    const Node& nj,
    const Eigen::VectorXd& U)
{
    double L = std::hypot(nj.x - ni.x, nj.y - ni.y);

    Eigen::Matrix<double, 6, 6> k_local =
        localStiffness(m.E, m.A, m.I, L);

    Eigen::Matrix<double, 6, 6> T =
        transformation(ni.x, ni.y, nj.x, nj.y);

    auto val = [&](int dof) {
        return (dof >= 0) ? U[dof] : 0.0;
    };

    Eigen::Matrix<double, 6, 1> u_global;
    u_global <<
        val(ni.dofUx),
        val(ni.dofUy),
        val(ni.dofRz),
        val(nj.dofUx),
        val(nj.dofUy),
        val(nj.dofRz);

    Eigen::Matrix<double, 6, 1> u_local = T * u_global;

    return k_local * u_local;
}
/*-------------------------------------------------------
  GLOBAL STIFFNESS (FULL SYSTEM)
-------------------------------------------------------*/
Eigen::MatrixXd FrameSolver::assembleFullStiffness(
    const std::vector<Node>& nodes,
    const std::vector<Member>& members)
{
    int nNodes = nodes.size();
    int fullDOF = nNodes * 3;

    Eigen::MatrixXd K =
        Eigen::MatrixXd::Zero(fullDOF, fullDOF);

    auto nodeIndex = [&](int nodeId) {
        for (int i = 0; i < nNodes; ++i)
            if (nodes[i].id == nodeId) return i;
        throw std::runtime_error("Node not found");
    };

    for (const Member& m : members)
    {
        int i = nodeIndex(m.startNode);
        int j = nodeIndex(m.endNode);

        const Node& ni = nodes[i];
        const Node& nj = nodes[j];

        Eigen::Matrix<double, 6, 6> ke =
            globalStiffness(m.E, m.A, m.I,
                            ni.x, ni.y,
                            nj.x, nj.y);

        int map[6] = {
            i * 3 + 0,  // Ui
            i * 3 + 1,
            i * 3 + 2,
            j * 3 + 0,  // Uj
            j * 3 + 1,
            j * 3 + 2
        };

        for (int r = 0; r < 6; ++r)
            for (int c = 0; c < 6; ++c)
                K(map[r], map[c]) += ke(r, c);
    }

    return K;
}

/*-------------------------------------------------------
 UDL (uniform distributed load)
-------------------------------------------------------*/
Eigen::Matrix<double,6,1>
FrameSolver::fixedEndForcesUDL(double w, double L)
{
    Eigen::Matrix<double,6,1> f;
    f.setZero();

    f(1) =  w * L / 2.0;
    f(2) =  w * L * L / 12.0;

    f(4) =  w * L / 2.0;
    f(5) = -w * L * L / 12.0;

    return f;
}

Eigen::Matrix<double,6,1>
FrameSolver::fixedEndForcesPartialUDL(
    double w,
    double L,
    double a,
    double b)
{
    Eigen::Matrix<double,6,1> f;
    f.setZero();

    double l = b - a;
    if (l <= 0.0) return f;

    double V1 =
        w*l/2.0 *
        (1.0 - a/L - l/(3.0*L));

    double V2 =
        w*l/2.0 *
        (a/L + 2.0*l/(3.0*L));

    double M1 =
        w*l/12.0 *
        (6*a + 3*l - 2*l*l/L - 3*a*a/L);

    double M2 =
        -w*l/12.0 *
        (3*a + 6*l - 2*l*l/L - 3*a*a/L);

    f(1) = V1;
    f(2) = M1;
    f(4) = V2;
    f(5) = M2;

    return f;
}

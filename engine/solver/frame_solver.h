#ifndef FRAME_SOLVER_H
#define FRAME_SOLVER_H
#include <Eigen/Dense>
#include "../model/Node.h"
#include "../model/Member.h"

class FrameSolver {
public:

    static int assignDOFs(std::vector<Node>& nodes);
    static Eigen::Matrix<double, 6, 6>
    localStiffness(double E, double A, double I, double L);

    Eigen::Matrix<double, 6, 6>
    transformation(double xi, double yi,
                   double xj, double yj);

    Eigen::Matrix<double, 6, 6>
    globalStiffness(double E, double A, double I,
                    double xi, double yi,
                    double xj, double yj);

    Eigen::MatrixXd assembleGlobalStiffness(
        const std::vector<Node>& nodes,
        const std::vector<Member>& members,
        int totalDOF
        );


    void applyBoundaryConditions(
        Eigen::MatrixXd& K,
        Eigen::VectorXd& F,
        const std::vector<Node>& nodes
        );

    void constrainDOF(
        Eigen::MatrixXd& K,
        Eigen::VectorXd& F,
        int dof
        );

    Eigen::VectorXd solveDisplacements(
        const Eigen::MatrixXd& K,
        const Eigen::VectorXd& F
        );

    Eigen::VectorXd computeReactions(
        const Eigen::MatrixXd& K_original,
        const Eigen::VectorXd& F_original,
        const Eigen::VectorXd& U
        );

    Eigen::VectorXd computeMemberEndForces(
        const Member& m,
        const Node& ni,
        const Node& nj,
        const Eigen::VectorXd& U
        );

    Eigen::VectorXd expandDisplacements(
        const std::vector<Node>& nodes,
        const Eigen::VectorXd& U_reduced
        );

    Eigen::MatrixXd assembleFullStiffness(
        const std::vector<Node>& nodes,
        const std::vector<Member>& members);


    Eigen::Matrix<double, 6, 1> getMemberGlobalDisplacements(
        const Member& m,
        const std::vector<Node>& nodes,
        const Eigen::VectorXd& U_full
        );

    Eigen::Matrix<double, 6, 1> computeMemberEndForces(
        const Member& m,
        const std::vector<Node>& nodes,
        const Eigen::VectorXd& U_full
        );

    Eigen::Matrix<double,6,1>
    fixedEndForcesUDL(double w, double L);

    Eigen::Matrix<double,6,1>
    fixedEndForcesPartialUDL(
        double w,
        double L,
        double a,
        double b
        );


private:
    int nodeIndex(const std::vector<Node>& nodes, int nodeId);




};

#endif // FRAME_SOLVER_H

// #pragma once

// #include <Eigen/Dense>
// #include "Load.h"
// struct Member {
//     int id;
//     int startNode;
//     int endNode;

//     double E;
//     double A;
//     double I;

//     // 🔥 MEMBER END FORCES (LOCAL)
//     double N1 = 0.0, V1 = 0.0, M1 = 0.0;
//     double N2 = 0.0, V2 = 0.0, M2 = 0.0;

//     // double udl = 0.0;   // Uniform load in LOCAL Y (kN/m)
//     // double udlStart = 0.0;   // a (local x start)
//     // double udlEnd   = 0.0;   // b (local x end)

//     // bool hasUDL() const
//     // {
//     //     return std::abs(udlEnd - udlStart) > 1e-12
//     //            && std::abs(udl) > 1e-12;
//     // }

//     // double shearAt(double x) const;
//     double shearAt(double x,
//                            const std::vector<Load>& loads,
//                    double L) const;
//     // double momentAt(double x) const;
//     double momentAt(double x,
//                             const std::vector<Load>& loads,
//                             double L) const;
//     // double getCriticalX() const;
//     double getCriticalX(const std::vector<Load>& loads,
//                                 double L) const;
//     // double getMaxMoment(double L) const;
//     double getMaxMoment(double L,
//                                 const std::vector<Load>& loads) const;

//     Member(int id_, int s, int e, double E_, double A_, double I_)
//         : id(id_), startNode(s), endNode(e), E(E_), A(A_), I(I_) {}
// };

#pragma once

#include <Eigen/Dense>
#include "Load.h"

struct Member
{
    int id;
    int startNode;
    int endNode;

    double E;
    double A;
    double I;

    // --- Display values (UI only) ---
    double displayN1 = 0.0;
    double displayV1 = 0.0;
    double displayM1 = 0.0;

    double displayN2 = 0.0;
    double displayV2 = 0.0;
    double displayM2 = 0.0;

    Member(int id_, int s, int e,
           double E_, double A_, double I_)
        : id(id_),
        startNode(s),
        endNode(e),
        E(E_),
        A(A_),
        I(I_) {}

    // Shear and moment evaluation now require
    // end forces passed explicitly

    static double shearAt(
        double x,
        double L,
        double V1,
        double V2,
        const std::vector<Load>& loads);

    static double momentAt(
        double x,
        double L,
        double M1,
        double V1,
        const std::vector<Load>& loads);
};

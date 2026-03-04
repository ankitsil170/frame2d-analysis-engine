#pragma once

enum class LoadType
{
    Nodal,
    UDL,
    PartialUDL
};

struct Load
{
    LoadType type;

    int nodeId = -1;      // for nodal loads
    int memberId = -1;    // for member loads

    // Nodal load components
    double fx = 0.0;
    double fy = 0.0;
    double mz = 0.0;

    // UDL intensity
    double w = 0.0;

    // Partial UDL positions
    double startPos = 0.0;
    double endPos = 0.0;
};

// struct Load {
//     int nodeId;
//     double fx;   // global X
//     double fy;   // global Y
//     double mz;   // moment (2D)

//     Load(int nid, double fx_, double fy_, double mz_ = 0.0)
//         : nodeId(nid), fx(fx_), fy(fy_), mz(mz_) {}
// };


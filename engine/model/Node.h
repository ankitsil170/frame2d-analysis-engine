// #pragma once
// enum class SupportType {
//     None,
//     Fixed,
//     Pinned,
//     Roller
// };
// struct Node {
//     int id;
//     double x, y, z;



//     SupportType support = SupportType::None;

//     bool fixUx = false;
//     bool fixUy = false;
//     bool fixRz = false;

//     int dofUx = -1;
//     int dofUy = -1;
//     int dofRz = -1;

//     // int dofUx, dofUy, dofRz;

//     // bool restrainUx = false;
//     // bool restrainUy = false;
//     // bool restrainRz = false;

//     // Full system DOFs (always exist)
//     int fullDofUx = -1;
//     int fullDofUy = -1;
//     int fullDofRz = -1;

//     // STORED REACTIONS
//     double Rx = 0.0;
//     double Ry = 0.0;
//     double Mz = 0.0;

//     Node(int id_, double x_, double y_, double z_ = 0.0)
//         : id(id_), x(x_), y(y_), z(z_) {}
// };

#pragma once

enum class SupportType
{
    None,
    Fixed,
    Pinned,
    Roller
};

struct Node
{
    int id;
    double x, y, z;

    SupportType support = SupportType::None;

    bool fixUx = false;
    bool fixUy = false;
    bool fixRz = false;

    // Reduced system DOFs
    int dofUx = -1;
    int dofUy = -1;
    int dofRz = -1;

    // Full system DOFs
    int fullDofUx = -1;
    int fullDofUy = -1;
    int fullDofRz = -1;

    // --- Display values (UI only) ---
    double displayRx = 0.0;
    double displayRy = 0.0;
    double displayMz = 0.0;

    Node(int id_, double x_, double y_, double z_ = 0.0)
        : id(id_), x(x_), y(y_), z(z_) {}
};

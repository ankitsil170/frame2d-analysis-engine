// #include "member.h"


// double Member::shearAt(double x,
//                        const std::vector<Load>& loads,
//                        double L) const
// {
//     double V = V1;

//     for (const auto& load : loads)
//     {
//         if (load.memberId != id)
//             continue;

//         if (load.type == LoadType::UDL ||
//             load.type == LoadType::PartialUDL)
//         {
//             double a = 0.0;
//             double b = L;

//             if (load.type == LoadType::PartialUDL)
//             {
//                 a = load.startPos;
//                 b = load.endPos;
//             }

//             if (x > a)
//             {
//                 double effective = std::min(x, b) - a;
//                 if (effective > 0)
//                     V -= load.w * effective;
//             }
//         }
//     }

//     return V;
// }


// double Member::momentAt(double x,
//                         const std::vector<Load>& loads,
//                         double L) const
// {
//     double M = M1 + V1 * x;

//     for (const auto& load : loads)
//     {
//         if (load.memberId != id)
//             continue;

//         if (load.type == LoadType::UDL ||
//             load.type == LoadType::PartialUDL)
//         {
//             double a = 0.0;
//             double b = L;

//             if (load.type == LoadType::PartialUDL)
//             {
//                 a = load.startPos;
//                 b = load.endPos;
//             }

//             if (x > a)
//             {
//                 double effective = std::min(x, b) - a;
//                 if (effective > 0)
//                     M -= load.w * effective * effective / 2.0;
//             }
//         }
//     }

//     return M;
// }


// // double Member::getCriticalX() const
// // {
// //     if (!hasUDL())
// //         return -1;

// //     if (std::abs(udl) < 1e-12)
// //         return -1;

// //     double x = udlStart + V1 / udl;

// //     if (x >= udlStart && x <= udlEnd)
// //         return x;

// //     return -1;
// // }

// double Member::getCriticalX(const std::vector<Load>& loads,
//                             double L) const
// {
//     for (const auto& load : loads)
//     {
//         if (load.memberId != id)
//             continue;

//         if (load.type == LoadType::UDL ||
//             load.type == LoadType::PartialUDL)
//         {
//             if (std::abs(load.w) < 1e-12)
//                 continue;

//             double a = 0.0;
//             double b = L;

//             if (load.type == LoadType::PartialUDL)
//             {
//                 a = load.startPos;
//                 b = load.endPos;
//             }

//             double x = a + V1 / load.w;

//             if (x >= a && x <= b)
//                 return x;
//         }
//     }

//     return -1.0;
// }

// double Member::getMaxMoment(double L,
//                             const std::vector<Load>& loads) const
// {
//     double maxM = std::max(std::abs(M1), std::abs(M2));

//     double xc = getCriticalX(loads, L);

//     if (xc >= 0.0 && xc <= L)
//     {
//         double Mc = momentAt(xc, loads, L);
//         maxM = std::max(maxM, std::abs(Mc));
//     }

//     return maxM;
// }


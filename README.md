# Frame2D Analysis Engine

A professional-grade 2D frame structural analysis engine built in **C++** and **Qt**, implementing the **Finite Element Method (Direct Stiffness Method)** from first principles.

This system supports full geometry modeling, multiple load cases, load combinations, envelope generation, and structural post-processing.

---

## 1. Geometry Modeling

The software provides complete 2D structural modeling capabilities:

- Add nodes (X, Y coordinates)
- Add members between nodes
- Move nodes
- Delete nodes and members
- Assign supports:
  - Fixed
  - Pinned
  - Roller
- Automatic boundary condition handling

This enables full 2D frame/beam structure creation.

---

## 2. Finite Element Solver Core

The engine implements a complete FEM pipeline:

- Global stiffness matrix assembly
- DOF handling (Ux, Uy, Rz per node)
- Load vector generation
- Boundary condition application
- Linear system solving (`Kx = F`)
- Support reaction calculation

The implementation is built from first principles without using external structural analysis libraries.

---

## 3. Load Types

Supported load types:

- Nodal loads (Fx, Fy, Mz)
- Uniformly Distributed Load (UDL)
- Partial UDL (user-defined start and end positions on a member)

Partial UDL handling includes correct equivalent nodal force transformation.

---

## 4. Post-Processing

After analysis, the engine computes:

- Node displacements
- Support reactions
- Member end forces:
  - Axial force (N)
  - Shear force (V)
  - Bending moment (M)
- Shear Force Diagram (SFD)
- Bending Moment Diagram (BMD)
- Internal critical point detection
- Maximum moment extraction

---

## 5. Load Case System

Professional load case architecture:

- Multiple independent load cases
- Separate load storage per case
- Independent solving per case
- Active case switching
- Case result storage

---

## 6. Load Combinations

Implemented using linear superposition:

- Load combination definition
- Case + factor system
- Combination solving
- Storing combination results
- Switching active combination

---

## 7. Envelope System

The engine generates structural envelopes across combinations:

- Maximum positive bending moment
- Maximum negative bending moment
- Maximum shear force
- Governing combination detection
- Envelope visualization
- Envelope result table

---

## 8. Results Interface

The UI includes:

- Member force result tables
- Envelope tables
- Docked result panel
- Structured numerical output
- Undeformed / deformed visualization
- Diagram scaling controls

---

## Tech Stack

- C++
- Qt (UI and visualization)
- CMake (build system)
- Eigen (linear algebra solver)

---

## Project Structure
.
├── CMakeLists.txt
├── main.cpp
│── mainwindow.cpp
├── mainwindow.h
├── mainwindow.ui
├── engine/
│ ├── engine.cpp
│ ├── engine.h
│ │
│ ├── model/
│ │ ├── Node.h
│ │ ├── member.h
│ │ ├── member.cpp
│ │ ├── Load.h
│ │
│ ├── loads/
│ │ └── (load handling logic)
│ │
│ ├── analysis/
│ │ └── (post-processing & results logic)
│ │
│ └── solver/
│ ├── frame_solver.h
│ └── frame_solver.cpp
│
├── ui/
│ │
│ ├── graphicsview.cpp
│ ├── graphicsview.h
│ │
│ └── dialogs/
│ ├── loaddialog.cpp
│ ├── loaddialog.h
│ └── loaddialog.ui
│
├── external/
│ └── (third-party libraries if bundled)
│
└── build/ (ignored via .gitignore)

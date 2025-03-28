# Tiny Simulator 



## Overview

A lightweight physics simulation engine implemented in C++ using Entity-Component-System (ECS) architecture via [entt](https://github.com/skypjack/entt), with OpenGL-based visualization. Key features include:

1. Position-Based Dynamics for cloth simulation
2. Rigid Body Dynamics
3. Collision Detection (OBB, Capsule, Sphere)

## Building Instructions



### Windows 11

**Compiler**: Visual Studio 2022

From project root:

```cmd
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022"
cmake --build . --config Release
```

Binary: `build/src/Release/tiny-simulator.exe`

### macOS

**Toolchain**: Xcode command-line tools

From project root:

```bash
mkdir build
cd build
cmake .. -G "Xcode"
xcodebuild -configuration Release
```

Binary: `build/src/Release/tiny-simulator`

### Linux

**Compiler**: g++

From project root:

```bash
cmake -DCMAKE_BUILD_TYPE=Release -S . -B build
cd build
make -j$(nproc)
```

Binary: `build/src/tiny-simulator`



## Architecture

```
.
├── include
│   ├── core            # Core modules (Event/FSM/Window)
│   ├── ecs             # ECS implementation (Components/Systems)
│   ├── renderer        # Rendering pipeline
│   └── scene           # Scene management (Camera/Lighting/Assets)
├── shaders            # GLSL shaders
└── src                # Source implementation
    └── (Mirrors include structure)
```



## Core Technologies

### Physics Simulation

#### Position-Based Dynamics (PBD)

Algorithm pseudocode:

```pseudocode
// Initialization
for all vertices i do
    initialize xi = xi0, vi = vi0, wi = 1/mi
end

// Main loop
while simulating do
    // External forces
    for all vertices i do
        vi ← vi + Δt * wi * fext(xi)
        pi ← xi + Δt * vi
    end

    // Collision constraints
    for all vertices i do
        generate_collision_constraints(xi → pi)
    end

    // Constraint projection
    for k = 1 to solverIterations do
        project_constraints(C1,...,Cm, p1,...,pn)
    end

    // Update states
    for all vertices i do
        vi ← (pi - xi) / Δt
        xi ← pi
    end

    // Velocity post-processing
    apply_velocity_damping(v1,...,vn)
end
```

#### Constraint Implementations

1. **Maintain cloth edge elasticity**:

   Maintain the elastic constraints on the edge lengths of the cloth mesh, following Hooke's Law.
   $$
   \Delta \mathbf{p}_1 = -\frac{m_2}{m_1+m_2}(||\mathbf{p}_1-\mathbf{p}_2||-d)\frac{\mathbf{p}_1-\mathbf{p}_2}{||\mathbf{p}_1-\mathbf{p}_2||} \\
   \Delta \mathbf{p}_2 = +\frac{m_1}{m_1+m_2}(||\mathbf{p}_1-\mathbf{p}_2||-d)\frac{\mathbf{p}_1-\mathbf{p}_2}{||\mathbf{p}_1-\mathbf{p}_2||}
   $$

2. **Bending Constraints**

   Control bending stiffness via adjacent face normals:
   $$
   \begin{align}
   & \text{Without loss of generatlity we set:}\\
   & p_1=0,\ d=n_1\cdot n_2,\ n_1=\frac{p_2\times p_3}{|p_2\times p_3|},\ n_2=\frac{p_2\times p_4}{|p_2\times p_4|}\\
   & q_3=\frac{p_2\times n_2 + (n_1\times p_2)d}{|p_2\times p_3|}\\
   & q_4=\frac{p_2\times n_1 + (n_2\times p_2)d}{|p_2\times p_4|}\\
   & q_2=-\frac{p_3\times n_2 + (n_1\times p_3)d}{|p_2\times p_3|}-\frac{p_4\times n_1 + (n_2\times p_4)d}{|p_2\times p_4|}\\
   & q_1=-q_2-q_3-q_4\\
   & \Delta p_i=-\frac{w_i\sqrt{1-d^2}(\arccos(d)-\phi_0)}{\sum_jw_j|q_j|^2}q_i
   \end{align}
   $$
   

3. **Collision Handling**

   Discrete collision detection with surface projection.



## Demo

![Cloth Simulation Demo](./assets/pbd.gif)
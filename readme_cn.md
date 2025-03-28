# Tiny Simulator



## 项目概述

本系统是基于C++实现的轻量级物理仿真引擎，采用[entt](https://github.com/skypjack/entt)库构建的实体组件系统（ECS）架构，并集成OpenGL可视化模块。当前实现的物理特性包括：

1. 基于位置动力学的布料仿真（Position Based Dynamics, PBD）
2. 刚体动力学系统
3. 碰撞检测系统（支持OBB定向包围盒、胶囊体、球体）



## 编译指南

### Windows 11
**编译环境**: Visual Studio 2022
在项目根目录执行：
```cmd
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022"
cmake --build . --config Release
```
生成可执行文件路径：`build/src/Release/tiny-simulator.exe`

### macOS
**编译环境**: Xcode 命令行工具
在项目根目录执行：
```bash
mkdir build
cd build
cmake .. -G "Xcode"
xcodebuild -configuration Release
```
生成可执行文件路径：`build/src/Release/tiny-simulator`

### Linux
**编译环境**: g++
在项目根目录执行：
```bash
cmake -DCMAKE_BUILD_TYPE=Release -S . -B build
cd build
make -j$(nproc)
```
生成可执行文件路径：`build/src/tiny-simulator`



## 架构设计

```
.
├── include
│   ├── core            # 核心模块（事件处理/文件系统/窗口管理）
│   ├── ecs             # ECS架构实现（组件/系统）
│   ├── renderer        # 渲染管线管理
│   └── scene           # 场景管理（相机/光照/模型/资源）
├── shaders            # GLSL着色器资源
└── src                # 源代码实现
    └── (结构与include对应)
```



## 核心技术

### 物理仿真系统

#### 位置动力学（PBD）算法
算法流程伪代码：
```pseudocode
// 初始化阶段
for all vertices i do
    initialize xi = xi0, vi = vi0, wi = 1/mi
end

// 主循环
while simulating do
    // 外力积分
    for all vertices i do
        vi ← vi + Δt * wi * fext(xi)
        pi ← xi + Δt * vi
    end

    // 碰撞约束生成
    for all vertices i do
        generate_collision_constraints(xi → pi)
    end

    // 约束求解迭代
    for k = 1 to solverIterations do
        project_constraints(C1,...,Cm, p1,...,pn)
    end

    // 位置与速度更新
    for all vertices i do
        vi ← (pi - xi) / Δt
        xi ← pi
    end

    // 速度后处理
    apply_velocity_damping(v1,...,vn)
end
```

#### 约束类型实现

1. **距离约束**

   维持布料网格边长的弹性约束，遵循胡克定律：
   $$
   \Delta \mathbf{p}_1 = -\frac{m_2}{m_1+m_2}(||\mathbf{p}_1-\mathbf{p}_2||-d)\frac{\mathbf{p}_1-\mathbf{p}_2}{||\mathbf{p}_1-\mathbf{p}_2||} \\
   \Delta \mathbf{p}_2 = +\frac{m_1}{m_1+m_2}(||\mathbf{p}_1-\mathbf{p}_2||-d)\frac{\mathbf{p}_1-\mathbf{p}_2}{||\mathbf{p}_1-\mathbf{p}_2||}
   $$

2. **弯曲约束**

   通过相邻三角面法向量控制布料弯曲刚度：
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

3. **碰撞约束**

   采用离散检测法，将穿透碰撞体的顶点投影至最近表面。



## 效果演示

![布料仿真演示](./assets/pbd.gif)

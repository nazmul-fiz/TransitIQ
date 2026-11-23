# TransitIQ — Intelligent Public Transport & Journey Explorer (Qt6 GUI Edition)

TransitIQ is a modern **C++17 bus transport simulator & visual journey planner** built with **Qt6 and CMake**, modeled after Dhaka's metropolitan bus transit network (23 bus stops, 30 major road corridors, and 8 prominent bus fleets).

The visual UI is directly inspired by the modern dark-mode transit dashboard design of [transitiq.bolt.host](https://transitiq.bolt.host), featuring deep obsidian cards, electric cyan neon accents, interactive bus cards, and a vector Dhaka transit map.

The project strictly demonstrates **Breadth-First Search (BFS)** and **Depth-First Search (DFS)** as its primary algorithms (no Dijkstra or weighted shortest-path algorithms), making it intuitive, visually engaging, and ideal for academic viva presentations.

---

## 🎨 Design & Visual Features (Inspired by transitiq.bolt.host)

### 1. 🖥️ Interactive Vector Dhaka Transit Map Canvas (`QtMapWidget`)
* **Vector Anti-aliased Map**: High-DPI QPainter rendering with smooth bezier/rounded corridors connecting all 23 Dhaka stops.
* **Vibrant Route Accents**:
  * 🟡 **Golden-Yellow**: Origin / Boarding Stop with an outer ambient pulse halo.
  * 🔵 **Electric Cyan (`#38bdf8`)**: Final Destination Stop with a target reticle.
  * 🟢 **Glowing Emerald Green (`#10b981`)**: Active BFS journey route with directional arrow indicators.
  * 🔴 **Red Dashed Corridor & ⛔ Badge**: Dynamic road closure simulation (construction/blockage).
  * 🟣 **Magenta & Multi-color Lines**: Multi-leg bus transfers.
* **Animated Step-by-Step Traversal**:
  * Clicking **`▶ Animate Steps`** triggers a smooth timer that visibly illustrates how BFS expands outward layer-by-layer or how DFS explores corridors deeply before backtracking!
* **Map Legend**: Glassmorphism HUD overlay at the bottom right.

### 2. 🖱️ Zero Manual Typing – Pure Dropdown Selection
No typing or manual command entry is required:
* **Boarding / Origin Stop**: Select from a dropdown list of all 23 Dhaka stops.
* **Destination Stop**: Select from a dropdown list.
* **Algorithm / Exploration Mode**:
  1. 🧭 **BFS Journey Finder**: Route with minimum intermediate connections.
  2. 🔄 **BFS Bus Transfer Finder**: Route with minimum vehicle changes across bus lines.
  3. 🌐 **BFS Reachability Explorer**: Discovers all stops reachable within hop levels.
  4. 🔍 **DFS Network Explorer**: Deep corridor walk and recursion visualization.
  5. 🏝️ **DFS Connected Components & Island Detection**: Detects isolated/unreachable stops.
  6. ⚠️ **DFS Road Closure & Bottleneck Impact**: Bridge analysis and bypasses.
  7. 🚌 **Bus Recommendation System**: Ranks Top 3 buses with timetable comparisons.
* **Simulate Road Block / Closure**:
  * Dropdown options to simulate closing corridors (*Farmgate $\leftrightarrow$ Karwan-Bazar*, *Airport $\leftrightarrow$ Khilkhet*, *Kuril $\leftrightarrow$ Banani*, etc.) to watch BFS dynamically calculate detours!

### 3. 🚌 Bus Recommendation Cards (Matching transitiq.bolt.host)
* Predefined comparison cards with:
  * **Tagline badges**: *"Best match for this route"*, *"Fewest stops"*, *"Fastest journey time"*.
  * **Bus Service Name** + **Bus Type** (`AC Express`, `Semi-Seating`, `Regular`).
  * **Timetable Metrics**: Estimated travel time (mins), road distance (km), intermediate stops count, frequency, and fare in BDT (৳).
* **Route Metrics Summary Grid**: Instant badges for **STOPS**, **HOPS**, **DIST (km)**, and **EST. TIME (min)**.

---

## 🌟 Algorithmic Foundations

### 1. BFS – Journey Finder & Route Optimization
* **Minimum Connections Route**: In an unweighted graph where each road is 1 transit connection, BFS mathematically guarantees the route with the **minimum number of connections (stop hops)** in $\mathcal{O}(V + E)$ time.
* **Dynamic Road Closure Bypass**: When a road is blocked, BFS skips the closed edge and discovers the alternative detour path automatically.
* **Minimum Bus Transfers Model**: Evaluates the transit network where states represent bus stops and transitions represent boarding a bus line. BFS guarantees the route requiring the fewest bus switches.

### 2. DFS – Network Explorer & Topology Analysis
* **Corridor Exploration Trace**: Follows corridors deeply until dead ends before backtracking.
* **Connected Components & Island Detection**: Identifies separated subnetworks and isolated stops that have no operational connections.
* **Road Closure & Bottleneck Impact Analysis**: Simulates closing a corridor $(u, v)$ and compares connected components before vs. after. If the number of components increases, the road is detected as a **critical bridge** (single point of failure).

---

## 🚀 How to Run TransitIQ

### 1. One-Click Launch (Double Click)
* **[TransitIQ.exe](file:///d:/New%20folder/Algo%20Project/TransitIQ.exe)** or **[Run_TransitIQ.bat](file:///d:/New%20folder/Algo%20Project/Run_TransitIQ.bat)**:
  * Double-click either file to launch the Qt6 GUI desktop window.
  * All required Qt6 and MinGW runtime libraries are bundled.

### 2. Building from Source with CMake & Qt6
```cmd
build.bat
```
Or manually with CMake:
```bash
mkdir build && cd build
cmake -G "MinGW Makefiles" -DCMAKE_PREFIX_PATH="C:/Qt/6.6.3/mingw_64" ..
cmake --build .
```

### 3. Running in VS Code
1. Open the project folder in VS Code.
2. Press **`Ctrl + Shift + B`** to trigger the build task (`Build TransitIQ (CMake Qt6)`).
3. Press **`F5`** or launch `TransitIQ.exe`.

### 4. Running Algorithm Verification Tests
```powershell
.\test_algorithms.exe
```

---

## 📁 Project Structure

```
d:/New folder/Algo Project/
├── .vscode/
│   ├── tasks.json             # VS Code build and test tasks
│   └── launch.json            # VS Code GDB debug configuration
├── CMakeLists.txt             # Modern CMake build configuration for Qt6
├── data/
│   ├── stops.txt              # Stop ID, name, and zone (23 Dhaka stops)
│   ├── roads.txt              # Road connections, distance (km), time (min)
│   └── buses.txt              # 8 bus services, frequencies, fares, routes
├── include/
│   ├── Common.h               # ANSI colors and global animation config
│   ├── Stop.h                 # Bus stop data model
│   ├── Road.h                 # Road corridor data model
│   ├── Bus.h                  # Bus fleet model & subroute methods
│   ├── Graph.h                # Adjacency list graph & BFS/DFS algorithms
│   ├── BusManager.h           # Bus search & recommendation logic
│   ├── DataManager.h          # File reading/writing & data persistence
│   ├── UI.h                   # Console visualizers & banners
│   ├── QtMapWidget.h          # Qt6 vector Dhaka map canvas
│   └── QtMainWindow.h         # Qt6 modern dark-mode GUI dashboard
├── src/
│   ├── Common.cpp             # Windows console & sleep helpers
│   ├── Graph.cpp              # BFS & DFS core implementations
│   ├── BusManager.cpp         # Route analysis & bus ranking
│   ├── DataManager.cpp        # File I/O parser & default network builder
│   ├── UI.cpp                 # Screen rendering & ASCII route maps
│   ├── App.cpp                # Console application controller
│   ├── QtMapWidget.cpp        # Map canvas rendering & animations
│   ├── QtMainWindow.cpp       # Dashboard widgets, theme, & callbacks
│   └── main.cpp               # Program entry point (launches Qt6 GUI)
├── tests/
│   └── test_algorithms.cpp    # Automated test suite for all 7 algorithm requirements
├── build.bat                  # One-click CMake & Qt6 build script
├── Run_TransitIQ.bat          # Desktop launch shortcut script
├── TransitIQ.exe              # Ready-to-click Qt6 executable
└── test_algorithms.exe        # Standalone verification test runner
```

---

## 🌐 GitHub Repository

* **Repository**: [https://github.com/nazmul-fiz/TransitIQ.git](https://github.com/nazmul-fiz/TransitIQ.git)
* **Branch**: `main`

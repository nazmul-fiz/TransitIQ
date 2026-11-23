# TransitIQ – Smart Bus Network Explorer & Journey Finder (Visual GUI Edition)

TransitIQ is a modern, interactive **C++17 GUI & Console bus transport simulator** modeled after Dhaka's public transit network (23 bus stops, 30 major road corridors, and 8 prominent bus lines).

Designed specifically for academic coursework and project presentation, it strictly demonstrates **Breadth-First Search (BFS)** and **Depth-First Search (DFS)** as its primary algorithms. No Dijkstra or weighted shortest-path algorithms are used, keeping the project authentic, visual, and easy to explain.

---

## 🖥️ Visual Graphical Interface (Zero Manual Typing)

The application provides a **native graphical desktop interface (GUI)**:
* **Interactive Dhaka Transit Map Canvas**:
  * Real-time 2D visualization of Dhaka's bus transit network across North, Central, West, and South zones.
  * Stops are rendered as glowing circular nodes labeled with stop names.
  * Roads are rendered as sleek corridors.
  * **Selected Journey Route**: Lights up with thick vibrant emerald green lines (`#10b981`) and directional arrows.
  * **Blocked Roads**: Rendered as red dashed lines with a ⛔ roadblock indicator.
  * **Start / Boarding Stop**: Highlighted in golden-yellow (`#f59e0b`) with a pulse ring.
  * **Destination / Alight Stop**: Highlighted in bright cyan (`#06b6d4`).
* **Dropdown Selection (No Keyboard Input Required)**:
  * **Boarding / Origin Stop**: Select from a dropdown list of all 23 Dhaka stops.
  * **Destination Stop**: Select from a dropdown list.
  * **Algorithm / Exploration Mode**:
    1. 🧭 **BFS Journey Finder**: Minimum connections route between source and destination.
    2. 🔄 **BFS Bus Transfer Finder**: Minimum vehicle changes route across bus lines.
    3. 🌐 **BFS Reachability Explorer**: Hop-by-hop expansion from starting location.
    4. 🔍 **DFS Network Explorer**: Deep corridor walk trace and recursion visualization.
    5. 🏝️ **DFS Connected Components & Island Detection**: Identifies isolated stops.
    6. ⚠️ **DFS Road Closure & Bottleneck Impact**: Simulates closing corridors and detects critical bridges.
    7. 🚌 **Bus Recommendation System**: Ranks Top 3 direct/transfer buses with fare and travel time.
  * **Simulate Road Block / Closure**: Dropdown list to simulate closing corridors (e.g. *Farmgate $\leftrightarrow$ Karwan-Bazar*, *Airport $\leftrightarrow$ Khilkhet*, *Kuril $\leftrightarrow$ Banani*, etc.) to watch BFS dynamically calculate detours!
* **Step-by-Step Animation (`▶️ Animate Steps`)**:
  * Visualizes the BFS queue frontier or DFS recursion stack step-by-step with an animated timer on the map.
* **Side Panel Summary & Trace**:
  * Instant readout of total stops, transit hops, estimated distance (km), estimated time (mins), and bus recommendation comparison cards.

---

## 🌟 Core Algorithmic Implementations

### 1. BFS – Journey Finder & Route Optimization
* **Minimum Connections Route**: Uses `std::queue<int>` to explore level-by-level. In an unweighted transit graph where each road is 1 hop, BFS mathematically guarantees the route with the **minimum number of intermediate stops** in $\mathcal{O}(V + E)$ time.
* **Dynamic Road Closure Bypass**: When a road is blocked, BFS automatically avoids the blocked edge and discovers the bypass route.
* **Minimum Bus Transfers Model**: Runs BFS across the transit network where states represent bus stops and transitions represent taking a bus line. Level 0 = origin, Level 1 = reachable with 1 bus (direct), Level 2 = reachable with 2 buses (1 transfer). The first time the destination is dequeued, it is guaranteed to have the **minimum number of bus changes/transfers**.

### 2. DFS – Network Explorer & Topology Analysis
* **Corridor Exploration Trace**: Follows transit corridors deeply until dead ends before backtracking, illustrating recursion depth and call-stack behavior.
* **Connected Components & Island Detection**: Uses DFS to identify separated transit networks and isolate stops that have no operational connections.
* **Road Closure & Bottleneck Impact Analysis**: Simulates closing a corridor $(u, v)$ and compares connected components before vs. after. If the number of components increases, the road is detected as a **critical bridge** (single point of failure).

### 3. Bus Recommendation & Comparison
* **Top 3 Bus Recommendations**: Recommends direct or 1-transfer buses between two stops.
* **Predefined Comparisons**: Compares services by estimated travel time, total distance, intermediate stops, frequency, and base fare using the predefined timetable metrics of each bus corridor.

---

## 🚀 How to Run the Visual GUI

### 1. Single Click (Recommended)
Simply double-click:
* **[TransitIQ.exe](file:///d:/New%20folder/Algo%20Project/TransitIQ.exe)** or **[TransitIQ_GUI.exe](file:///d:/New%20folder/Algo%20Project/TransitIQ_GUI.exe)**
* Or **[Run_TransitIQ.bat](file:///d:/New%20folder/Algo%20Project/Run_TransitIQ.bat)**

The interactive GUI window will immediately open!

### 2. Optional Console Mode
If you wish to run the animated console terminal mode instead:
```powershell
.\TransitIQ.exe --cli
```

### 3. In VS Code
1. Open the project folder in VS Code.
2. Press **`Ctrl + Shift + B`** to build.
3. Launch with **`F5`** or run `./TransitIQ.exe`.

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
├── data/
│   ├── stops.txt              # Stop ID, name, and zone (23 Dhaka stops)
│   ├── roads.txt              # Physical road corridors, distance (km), time (min)
│   └── buses.txt              # 8 bus services, frequencies, fares, routes
├── include/
│   ├── Common.h               # ANSI colors, console configuration, and animation settings
│   ├── Stop.h                 # Bus stop data model
│   ├── Road.h                 # Road corridor data model
│   ├── Bus.h                  # Bus fleet data model & subroute extractors
│   ├── Graph.h                # Adjacency list graph & BFS/DFS algorithms
│   ├── BusManager.h           # Bus search & recommendation logic
│   ├── DataManager.h          # File reading/writing & data persistence
│   ├── UI.h                   # Visualizers, banners, and interactive prompts
│   ├── GUIApp.h               # Win32 Graphical User Interface & Canvas header
│   └── App.h                  # Application menu controller
├── src/
│   ├── Common.cpp             # Windows ANSI initialization & sleep helpers
│   ├── Graph.cpp              # BFS and DFS core implementations
│   ├── BusManager.cpp         # Route analysis & bus ranking
│   ├── DataManager.cpp        # File I/O parser & default network builder
│   ├── UI.cpp                 # Screen rendering & ASCII route maps
│   ├── GUIApp.cpp             # Interactive Dhaka map drawing & GUI controls
│   ├── App.cpp                # Console application workflows
│   └── main.cpp               # Program entry point (launches GUI or CLI)
├── tests/
│   └── test_algorithms.cpp    # Automated test suite for all 7 algorithm requirements
├── build.bat                  # One-click Windows build and run batch file
├── Run_TransitIQ.bat          # Desktop launch shortcut script
├── Makefile                   # Make build script
├── TransitIQ.exe              # Ready-to-click GUI executable
└── TransitIQ_GUI.exe          # Dedicated GUI executable
```

---

## 🌐 GitHub Repository

* **Repository**: [https://github.com/nazmul-fiz/TransitIQ.git](https://github.com/nazmul-fiz/TransitIQ.git)
* **Branch**: `main`

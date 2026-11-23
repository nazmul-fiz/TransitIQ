# TransitIQ – Smart Bus Network Explorer & Journey Finder

TransitIQ is a modern, modular **C++17 console-based bus transport simulator** modeled after Dhaka's public transit network (featuring 23 bus stops, 30 major road corridors, and 8 prominent bus lines).

Designed specifically for academic coursework, this project strictly demonstrates **Breadth-First Search (BFS)** and **Depth-First Search (DFS)** as its primary algorithms, avoiding shortest-path algorithms like Dijkstra to keep the algorithmic focus clear, rigorous, and easy to explain during viva presentations.

---

## 🌟 Key Features

### 1. BFS – Journey Finder & Route Optimization
* **Minimum Connections Route**: Finds the route with the fewest intermediate stops/hops between any two locations using standard level-by-level BFS queue expansion.
* **Dynamic Road Closure Bypass**: When a road is blocked (e.g., due to construction or VIP movement), BFS dynamically identifies the shortest detour/alternative corridor.
* **Hop-based Reachability Explorer**: Explores all transit stops reachable from an origin within $k$ connections or overall.
* **Minimum Bus Transfers Model**: Evaluates the bus route transit network using BFS where each state transition represents boarding a bus line, guaranteeing the journey with the minimum number of vehicle switches.
* **Animated Queue Visualization**: Renders FIFO queue pops, pushes, and frontier changes in real time with step-by-step color-coded feedback.

### 2. DFS – Network Explorer & Topology Analysis
* **Corridor Exploration Trace**: Follows transit corridors deeply until dead ends before backtracking, illustrating recursion depth and call-stack behavior.
* **Connected Components & Island Detection**: Uses DFS to identify separated transit networks and isolate stops that have no operational connections.
* **Road Closure & Bottleneck Impact Analysis**: Simulates closing any road and uses DFS to analyze if the corridor is a **critical bridge** (single point of failure) that splits the city into disconnected components.

### 3. Bus Recommendation & Comparison
* **Top 3 Bus Recommendations**: Recommends direct or 1-transfer buses between two stops.
* **Multifaceted Comparison**: Compares services by estimated travel time, total distance, intermediate stops, frequency, and base fare.
* **Honest Algorithmic Framing**: Predefined bus timetable speeds and road corridor metrics are used for comparisons, clearly distinguishing route metrics from unweighted graph traversals.

### 4. Interactive Console Interface & Data Persistence
* **Modern ANSI Color Palette**: Green for active routes, Red for blocked roads, Yellow for origin, Cyan for destination, and Magenta for transfer interchanges.
* **ASCII Visual Maps**: Clear directional diagrams showing intermediate stops, road distances, and time estimates.
* **Full CRUD Admin Menu**: Add, edit, or delete bus stops, road segments, and bus lines directly from the console.
* **File Persistence**: All changes automatically synchronize with `data/stops.txt`, `data/roads.txt`, and `data/buses.txt`.

---

## 📁 Project Structure

```
d:/New folder/Algo Project/
├── .vscode/
│   ├── tasks.json             # VS Code build and test tasks
│   └── launch.json            # VS Code GDB debug configuration
├── data/
│   ├── stops.txt              # Stop ID, name, and zone
│   ├── roads.txt              # Direct road connections, distance (km), time (min)
│   └── buses.txt              # Bus ID, name, type, frequency, fare, route stop IDs
├── include/
│   ├── Common.h               # ANSI colors, console configuration, and animation settings
│   ├── Stop.h                 # Bus stop data model
│   ├── Road.h                 # Road corridor data model
│   ├── Bus.h                  # Bus fleet data model & subroute extractors
│   ├── Graph.h                # Adjacency list graph & BFS/DFS algorithms
│   ├── BusManager.h           # Bus search & recommendation logic
│   ├── DataManager.h          # File reading/writing & data persistence
│   ├── UI.h                   # Visualizers, banners, and interactive prompts
│   └── App.h                  # Application menu controller
├── src/
│   ├── Common.cpp             # Windows ANSI initialization & sleep helpers
│   ├── Graph.cpp              # BFS and DFS core implementations
│   ├── BusManager.cpp         # Route analysis & bus ranking
│   ├── DataManager.cpp        # File I/O parser & default network builder
│   ├── UI.cpp                 # Screen rendering & ASCII route maps
│   ├── App.cpp                # Main menu and interactive workflows
│   └── main.cpp               # Program entry point
├── tests/
│   └── test_algorithms.cpp    # Automated test suite for all 7 algorithm requirements
├── build.bat                  # One-click Windows build and run batch file
├── Makefile                   # Make build script
└── README.md                  # Complete documentation and viva guide
```

---

## 🚀 How to Compile and Run

### Option 1: Quick Command Line (MinGW g++)

Open a terminal or PowerShell in the project directory and run:

```bash
g++ -std=c++17 -O2 -Wall -Wextra -Iinclude src/Common.cpp src/Graph.cpp src/BusManager.cpp src/DataManager.cpp src/UI.cpp src/App.cpp src/main.cpp -o TransitIQ.exe
.\TransitIQ.exe
```

### Option 2: Using the One-Click Batch Script (Windows)

Simply double-click `build.bat` or run in terminal:
```cmd
build.bat
```

### Option 3: In VS Code

1. Open the project folder `d:\New folder\Algo Project` in VS Code.
2. Press **`Ctrl + Shift + B`** to trigger the default build task (`Build TransitIQ (Release)`).
3. Open the integrated terminal (`Ctrl + ~`) and launch `./TransitIQ.exe`.
4. To run under the debugger, press **`F5`**.

### Option 4: Running the Automated Verification Tests

To verify all algorithms independently via assertions:
```bash
g++ -std=c++17 -Iinclude src/Common.cpp src/Graph.cpp src/BusManager.cpp src/DataManager.cpp src/UI.cpp tests/test_algorithms.cpp -o test_algorithms.exe
.\test_algorithms.exe
```

---

## 🧠 Algorithmic Explanations (Viva / CSE Interview Guide)

### Q1: Why use BFS for the Journey Finder instead of Dijkstra?
* **Answer**: BFS expands layer by layer ($0, 1, 2, \dots$ hops) using a First-In, First-Out (`std::queue`). In an unweighted graph where each edge represents 1 transit connection, BFS is mathematically guaranteed to find the route with the **minimum number of connections (stop hops)** in $O(V + E)$ time. Dijkstra is designed for weighted shortest paths (e.g., minimizing exact meters or minutes); using BFS keeps the solution clean, optimal for minimizing vehicle boarding hops, and computationally efficient.

### Q2: How does the "Minimum Bus Transfers" BFS model work?
* **Answer**: In our bus-transfer model, a transit state is defined at each bus stop. When expanding from a stop, we examine all bus lines passing through it. In a single BFS step, taking that bus allows us to reach any stop along that line. Because BFS searches level by level (level 0 = 0 buses, level 1 = 1 bus / direct, level 2 = 2 buses / 1 transfer), the first time the destination is popped from the queue, it is mathematically guaranteed to have the minimum number of bus boardings/transfers!

### Q3: How does DFS detect critical bridges and disconnected networks?
* **Answer**: DFS explores paths deeply using recursion or a call stack. By running DFS from all unvisited vertices, we partition the vertices into distinct connected components. When a road $(u, v)$ is simulated as closed:
  1. We count connected components before closing.
  2. We temporarily block $(u, v)$ and recount components.
  3. If the number of components increases, edge $(u, v)$ is a **bridge (bottleneck)**, meaning its failure partitions the city network into disconnected islands.

---

##  Dhaka Sample Transit Network

The simulator comes preloaded with 23 popular transit locations:
* **North Zone**: Uttara, Airport, Khilkhet
* **North-East & East Zone**: Kuril, Bashundhara, Badda, Rampura
* **Central & Business**: Banani, Gulshan-2, Gulshan-1, Mohakhali, Farmgate, Karwan Bazar, Moghbazar, Malibagh
* **West Zone**: Mirpur-10, Mirpur-1, Gabtoli, Dhanmondi
* **South Zone**: Shahbagh, New Market, Motijheel, Sayedabad

Preloaded bus lines include:
1. **BRTC Articulated** (AC Express: Uttara $\leftrightarrow$ Motijheel via Farmgate)
2. **Raida Enterprise** (Semi-Seating: Uttara $\leftrightarrow$ Sayedabad via Kuril, Badda, Rampura)
3. **Bikolpo Auto** (Regular: Mirpur-10 $\leftrightarrow$ Motijheel)
4. **Turag Transport** (Regular: Uttara $\leftrightarrow$ New Market)
5. **Anabil Super** (Regular: Kuril $\leftrightarrow$ Sayedabad)
6. **Alif Paribahan** (Semi-Seating: Mirpur-10 $\leftrightarrow$ Rampura via Gulshan)
7. **Shikor Paribahan** (Regular: Gabtoli $\leftrightarrow$ Motijheel via Dhanmondi)
8. **Winner City Service** (Regular: Mirpur-10 $\leftrightarrow$ Motijheel via Moghbazar)


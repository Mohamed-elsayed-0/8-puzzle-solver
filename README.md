# 🧩 8-Puzzle Solver

A C++ implementation of the classic **8-Puzzle problem** that compares uninformed and informed search algorithms, with both a console mode and an interactive SFML GUI.

---

## 📌 About the Project

The 8-puzzle is a sliding tile game on a 3×3 grid with 8 numbered tiles and one blank space. The goal is to reach a target configuration by sliding tiles into the blank space.

This project solves the puzzle using three search strategies and lets you compare their performance side by side.

---

## ✨ Features

- 🔍 **BFS** — Breadth-First Search (uninformed, guarantees shortest path)
- ⭐ **A\* Search** with three heuristics:
  - Misplaced Tiles
  - Manhattan Distance
  - Linear Conflict
- 📊 **Performance comparison table** (nodes expanded, time, path length)
- 🎮 **SFML GUI** — visualize the solution step by step
- 🎲 **Random puzzle generator** with solvability check

---

## 📁 Project Structure

```
8-puzzle-solver/
├── main.cpp         # Entry point (console + GUI mode)
├── Solver.h         # BFS and A* algorithm implementations
├── Puzzle.h         # Puzzle state, moves, and heuristics
├── Generator.h      # Random solvable puzzle generator
├── GUI.h            # SFML graphical interface
├── CMakeLIsts.txt   # CMake build configuration
└── .vscode/
    └── tasks.json   # VS Code build tasks
```

---

## 🛠️ Build Instructions

### Requirements
- C++17 or later
- [CMake](https://cmake.org/) 3.10+
- [SFML](https://www.sfml-dev.org/) 2.5+ (for GUI mode)

### Steps

```bash
# 1. Clone the repository
git clone https://github.com/YOUR_USERNAME/8-puzzle-solver.git
cd 8-puzzle-solver

# 2. Create a build directory
mkdir build && cd build

# 3. Configure with CMake
cmake ..

# 4. Build
cmake --build .

# 5. Run
./8PuzzleSolver
```

> **Note:** To run in console-only mode (no SFML required), remove the `-DUSE_GUI` definition from `CMakeLIsts.txt`.

---

## 📊 Sample Output

```
--------------------------------------------------
  =========== COMPARISON TABLE ===========

  Metric                BFS         A*(Manhattan)
--------------------------------------------------
  Solution length  :    20          20
  Nodes expanded   :    18543       512
  Nodes generated  :    21034       623
  Time             :    142.3 ms    3.1 ms
--------------------------------------------------
```

---

## 🧠 Algorithms

| Algorithm | Type | Optimal | Complete |
|---|---|---|---|
| BFS | Uninformed | ✅ Yes | ✅ Yes |
| A* (Misplaced Tiles) | Informed | ✅ Yes | ✅ Yes |
| A* (Manhattan Distance) | Informed | ✅ Yes | ✅ Yes |
| A* (Linear Conflict) | Informed | ✅ Yes | ✅ Yes |

---

## 📄 License

This project is for educational purposes.

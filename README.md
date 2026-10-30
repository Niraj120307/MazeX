# MAZE-X

**Interactive maze generation, pathfinding and DSA visualization platform** — C++17 + SFML 3, DSA course project.

MAZE-X turns a maze into a visible grid graph. You draw or generate a maze, pick DFS, BFS, Dijkstra or A*, and watch the search step by step — with the algorithm's **real** stack / queue / priority queue shown live beside it — then compare all four on the same maze.

Every walkable cell is a **vertex**; every legal Up/Right/Down/Left move is an **edge**. All four algorithms share one loop; only the data structure holding the frontier changes:

| Algorithm | Data structure | Guarantee | Complexity |
|---|---|---|---|
| DFS | Stack (`std::stack`, LIFO) | Finds *a* path, not necessarily the shortest | O(V + E) |
| BFS | Queue (`std::queue`, FIFO) | Fewest moves (ignores terrain cost) | O(V + E) |
| Dijkstra | Min-priority queue by g | Minimum total cost (all weights ≥ 1) | O((V + E) log V) |
| A* | Min-priority queue by f = g + h | Minimum total cost; usually explores fewer cells | O((V + E) log V) here |

A* heuristic: `h(n) = Manhattan(n, End) × minWeight`. Every move costs at least `minWeight`, so `h` never overestimates (admissible) and is consistent.

---

## Features

| ID | Requirement | Where |
|---|---|---|
| FR-01 | Grid with start, end, walls, walkable cells | `Maze`, GUI canvas |
| FR-02 | Draw / erase walls with the mouse | left-drag / right-drag |
| FR-03 | DFS and BFS with path reconstruction | `DFS.cpp`, `BFS.cpp`, `reconstructPath` |
| FR-04 | Animated explored cells + final path | `Playback` + `Renderer.cpp` |
| FR-05 | Play, pause, step, restart, finish, speed | bottom bar / Space, →, F, ↑↓ |
| FR-06 | Nodes explored, path length/cost, time | right panel |
| FR-07 | Recursive-backtracking generator | `MazeGenerator.cpp` (G) |
| FR-08 | Weighted terrain + Dijkstra | terrain brush (B), random terrain (T) |
| FR-09 | A* with documented heuristic | `AStar.cpp` |
| FR-10 | Live stack / queue / priority-queue panel | right panel, rebuilt from real events |
| FR-11 | Compare all algorithms on the same maze | Compare all (C) |
| FR-12 | Save / load maze layouts | Ctrl+S / Ctrl+L → `saves/maze.txt` |
| FR-13 | Batch benchmarks on several grid sizes → CSV | Bench CSV button, `mazex_console --bench` |
| extra | Random obstacles (always solvable), prepared demo maze, help overlay | O, `assets/mazes/demo.txt`, H |

---

## Build and run (Windows)

There are three programs: **MazeX.exe** (the graphical app), **mazex_console.exe** (console demo + benchmark) and **test_mazex.exe** (tests). The console app and tests need only g++; the GUI also needs **SFML 3**.

### Option A — MSYS2 UCRT64 (recommended; this is the g++ VS Code's C++ guide installs)

1. In the **MSYS2 UCRT64** terminal, install the compiler and SFML 3 once:
   ```
   pacman -S --needed mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-sfml
   ```
2. Make sure `C:\msys64\ucrt64\bin` is on your Windows PATH (`where g++` should show `C:\msys64\ucrt64\bin\g++.exe`).
3. In the VS Code terminal (Command Prompt), from the project folder:
   ```bat
   build_console.bat     :: console demo + tests (expects "159/159 checks passed")
   build_gui.bat         :: MazeX.exe
   MazeX.exe
   ```
   VS Code: **Terminal → Run Build Task** offers the same steps (`.vscode/tasks.json`).

### Option B — CMake (any MinGW or Visual Studio, no SFML install needed)

Needs CMake 3.24+ and Git. If SFML 3 isn't installed, CMake downloads SFML 3.0.2 and builds it with *your* compiler, so there is no compiler-version mismatch.
```bat
build_cmake.bat
build\MazeX.exe
```

### Option C — official SFML 3.0.2 zip

The prebuilt MinGW zip only works with **exactly** GCC 14.2.0 MinGW (SEH, UCRT) — "the compiler versions have to match 100%". If yours differs, use A or B. With the matching compiler:
```bat
g++ -std=c++17 -O2 -Iinclude -IC:\SFML-3.0.2\include src\ui\main_gui.cpp src\ui\App.cpp src\ui\Controls.cpp src\ui\Renderer.cpp src\ui\Widgets.cpp src\Maze.cpp src\MazeIO.cpp src\SolverResult.cpp src\DFS.cpp src\BFS.cpp src\Dijkstra.cpp src\AStar.cpp src\MazeGenerator.cpp src\Playback.cpp src\Benchmark.cpp src\ConsoleView.cpp -LC:\SFML-3.0.2\lib -lsfml-graphics -lsfml-window -lsfml-system -o MazeX.exe
```
then copy `sfml-graphics-3.dll`, `sfml-window-3.dll`, `sfml-system-3.dll` from `C:\SFML-3.0.2\bin` next to `MazeX.exe`.

Run MazeX.exe from the project folder (or anywhere with `assets/` beside it) so it finds the fonts and the demo maze.

### Linux / macOS
```
make test        # console tests
make gui         # needs SFML 3 (e.g. from your package manager or built from source)
cmake -S . -B build && cmake --build build -j   # alternative
```

---

## Using the app

| Action | Mouse / button | Key |
|---|---|---|
| Choose algorithm | DFS / BFS / Dijkstra / A* | 1 2 3 4 |
| Solve (runs once, then animates) | Solve | Enter |
| Play / pause | Play | Space |
| Step one event | Step | → or N |
| Jump to end / rewind | Finish / Restart | F |
| Faster / slower animation | + / − | ↑ / ↓ |
| Reset search (keeps maze) | Reset | R or Esc |
| Generate maze / obstacles / terrain | buttons | G / O / T |
| Grid size for Generate | − / + under MAZE | |
| Tools: wall, erase, start, end, terrain | EDIT TOOL buttons | W E S D B |
| Terrain brush cost 2–9 | − / + under the tool | [ and ] |
| Erase anything | right-drag | |
| Compare all four on this maze | Compare all | C |
| Save / load | Save / Load | Ctrl+S / Ctrl+L |
| Benchmark → CSV | Bench CSV | |
| Help | Help | H |

**Colours:** green S = start, red E = end, dark = wall, light = walkable, tan→brown = terrain cost 2→9, cyan = in the frontier (stack/queue/PQ), blue = explored, magenta = being expanded now, yellow + line = final path.

**Reset vs Clear:** *Reset* removes the search overlay and keeps your maze. *Clear walls* removes walls and terrain.

---

## Architecture

```
            SFML GUI  (src/ui)                         Core engine (include/, src/)  - no SFML
 ┌──────────────────────────────────────┐   ┌──────────────────────────────────────────────┐
 │ Controls.cpp  buttons, layout        │   │ Maze            grid, walls, S/E, terrain     │
 │ App.cpp       event + frame loop ────┼──►│ MazeGenerator   backtracker, obstacles, terrain│
 │               edits the Maze         │   │ Solvers         DFS  BFS  Dijkstra  A*         │
 │               Solve: copy of maze ───┼──►│   └─► SolverResult  path, events, metrics      │
 │ Renderer.cpp  draws grid + panels ◄──┼───│ Playback        replays events, rebuilds DS    │
 │ Widgets.cpp   text, buttons, quads   │   │ Benchmark       median timing, CSV             │
 └──────────────────────────────────────┘   │ MazeIO          text format, save/load         │
                                            └──────────────────────────────────────────────┘
```

**The key design decision:** a solver runs **once, at full speed**, on a *copy* of the maze, and records every event (`Push`, `Expand`, `Skip`) in its `SolverResult`. The GUI then **replays** those events. So:

- animation speed can never change correctness or the measured time;
- pause / step / restart only move an index;
- the live data-structure panel is rebuilt from the solver's own events, so it shows the **real** contents — the tests replay 480 searches and check every pop came from the top of the stack / front of the queue / minimum of the priority queue.

Permanent maze data (walls, terrain) is never mixed with search state (frontier, visited, path); that lives in `Playback` as an overlay.

```
include/                     src/                         src/ui/  (SFML 3)
  Cell.h        CellType, Position, Cell                    App.h / App.cpp   loop, editing, actions
  Maze.h        grid model                 Maze.cpp         Controls.cpp      button layout
  MazeIO.h      text <-> maze, save/load   MazeIO.cpp       Renderer.cpp      all drawing
  SolverResult.h result, events, helpers   SolverResult.cpp Widgets.h/.cpp    text, buttons, quads
  Solvers.h     4 solvers + registry       DFS BFS Dijkstra AStar .cpp        Theme.h  colours, sizes
  MazeGenerator.h                          MazeGenerator.cpp main_gui.cpp     entry point
  Playback.h    event replay               Playback.cpp
  Benchmark.h   timing, CSV                Benchmark.cpp
  ConsoleView.h terminal output            ConsoleView.cpp   main.cpp  console app
tests/  test_main.cpp  test_maze.cpp  test_solvers.cpp  test_engine.cpp  TestUtil.h
assets/ fonts/ (Inter, JetBrains Mono - SIL OFL)   mazes/demo.txt (prepared demo maze)
docs/   DEMO_SCRIPT.md   VIVA_NOTES.md
```

---

## Definitions (used consistently everywhere)

- **Neighbour order:** Up, Right, Down, Left — runs are repeatable and comparable.
- **Path length:** number of moves = path cells − 1.
- **Path cost:** sum of the weights of every cell *entered* (Start is not entered). Unweighted: cost = moves.
- **Nodes explored:** cells taken out of the container and expanded (stale duplicates are skipped, not counted).
- **Max frontier:** the largest size the stack / queue / PQ reached (memory use).
- **Time:** solver only, no drawing; the GUI shows the median of 5 runs, Compare uses 7, the benchmark 5. Small mazes give noisy times — compare explored cells and cost first; never claim one algorithm is always fastest.
- **Terrain:** cost 1–9 per cell (Dijkstra needs non-negative weights; we require ≥ 1).

## Implementation notes

- **BFS** marks a cell discovered when it is *enqueued*, so a cell is never queued twice.
- **DFS** uses an explicit `std::stack` (no recursion-depth limit on big mazes); neighbours are pushed in reverse so Up is explored first. A cell is marked visited when popped.
- **Dijkstra / A\*** use *lazy deletion*: `std::priority_queue` can't decrease a key, so a cheaper route pushes a new entry and stale entries are skipped when popped. Ties break by insertion order (A*: smaller h first), so results are deterministic.
- **Parent array:** `parent[i]` = flat index (`row * cols + col`) of the cell that discovered `i`; the path is rebuilt from End back to Start, then reversed.
- **Recursive backtracking** is written iteratively with an explicit stack; rooms sit on even coordinates, walls between them are knocked down. With 0% extra openings it produces a *perfect maze* (a spanning tree — one route between any two rooms); the GUI adds 12% extra openings so BFS and DFS differ visibly.

## Maze text format (save/load, tests)

```
S..#5      #  wall        .  empty (cost 1)
.#2..      S  start       E  end
...#E      2-9  terrain cost
```

## Testing — 159 checks (`test_mazex.exe -q`)

- Maze rules, indexing, terrain, text I/O and file save/load.
- The plan's cases: direct corridor, forced detour, blocked destination, start walled in, adjacent S/E, weighted alternative, DFS-not-shortest, BFS ring order, A* explores fewer cells than Dijkstra on an open grid.
- **400 random mazes** (half weighted) against an independent Bellman-Ford oracle: every path legal, BFS = fewest moves, Dijkstra = A* = minimum cost, all four agree on reachability.
- Generator: 40 mazes of many sizes are solvable, fully connected and (with 0% loops) perfect; same seed → same maze.
- Playback: 480 replays with zero out-of-order pops; panel order checks for stack, queue and PQ.
- Benchmark: comparison on one snapshot, CSV rows and header.

The GUI code is compile-checked against the SFML 3.0.2 headers; it is tested by running it (see `docs/DEMO_SCRIPT.md`).

## Benchmark sample (`mazex_console --bench`)

Median of 5 runs, 81 × 81, one machine (your numbers will differ):

| Maze | Algorithm | Explored | Moves | Cost |
|---|---|---|---|---|
| terrain | BFS | 3761 | 172 | 565 |
| terrain | Dijkstra | 3761 | 196 | **456** |
| terrain | A* | 3752 | 196 | **456** |

On heavy terrain A* barely beats Dijkstra: its heuristic is scaled by the cheapest step (cost 1), which is very optimistic, so it gives little guidance — while on an open unweighted grid A* explores a fraction of what Dijkstra does. That trade-off is the honest result, not a bug.

## Known limitations

- 4-direction movement only (no diagonals).
- One save slot (`saves/maze.txt`); edit the file by hand for more mazes — it's plain text.
- The window is not DPI-aware; on 125–150 % Windows scaling it is bitmap-scaled (slightly soft text).
- Stretch items from the plan (race mode, challenge mode, themes, Prim/Kruskal generators) are not included.

## Credits

Fonts: [Inter](https://github.com/rsms/inter) and [JetBrains Mono](https://github.com/JetBrains/JetBrainsMono), SIL Open Font License 1.1 (licences in `assets/fonts/`). Graphics: [SFML 3](https://www.sfml-dev.org/), zlib licence.

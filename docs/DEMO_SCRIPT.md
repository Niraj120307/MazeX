# MAZE-X — Demo script (5–6 minutes)

Start `MazeX.exe`. It opens the **prepared demo maze** (`assets/mazes/demo.txt`, 21 × 35 with terrain), so the demo never depends on random output. Expected numbers on that maze:

| Algorithm | Explored | Moves | Cost |
|---|---|---|---|
| DFS | 330 | 258 | 906 |
| BFS | 413 | 66 | 224 |
| Dijkstra | 350 | 74 | 175 |
| A* | 326 | 74 | 175 |

Before presenting: set the speed to about **160 steps/s** (↑/↓).

| # | Do | Say |
|---|---|---|
| 1 | Point at the canvas, the legend (bottom right) and the controls. | "The maze is a graph: every light cell is a vertex, every legal Up/Right/Down/Left move is an edge. Dark cells are walls, brown cells cost more to enter." |
| 2 | Pick **Wall** (W), draw a short wall; pick **Start** (S) and drag the start one cell; right-drag to erase. Put it back. | "Edits go through the Maze class, which validates them — you can't wall over Start or End, or put them on the same cell." |
| 3 | Press **1** (DFS), **Enter**. After a second press **Space** to pause and **→** a few times. Point at the STACK panel. | "DFS uses a stack. The top entry — highlighted — is popped next, so it keeps going deeper. Each step here is a real push or pop from the solver." Let it finish: "258 moves — DFS finds *a* path, not the shortest." |
| 4 | **R** to reset, **2** (BFS), **Enter**. Point at the QUEUE panel and the cyan ring. | "BFS uses a FIFO queue, so it spreads in rings — first everything 1 move away, then 2… The first time it reaches E is with the fewest moves: 66. But look at the cost: 224 — it walked straight through expensive terrain, because BFS ignores weights." |
| 5 | **R**, **3** (Dijkstra), **Enter**, then **R**, **4** (A*), **Enter**. Point at the PRIORITY QUEUE panel with g / f values. | "Dijkstra's priority queue always pops the cheapest cell so far (g). It takes 74 moves but costs only 175 — it goes around the expensive terrain. A* adds h = Manhattan distance × the cheapest step, so it's pulled toward E: same optimal cost, fewer cells explored." |
| 6 | Press **C** (Compare all). | "All four ran on an identical copy of this maze, same neighbour order. Green cost = optimal. Time is the median of 7 runs and only the solver — no drawing — and on mazes this small it's noisy, so I compare explored cells and cost first." |
| 7 | Close it, set size with − / + under MAZE, press **G** (Generate). Run BFS once. Optionally **Ctrl+S**, change the maze, **Ctrl+L**. | "Recursive backtracking: an explicit stack carves from room to room and backtracks at dead ends — the same DFS idea used to *build* the maze. It's always solvable. Save/load writes a plain text file." |

**If something goes wrong:** R resets the search, Ctrl+L reloads the last save, restarting the app reloads the demo maze.

**Optional extras (if asked):** `mazex_console.exe --bench` writes `benchmarks/results.csv`; `test_mazex.exe -q` runs the 159 checks, including 400 random mazes against a Bellman-Ford referee.

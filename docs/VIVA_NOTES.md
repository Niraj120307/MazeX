# MAZE-X — Viva notes

## Graph basics

**Why can a grid maze be modelled as a graph?** Each walkable cell is a vertex; each legal move to an adjacent walkable cell is an edge. `Maze::getNeighbors()` returns exactly the edges leaving a cell, in the fixed order Up, Right, Down, Left.

**V and E for an R × C grid?** V ≤ R·C. Each cell has at most 4 neighbours, so E ≤ 4V — that's why O(V + E) = O(V) here.

**How are cells stored for the solvers?** Flat index `row * cols + col`, so `visited`, `parent`, `dist` are plain `std::vector`s instead of maps — O(1) access and cache friendly.

## The four algorithms

**What is the one difference between them?** The container that holds cells waiting to be explored: stack (DFS), queue (BFS), min-priority queue by g (Dijkstra), min-priority queue by f = g + h (A*). The loop is the same.

**Why does BFS give the shortest path?** The FIFO queue processes cells in order of increasing distance; the first time it reaches the end, it has used the fewest moves. Only true when every move costs the same.

**Why mark BFS cells when enqueued, not dequeued?** Otherwise the same cell can be added several times by different neighbours before it is processed — more memory and wasted work.

**Why doesn't DFS guarantee the shortest path?** It commits deep along one branch and returns the first path it finds. On the demo maze it takes 258 moves where BFS takes 66.

**Why an explicit stack instead of recursion?** A 61 × 101 open maze could recurse thousands of levels and overflow the call stack. An explicit `std::stack` also lets us show the stack in the UI.

**Why does Dijkstra need non-negative weights?** It finalizes the cheapest cell first. A negative edge could later make an already-finalized cell cheaper. Our terrain costs are 1–9.

**What is "lazy deletion"?** `std::priority_queue` cannot decrease a key. When a cheaper route to a cell is found we push it again; when an old (stale) entry is popped later, we skip it (`settled` check). Those skips appear as "stale, skip" in the step log.

**What does f(n) = g(n) + h(n) mean?** g = real cost from Start to n; h = estimated remaining cost to End; A* expands the smallest f.

**Why is Manhattan distance admissible?** With 4-direction moves you need at least |dr| + |dc| moves. With terrain, each move costs at least `minWeight`, so we use `h = Manhattan × minWeight`. It never overestimates → A* returns the optimal cost. It is also *consistent* (h drops by at most one step's cost per move), so a settled cell never needs re-opening.

**Why does A* barely beat Dijkstra on heavy terrain?** h is scaled by the cheapest step (cost 1), so on terrain where most steps cost 3–9 the estimate is far below the real cost and gives little guidance. On an open unweighted grid it explores far fewer cells. Tighter heuristics would need to stay admissible — that's the trade-off.

**Complexities?** DFS and BFS O(V + E). Dijkstra and A* with a binary heap O((V + E) log V); with lazy deletion the heap can hold up to O(E) entries, still O(E log V). Space O(V) for visited/parent/dist (+ the container).

**What does the parent array do?** `parent[i]` = the cell that discovered cell i. Walking it from End back to Start and reversing gives the path. If End was never discovered, there's no path.

## Design

**Why does the solver take the maze by value?** Every algorithm gets an identical snapshot; none can change the user's maze; comparisons are fair.

**How does the animation work without slowing the solver?** The solver runs once at full speed and logs every Push / Expand / Skip. The UI replays the log at the chosen speed. Speed affects only the replay, never correctness or the measured time.

**How do you know the stack/queue panel is real?** `Playback` rebuilds the container from the logged events. Tests replay 480 searches and verify every pop came from the top of the stack, the front of the queue, or the minimum of the priority queue — zero violations.

**Why separate .h and .cpp?** The header is the interface other files depend on; the .cpp hides the implementation and compiles independently.

**Why separate the GUI from the solvers?** Solvers are tested in the console, reused by the console app, benchmark and GUI, and contain no SFML code.

**How does the UI stay responsive?** One frame loop: handle events → `update(dt)` plays as many events as the speed allows this frame → draw. No `sleep`, so input is handled every frame.

**How is the grid drawn quickly?** All cells go into one `sf::VertexArray` (two triangles per cell) — one draw call per frame instead of thousands of rectangles.

## Generation

**How does recursive backtracking work?** Rooms are cells at even coordinates; everything else starts as wall. From the Start room, pick a random unvisited room two cells away, knock down the wall between, push it. At a dead end, pop (backtrack). It ends when the stack is empty.

**Why is the generated maze always solvable?** Every room is visited, and each visit knocks a wall from an already-connected room, so all rooms are connected — a spanning tree ("perfect maze": exactly one route between any two rooms). The tests check: open cells = 2 × rooms − 1.

**Why add extra openings?** A perfect maze has only one route, so BFS and DFS would always find the same path. Opening ~12% of the remaining inner walls creates loops, so the algorithms differ.

**How do random obstacles stay solvable?** After placing walls we run a BFS reachability check and retry with a new layout if End isn't reachable.

## Fair measurement

**How do you keep comparisons fair?** Same maze copy, same neighbour order, same definitions of path length/cost, timing of the solver only, median of repeated runs, and nodes explored + cost reported alongside time.

**Why not just report time?** On small mazes time is a few microseconds and dominated by noise; explored cells and path cost are exact and repeatable.

## Testing

**How do you know the algorithms are correct?** 159 checks, including 400 random mazes (half weighted) compared with an independent Bellman-Ford implementation: all paths legal, BFS = fewest moves, Dijkstra = A* = minimum cost, all agree on whether a path exists.

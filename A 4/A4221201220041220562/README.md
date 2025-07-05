# Assignment 4 – Routing Protocols (DVR & LSR)

## Overview

This assignment implements two fundamental routing algorithms:

- **Distance Vector Routing (DVR)**
- **Link State Routing (LSR)**

The goal is to simulate these algorithms using an adjacency matrix representing the network's topology. This simulation helps understand how routers discover and update optimal paths to every other router in the network.

---

## Files Included

- `routing_sim.cpp` – The main simulation source file.
- `input#n.txt` – Sample input files to test the simulation n -- 1,2,3,4.
- `README.md` – Instructions on how to compile and run the simulation.

---

## How to Compile and Run

### 1. **Compile**

Make sure you have a C++ compiler installed (like `g++`), then run:

```bash
g++ routing_sim.cpp -o routing_sim
```

This will generate an executable named `routing_sim`.

---

### 2. **Run**

```bash
./routing_sim input1.txt
```

Here, `input1.txt` is a sample input file containing the network’s adjacency matrix.

---

## Input Format

The input file should follow this format:

```
n
a11 a12 a13 ... a1n
a21 a22 a23 ... a2n
...
an1 an2 an3 ... ann
```

Where:

- `n` is the number of nodes in the network.
- `aij` is the cost of the link between node `i` and node `j`.
- A value of `0` (except on the diagonal) means **no direct link** and is treated as infinity (`9999`) internally.
- Self-links (`i==j`) are always `0`.

### Example – `input1.txt`

```
4
0 10 100 30
10 0 20 40
100 20 0 10
30 40 10 0
```

---

## Output

The simulation prints the routing tables for each node after running both algorithms.

### Sample Output:

```
--- Distance Vector Routing Simulation ---
--- DVR Final Tables ---
Node 0 Routing Table:
Dest    Cost    Next Hop
0       0       -
1       10      1
2       30      1
3       30      3

Node 1 Routing Table:
Dest    Cost    Next Hop
0       10      0
1       0       -
2       20      2
3       30      2

Node 2 Routing Table:
Dest    Cost    Next Hop
0       30      1
1       20      1
2       0       -
3       10      3

Node 3 Routing Table:
Dest    Cost    Next Hop
0       30      0
1       30      2
2       10      2
3       0       -

--- Link State Routing Simulation ---
Node 0 Routing Table:
Dest    Cost    Next Hop
1       10      1
2       30      1
3       30      3

Node 1 Routing Table:
Dest    Cost    Next Hop
0       10      0
2       20      2
3       30      2

Node 2 Routing Table:
Dest    Cost    Next Hop
0       30      1
1       20      1
3       10      3

Node 3 Routing Table:
Dest    Cost    Next Hop
0       30      0
1       30      2
2       10      2
```

---

## Notes

- Make sure your input files are correctly formatted.
- Infinite cost is represented internally as `9999`.
- The routing tables show the **cost** and **next hop** for reaching each destination from every node.

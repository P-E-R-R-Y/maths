# maths

> Geometry, graphs and interpolation.

Header-only, no dependency.

- `geometry.hpp` — triangulation, Delaunay
- `graph.hpp` — minimum spanning tree, BFS, connected components
- `interpolation.hpp` — linear and cubic

```cpp
#include "graph.hpp"

Graph<int> g;
g.addEdge(0, 1, 2.f);
auto tree = g.mst();
```

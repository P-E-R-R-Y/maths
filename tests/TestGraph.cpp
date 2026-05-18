#include <gtest/gtest.h>
#include <algorithm>
#include <cmath>
#include "graph.hpp"
#include "Lambdify.hpp"

// stateless wrappers via lambdify_t
constexpr bool   eqInt(int a, int b)   { return a == b; }
constexpr double diffAbs(int a, int b) { return std::abs(double(a - b)); }

using EqInt   = lambdify_t<eqInt>;
using DiffAbs = lambdify_t<diffAbs>;

// ----------- fromPairs + node accessors -----------

TEST(GraphTest, FromPairsDeduplicatesAndStoresNodes) {
    auto g = Graph<int, EqInt>::fromPairs({{1,2}, {2,3}});
    EXPECT_EQ(g.nodeCount(), 3u);
    EXPECT_EQ(g.node(0), 1);
    EXPECT_EQ(g.node(1), 2);
    EXPECT_EQ(g.node(2), 3);
    EXPECT_EQ(g.edgeCount(), 2u);
}

TEST(GraphTest, FromPairsWithoutEqualKeepsDuplicates) {
    auto g = Graph<int>::fromPairs({{1,2}, {2,3}});
    EXPECT_EQ(g.nodeCount(), 4u); // no dedup → 2 appears twice
}

// ----------- mutation API -----------

TEST(GraphTest, IncrementalConstructionWorks) {
    Graph<int, EqInt> g;
    int a = g.addNode(10);
    int b = g.addNode(20);
    int c = g.addNode(10); // dup → returns a
    EXPECT_EQ(a, c);
    EXPECT_EQ(g.nodeCount(), 2u);
    g.addEdge(a, b, 1.5);
    EXPECT_EQ(g.edgeCount(), 1u);
    EXPECT_DOUBLE_EQ(g.edges()[0].weight, 1.5);
}

// ----------- neighbors / successors / predecessors -----------

TEST(GraphTest, UndirectedNeighborsAreBidirectional) {
    auto g = Graph<int, EqInt>::fromPairs({{0,1}, {0,2}, {1,2}});
    EXPECT_EQ(g.neighbors(0).size(), 2u);
    EXPECT_EQ(g.neighbors(1).size(), 2u);
    EXPECT_EQ(g.neighbors(2).size(), 2u);
}

TEST(GraphTest, DirectedSuccessorsAndPredecessors) {
    using G = Graph<int, EqInt, GraphKind::Directed>;
    auto g = G::fromPairs({{0,1}, {0,2}, {1,2}});

    auto n0 = g.successors(0);
    ASSERT_EQ(n0.size(), 2u);
    EXPECT_EQ(n0[0], 1);
    EXPECT_EQ(n0[1], 2);

    EXPECT_TRUE(g.successors(2).empty());
    EXPECT_EQ(g.predecessors(2).size(), 2u); // 0 and 1 both point at 2
}

// ----------- MST -----------

TEST(GraphTest, MSTIsNonMutatingAndProducesNMinus1Edges) {
    auto g = Graph<int, EqInt>::fromPairs<DiffAbs>({
        {0,1}, {0,3}, {1,2}, {2,3}, {0,2}
    });
    auto mst = g.mst();
    EXPECT_EQ(g.edgeCount(), 5u);     // original untouched
    EXPECT_EQ(mst.edgeCount(), 3u);   // 4 nodes → tree has 3 edges
    EXPECT_EQ(mst.nodeCount(), g.nodeCount());

    // all nodes covered
    auto comps = mst.connectedComponents();
    EXPECT_EQ(comps.size(), 1u);
}

TEST(GraphTest, MSTOnEmptyGraphIsNoOp) {
    auto g = Graph<int>::fromPairs({});
    auto mst = g.mst();
    EXPECT_EQ(mst.nodeCount(), 0u);
    EXPECT_EQ(mst.edgeCount(), 0u);
}

TEST(GraphTest, MSTPicksLightestEdges) {
    // path graph 0-1-2-3 with weights, plus heavier chord
    auto g = Graph<int, EqInt>::fromPairs<DiffAbs>({
        {0,1}, {1,2}, {2,3}, {0,3}
    });
    auto mst = g.mst();
    // diffAbs gives weights 1,1,1,3 — MST should keep the three weight-1 edges
    double total = 0;
    for (const auto& e : mst.edges()) total += e.weight;
    EXPECT_DOUBLE_EQ(total, 3.0);
}

// ----------- difference (TinyKeep loopback pattern) -----------

TEST(GraphTest, DifferenceGivesLoopbackCandidates) {
    auto g   = Graph<int, EqInt>::fromPairs<DiffAbs>({
        {0,1}, {1,2}, {2,3}, {0,3}, {0,2}
    });
    auto mst  = g.mst();
    auto rest = g.difference(mst);
    EXPECT_EQ(mst.edgeCount() + rest.edgeCount(), g.edgeCount());
}

TEST(GraphTest, DifferenceTreatsUndirectedEdgesSymmetrically) {
    Graph<int, EqInt> g;
    g.addNode(0); g.addNode(1);
    g.addEdge(0, 1);

    Graph<int, EqInt> other;
    other.addNode(0); other.addNode(1);
    other.addEdge(1, 0); // reversed → same undirected edge

    EXPECT_EQ(g.difference(other).edgeCount(), 0u);
}

// ----------- connected components -----------

TEST(GraphTest, ConnectedComponentsSplitsDisjointSubgraphs) {
    Graph<int, EqInt> g;
    for (int i = 0; i < 5; ++i) g.addNode(i);
    g.addEdge(0, 1);
    g.addEdge(1, 2);
    g.addEdge(3, 4);
    auto comps = g.connectedComponents();
    EXPECT_EQ(comps.size(), 2u);
}

// ----------- BFS -----------

TEST(GraphTest, BFSVisitsAllReachableNodes) {
    auto g = Graph<int, EqInt>::fromPairs({{0,1}, {0,2}, {1,3}, {2,3}});
    auto order = g.bfs(0);
    EXPECT_EQ(order.size(), 4u);
    EXPECT_EQ(order.front(), 0);
}

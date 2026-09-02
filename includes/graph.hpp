/**
 * @file graph.hpp
 * @author Perry Chouteau
 * @date 2025-11-05
 * @brief Generic graph with optional node dedup, optional edge weighting,
 *        directed/undirected modes, and common algorithms (MST, components, BFS).
 *
 * Usage examples
 * --------------
 *
 *   // 1) Plain integer graph (undirected by default)
 *   auto g = Graph<int>::fromPairs({{0,1},{1,2},{2,0}});
 *
 *   // 2) With node deduplication
 *   struct EqInt { bool operator()(int a, int b) const { return a == b; } };
 *   auto g = Graph<int, EqInt>::fromPairs({{1,2},{2,3}});
 *
 *   // 3) With weighted edges (e.g. Vector2f points + euclidean distance)
 *   struct EqV2   { bool  operator()(Vector2f a, Vector2f b) const {...}; };
 *   struct DistV2 { float operator()(Vector2f a, Vector2f b) const {...}; };
 *   auto g    = Graph<Vector2f, EqV2>::fromPairs<DistV2>(pairs);
 *   auto mst  = g.mst();                  // non-mutating
 *   auto loop = g.difference(mst);        // remaining Delaunay edges → add back N for TinyKeep loops
 *
 * @addtogroup maths
 * @{
 */

#pragma once

#include <algorithm>
#include <concepts>
#include <cstddef>
#include <map>
#include <numeric>
#include <queue>
#include <set>
#include <type_traits>
#include <utility>
#include <vector>

// ---------- concepts ----------

template<typename F, typename T>
concept EqualityFn = std::is_invocable_r_v<bool, F, const T&, const T&>;

template<typename F, typename T>
concept WeightFn = std::is_invocable_v<F, const T&, const T&>
                && std::is_arithmetic_v<std::invoke_result_t<F, const T&, const T&>>;

// ---------- edge ----------

struct Link {
    int from = 0;
    int to = 0;
    double weight = 0.0;
};

// ---------- graph kind ----------

enum class GraphKind { Directed, Undirected };

// ---------- graph ----------

template<typename T, typename Equal = void, GraphKind Kind = GraphKind::Undirected>
class Graph {
public:
    Graph() = default;

    // ----- mutation -----

    // Add a node and return its index. If Equal is provided and an equivalent
    // node already exists, returns the existing index instead.
    int addNode(const T& value) {
        if constexpr (!std::is_void_v<Equal>) {
            static_assert(EqualityFn<Equal, T>,
                "Equal must be callable as bool(const T&, const T&)");
            Equal eq{};
            for (std::size_t i = 0; i < nodes_.size(); ++i) {
                if (eq(nodes_[i], value)) return static_cast<int>(i);
            }
        }
        nodes_.push_back(value);
        return static_cast<int>(nodes_.size() - 1);
    }

    void addEdge(int from, int to, double weight = 0.0) {
        edges_.push_back({from, to, weight});
    }

    void clear() { nodes_.clear(); edges_.clear(); }

    void reserveNodes(std::size_t n) { nodes_.reserve(n); }
    void reserveEdges(std::size_t n) { edges_.reserve(n); }

    // ----- accessors -----

    const std::vector<T>&    nodes() const { return nodes_; }
    const std::vector<Link>& edges() const { return edges_; }
    const T&                 node(int i) const { return nodes_[i]; }
    std::size_t              nodeCount() const { return nodes_.size(); }
    std::size_t              edgeCount() const { return edges_.size(); }
    bool                     empty() const { return nodes_.empty(); }
    static constexpr bool    isDirected() { return Kind == GraphKind::Directed; }

    // ----- queries -----

    // All nodes incident to `idx`. Honors the graph kind: undirected graphs
    // return both predecessors and successors.
    std::vector<int> neighbors(int idx) const {
        std::vector<int> result;
        for (const auto& e : edges_) {
            if (e.from == idx) {
                result.push_back(e.to);
            } else if constexpr (Kind == GraphKind::Undirected) {
                if (e.to == idx) result.push_back(e.from);
            }
        }
        return result;
    }

    // Always returns outgoing targets, regardless of Kind.
    std::vector<int> successors(int idx) const {
        std::vector<int> result;
        for (const auto& e : edges_)
            if (e.from == idx) result.push_back(e.to);
        return result;
    }

    // Always returns incoming sources, regardless of Kind.
    std::vector<int> predecessors(int idx) const {
        std::vector<int> result;
        for (const auto& e : edges_)
            if (e.to == idx) result.push_back(e.from);
        return result;
    }

    // ----- factories -----

    // Build a graph from a list of (T, T) pairs. Optionally provide a Weight
    // functor to compute edge weights from endpoint data.
    template<typename Weight = void>
    static Graph fromPairs(const std::vector<std::pair<T, T>>& pairs) {
        if constexpr (!std::is_void_v<Weight>) {
            static_assert(WeightFn<Weight, T>,
                "Weight must be callable as arithmetic(const T&, const T&)");
        }
        Graph g;
        g.reserveEdges(pairs.size());
        for (const auto& p : pairs) {
            int from = g.addNode(p.first);
            int to   = g.addNode(p.second);
            double w = 0.0;
            if constexpr (!std::is_void_v<Weight>) {
                Weight weight{};
                w = static_cast<double>(weight(p.first, p.second));
            }
            g.addEdge(from, to, w);
        }
        return g;
    }

    // ----- algorithms (non-mutating, return new graphs) -----

    // Minimum Spanning Tree via Kruskal. Returns a new graph sharing the same
    // node indexing as `*this`, containing only the MST edges. The original
    // is untouched, so caller can compute `difference(*this, mst)` for loopback.
    Graph mst() const {
        Graph result;
        result.nodes_ = nodes_;
        if (nodes_.empty() || edges_.empty()) return result;

        std::vector<Link> sorted = edges_;
        std::sort(sorted.begin(), sorted.end(),
                  [](const Link& a, const Link& b) { return a.weight < b.weight; });

        std::vector<int> parent(nodes_.size());
        std::vector<int> rank_(nodes_.size(), 0);
        std::iota(parent.begin(), parent.end(), 0);

        auto find = [&](int x) {
            while (x != parent[x]) {
                parent[x] = parent[parent[x]]; // path halving
                x = parent[x];
            }
            return x;
        };
        auto unite = [&](int a, int b) -> bool {
            int ra = find(a), rb = find(b);
            if (ra == rb) return false;
            if (rank_[ra] < rank_[rb]) std::swap(ra, rb);
            parent[rb] = ra;
            if (rank_[ra] == rank_[rb]) ++rank_[ra];
            return true;
        };

        result.reserveEdges(nodes_.size() - 1);
        for (const auto& e : sorted) {
            if (unite(e.from, e.to)) result.edges_.push_back(e);
        }
        return result;
    }

    // Edges in *this that are NOT in `other`. Useful for TinyKeep-style
    // "Delaunay minus MST" to recover loopback candidates.
    // Both graphs must share the same node indexing.
    Graph difference(const Graph& other) const {
        Graph result;
        result.nodes_ = nodes_;

        std::set<std::pair<int,int>> keys;
        for (const auto& e : other.edges_) keys.insert(edgeKey(e));

        for (const auto& e : edges_) {
            if (!keys.count(edgeKey(e))) result.edges_.push_back(e);
        }
        return result;
    }

    // Union of edge sets (does not deduplicate parallel edges, just concatenates).
    Graph unionWith(const Graph& other) const {
        Graph result = *this;
        result.edges_.insert(result.edges_.end(),
                             other.edges_.begin(), other.edges_.end());
        return result;
    }

    // Connected components as lists of node indices. Honors graph kind: in
    // directed mode this is *weakly* connected components.
    std::vector<std::vector<int>> connectedComponents() const {
        std::vector<int> parent(nodes_.size());
        std::iota(parent.begin(), parent.end(), 0);
        auto find = [&](int x) {
            while (x != parent[x]) { parent[x] = parent[parent[x]]; x = parent[x]; }
            return x;
        };
        for (const auto& e : edges_) {
            int ra = find(e.from), rb = find(e.to);
            if (ra != rb) parent[ra] = rb;
        }
        std::map<int, std::vector<int>> groups;
        for (std::size_t i = 0; i < nodes_.size(); ++i)
            groups[find(static_cast<int>(i))].push_back(static_cast<int>(i));

        std::vector<std::vector<int>> result;
        result.reserve(groups.size());
        for (auto& [_, v] : groups) result.push_back(std::move(v));
        return result;
    }

    // Breadth-first visit order starting from `start`. Honors graph kind.
    std::vector<int> bfs(int start) const {
        std::vector<int> order;
        if (nodes_.empty()) return order;
        std::vector<bool> seen(nodes_.size(), false);
        std::queue<int> q;
        q.push(start);
        seen[start] = true;
        while (!q.empty()) {
            int x = q.front(); q.pop();
            order.push_back(x);
            for (int n : neighbors(x))
                if (!seen[n]) { seen[n] = true; q.push(n); }
        }
        return order;
    }

private:
    std::vector<T>    nodes_;
    std::vector<Link> edges_;

    static std::pair<int,int> edgeKey(const Link& e) {
        if constexpr (Kind == GraphKind::Undirected) {
            return {std::min(e.from, e.to), std::max(e.from, e.to)};
        } else {
            return {e.from, e.to};
        }
    }
};
/** @} */



#pragma once
#include <stack>
#include <queue>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <ostream>

struct PureGraph 
{
    template<bool DoPred>
    struct EdgeIterator 
    {
        using ThisType = EdgeIterator<DoPred>;
    public:
        EdgeIterator(PureGraph const& graph, size_t state, bool isEndIterator) 
            : graph(graph), state(state), out(0) {
                if(isEndIterator) out = graph.nodeCount();
                else {
                    out = (hasConnection(0) ? 0 : findNextOut());
                }
        }

        bool operator==(ThisType const& other) const { return other.state == state && other.out == out; }
        ThisType operator++(int) { auto temp = *this; out = findNextOut(); return temp; }
        ThisType& operator++() { out = findNextOut(); return *this;}
        size_t operator*() const { return out; }
    private:
        bool hasConnection(size_t other) const {
            if constexpr (DoPred) return graph.hasEdge(other, state);
            else return graph.hasEdge(state, other);
        }
        size_t findNextOut() const {
            for(size_t i = out + 1; i < graph.nodeCount(); i++) {
                if(hasConnection(i)) return i;
            }
            return graph.nodeCount();
        }
        PureGraph const& graph;
        size_t state, out;
    };
    template<bool DoPred>
    struct EdgeProxy {
        EdgeProxy(PureGraph const& graph, size_t state) 
            : graph(graph), state(state){}

        auto begin() const { return EdgeIterator<DoPred>(graph, state, false);}
        auto end() const { return EdgeIterator<DoPred>(graph, state, true);}
    private:
        PureGraph const& graph;
        size_t state;
    };

    using PredProxy = EdgeProxy<true>;
    using SuccProxy = EdgeProxy<false>;

    static PureGraph trivialGraph(size_t totalNodes) {
        return PureGraph(totalNodes);
    }
    static PureGraph fullyConnected(size_t totalNodes) {
        auto temp = PureGraph(totalNodes);
        for(auto& row : temp.edges) {
            for (auto edge : row) {
                ++temp.totalEdges;
                edge = true;
            }
        } 
        return temp;
    }

    size_t createNode() {
        totalNodes++;
        //increase table size
        for(auto& src : edges) {src.push_back(false);}
        edges.emplace_back(std::vector<bool>(edges.size() + 1));
        return totalNodes - 1;
    }
    size_t predecessorCount(size_t node) const { 
        size_t count = 0; 
        for (size_t i = 0; i < nodeCount(); ++i)
            if (hasEdge(i, node)) count++;
        return count;
    }
    size_t successorCount(size_t node) const {
        return std::count(edges.at(node).begin(), edges.at(node).end(), true);
    }
    size_t edgeCount() const { return totalEdges; }
    size_t nodeCount() const { return totalNodes; }
    void removeAllEdges(size_t src) { for (auto succ : out(src)) removeEdge(src, succ); }
    void removeEdge(size_t src, size_t dst) { if (hasEdge(src, dst)) totalEdges--; edges[src][dst] = false; }
    void addEdge(size_t src, size_t dst) { if (!hasEdge(src, dst)) totalEdges++; edges[src][dst] = true; }
    bool hasEdge(size_t src, size_t dst) const { return edges[src][dst]; }
    SuccProxy out(size_t state) const { return SuccProxy{*this, state}; }
    PredProxy in(size_t state) const { return PredProxy{*this, state}; }
    bool operator==(PureGraph const& other) const {
        return other.totalNodes == totalNodes && other.edges == edges;
    }

    template<typename Callable>
    void bfs(size_t startNode, Callable callable) const {
        std::queue<size_t> worklist; worklist.push(startNode);
        std::unordered_set<size_t> visited; visited.insert(startNode);
        while(!worklist.empty()){
            auto n = worklist.front();
            callable(n);
            worklist.pop();
            for(auto succ : out(n)){
                if(visited.count(succ) == 0){
                    visited.insert(succ);
                    worklist.push(succ);
                }
            }
        } 
    }

    template<typename Callable>
    void dfs(size_t startNode, Callable callable) const {
        std::stack<size_t> worklist, order; worklist.push(startNode);
        std::unordered_set<size_t> visited; visited.insert(startNode);
        while (!worklist.empty()) {
            auto n = worklist.top();
            worklist.pop();
            for (auto succ : out(n)) {
                if (visited.count(succ) == 0) {
                    visited.insert(succ);
                    worklist.push(succ);
                }
            }
            order.push(n);
        }
        while (!order.empty()) {
            callable(order.top());
            order.pop();
        }
    }

private:
    PureGraph(size_t totalNodes) : totalNodes(totalNodes) {
        edges.resize(totalNodes);
        for(auto& row : edges) 
            row.resize(totalNodes);
    }

    size_t totalNodes = 0, totalEdges = 0;
    std::vector<std::vector<bool>> edges;
};

inline std::ostream& operator<<(std::ostream& stream, PureGraph const& graph) {
    if (graph.nodeCount() == 0) {
        stream << "Empty Graph\n";
        return stream;
    }
    for (size_t i = 0; i < graph.nodeCount(); i++) {
        stream << "[s:" << i << "] := {";
        bool first = true;
        for (size_t out : graph.out(i)) {
            if (!first) {
                stream << ", ";
            }
            stream << out;
            first = false;
        }
        stream << "}\n";
    }
    return stream;
}

template<typename T>
struct Graph : PureGraph
{
    Graph(PureGraph const& otherShape) 
        : PureGraph(otherShape) {
        m_nodeData.resize(otherShape.nodeCount());
    }
    Graph(size_t totalNodes = 0) 
        : PureGraph(PureGraph::trivialGraph(totalNodes)) {
        m_nodeData.resize(totalNodes);
    }

    T& nodeData(size_t i) { return m_nodeData[i]; }
    T const& nodeData(size_t i) const { return m_nodeData[i]; }

    size_t createNode(T data) {
        auto node = this->PureGraph::createNode();
        m_nodeData.emplace_back(std::move(data));
        return node;
    }

private:
    std::vector<T> m_nodeData;
};

using DominanceFrontiers = std::unordered_map<size_t, std::vector<size_t>>;

PureGraph calculateIdom(size_t entry, PureGraph const& graph);
DominanceFrontiers dominanceFrontier(size_t entry, size_t exit, PureGraph const& graph);
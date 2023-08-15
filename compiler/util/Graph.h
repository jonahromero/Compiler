
#pragma once
#include <stack>
#include <algorithm>

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
                else out = findNextOut();
            }

        ThisType operator++(int) { auto temp = *this; out = findNextOut(); return temp; }
        ThisType& operator++() { out = findNextOut(); return *this;}
        size_t operator*() const { return out; }
    private:
        size_t findNextOut() const {
            if(out == 0 && graph.hasEdge(state, 0)) return 0;
            for(size_t i = out + 1; i < graph.nodeCount(); i++) {
                bool existsEdge;
                if constexpr (DoPred) existsEdge = graph.hasEdge(i, state); 
                else existsEdge = graph.hasEdge(i, state);
                if(existsEdge) return i;
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
        size_t state, out;
    };

    using PredProxy = EdgeProxy<true>;
    using SuccProxy = EdgeProxy<false>;

    static PureGraph trivialGraph(size_t totalNodes) {
        return PureGraph(totalNodes);
    }
    static PureGraph fullyConnected(size_t totalNodes) {
        auto temp = PureGraph(totalNodes);
        for(auto& row : temp.edges) {
            for(auto& edge : row) 
                edge = true;
        } 
        return temp;
    }

    size_t createNode() {
        totalNodes++;
        //increase table size
        for(auto& src : edges) {src.push_back(false);}
        edges.emplace_back(std::vector<bool>(nodes.size()));
        return totalNodes - 1;
    }

    size_t edgeCount() const { return totalEdges; }
    size_t nodeCount() const { return totalNodes; }
    void removeEdge(size_t src, size_t dst) { edges[src][dst] = false; if(hasEdge(src, dst)) totalEdges--;}
    void addEdge(size_t src, size_t dst) { edges[src][dst] = true; totalEdges++; }
    bool hasEdge(size_t src, size_t dst) const { return edges[src][dst]; }
    SuccProxy out(size_t state) const { return SuccProxy{*this, state}; }
    PredProxy in(size_t state) const { return PredProxy{*this, state}; }

    template<typename Callable>
    void bfs(size_t startNode, Callable callable) const {
        std::queue<size_t> worklist; worklist.push(startNode);
        std::set<size_t> visited; visited.push(startNode);
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

private:
    PureGraph(size_t totalNodes) : totalNodes(totalNodes) {
        edges.resize(totalNodes);
        for(auto& row : edges) 
            row.resize(totalNodes);
    }

    size_t totalNodes = 0 totalEdges = 0;
    std::vector<std::vector<bool>> edges;
};

template<typename T>
struct Graph : PureGraph
{
    Graph(size_t totalNodes = 0) 
        : PureGraph(PureGraph::trivialGraph()) {
        nodeData.resize(totalNodes);
    }

    T& nodeData(size_t i) { return nodeData[i]; }
    T const& nodeData(size_t i) const { return nodeData[i]; }

    size_t createNode(T data) {
        auto node = this->PureGraph::createNode();
        nodeData.emplace_back(std::move(data));
        return node;
    }

private:
    std::vector<T> nodeData;
};

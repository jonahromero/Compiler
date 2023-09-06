
#include <gtest/gtest.h>
#include "Graph.h"

TEST(GraphTest, HasEdge)
{
	PureGraph full = PureGraph::fullyConnected(2);
	PureGraph empty = PureGraph::trivialGraph(2);
	for (size_t s = 0; s < 2; ++s)
		for (size_t d = 0; d < 2; ++d)
			ASSERT_EQ(full.hasEdge(s, d), true);

	for (size_t s = 0; s < 2; ++s)
		for (size_t d = 0; d < 2; ++d)
			ASSERT_EQ(empty.hasEdge(s, d), false);
}


TEST(GraphTest, edgeCount) 
{
	PureGraph graph = PureGraph::trivialGraph(2);
	graph.addEdge(0, 1);
	graph.addEdge(1, 0);
	ASSERT_EQ(graph.edgeCount(), 2) << "addEdge does not update edgeCount correctly.";
	graph.addEdge(0, 1);
	graph.addEdge(1, 0);
	ASSERT_EQ(graph.edgeCount(), 2) << "Duplicated calls to addEdge fail";
}

TEST(GraphTest, OutIterator)
{
	PureGraph graph = PureGraph::fullyConnected(3);
	size_t count = 0;
	for (size_t out : graph.out(0)) {
		++count;
	}
	ASSERT_EQ(count, 3);
	
	PureGraph graph2 = PureGraph::trivialGraph(3);
	graph2.addEdge(0, 1);
	count = 0;
	for (size_t out : graph2.out(0)) {
		++count;
	}
	ASSERT_EQ(count, 1);

}
TEST(GraphTest, DfsCorrectlyVisits)
{
	PureGraph graph = PureGraph::fullyConnected(4);
	size_t count = 0;
	graph.dfs(0, [&count](size_t) {
		count++;
	});
	ASSERT_EQ(count, 4);
	count = 0;
	for (size_t i = 0; i < graph.nodeCount(); ++i) {
		graph.removeAllEdges(i);
	}
	graph.dfs(0, [&](size_t) {
		count++;
	});
	ASSERT_EQ(count, 1);
}

TEST(GraphTest, BfsCorrectlyVisits)
{
	PureGraph graph = PureGraph::fullyConnected(4);
	size_t count = 0;
	graph.bfs(0, [&count](size_t) {
		count++;
	});
	ASSERT_EQ(count, 4);
	count = 0;
	for (size_t i = 0; i < graph.nodeCount(); ++i) {
		graph.removeAllEdges(i);
	}
	graph.dfs(0, [&](size_t) {
		count++;
		});
	ASSERT_EQ(count, 1);
}

TEST(GraphTest, CytronPaper) 
{
	// Example from Ron Cytron paper, page 456
	// https://www.cs.utexas.edu/~pingali/CS380C/2010/papers/ssaCytron.pdf
	PureGraph graph = PureGraph::trivialGraph(14);
	// 0 is entry, 13 is exit
	graph.addEdge(0, 13);
	graph.addEdge(0, 1);
	graph.addEdge(1, 2);
	graph.addEdge(2, 3);
	graph.addEdge(2, 7);
	graph.addEdge(3, 4);
	graph.addEdge(3, 5);
	graph.addEdge(4, 6);
	graph.addEdge(5, 6);
	graph.addEdge(6, 8);
	graph.addEdge(7, 8);
	graph.addEdge(8, 9);
	graph.addEdge(9, 10);
	graph.addEdge(9, 11);
	graph.addEdge(10, 11);
	graph.addEdge(11, 9);
	graph.addEdge(11, 12);
	graph.addEdge(12, 13);
	graph.addEdge(12, 2);
	// check the idom is correct
	PureGraph idom = PureGraph::trivialGraph(14);
	idom.addEdge(0, 1);
	idom.addEdge(0, 13);
	idom.addEdge(1, 2);
	idom.addEdge(2, 3);
	idom.addEdge(2, 7);
	idom.addEdge(2, 8);
	idom.addEdge(3, 4);
	idom.addEdge(3, 5);
	idom.addEdge(3, 6);
	idom.addEdge(8, 9);
	idom.addEdge(9, 10);
	idom.addEdge(9, 11);
	idom.addEdge(11, 12);

	ASSERT_EQ(idom, calculateIdom(0, graph));

	// check the frontiers are correct
	auto frontiers = dominanceFrontier(0, 13, graph);

	auto set = [](auto...args) { return std::set<size_t>({ size_t(args)... }); };
	auto toSet = [](auto& vec) { return std::set<size_t>(vec.begin(), vec.end()); };
#define EXPECT_SET(x, y) \
	EXPECT_EQ(toSet(x), y);

	EXPECT_SET(frontiers[0], set());
	EXPECT_SET(frontiers[1], set(13));
	EXPECT_SET(frontiers[2], set(13, 2));
	EXPECT_SET(frontiers[3], set(8));
	EXPECT_SET(frontiers[4], set(6));
	EXPECT_SET(frontiers[5], set(6));
	EXPECT_SET(frontiers[6], set(8));
	EXPECT_SET(frontiers[7], set(8));
	EXPECT_SET(frontiers[8], set(13, 2));
	EXPECT_SET(frontiers[9], set(13, 2, 9));
	EXPECT_SET(frontiers[10], set(11));
	EXPECT_SET(frontiers[11], set(13, 2, 9));
	EXPECT_SET(frontiers[12], set(13, 2));
	EXPECT_SET(frontiers[13], set());
#undef EXPECT_SET
}
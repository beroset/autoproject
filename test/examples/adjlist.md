I would like to create a graph library for fun and run various searching algorithms on it. For now, all I have is an adjacency list that I threw together. The adjacency list currently only supports undirected graphs. 

**node.h**

	#ifndef SN_GRAPH_NODE_H
	#define SN_GRAPH_NODE_H

	#include <ostream>
	#include <istream>

	/////////////////
	// struct Node //
	/////////////////
	//
	// A simple graph node, just holds data.
	// Can have counts, colors, is_visited, and other attributes later.
	template <typename T>
	struct Node
	{
		T data;
	
		Node(const T &data) : data(data)
		{
		}
	};

	template <typename T>
	bool operator<(const Node<T> &lhs, const Node<T> &rhs)
	{
		return lhs.data < rhs.data;
	}

	template <typename T>
	bool operator==(const Node<T> &lhs, const Node<T> &rhs)
	{
		return lhs.data == rhs.data;
	}

	template <typename T>
	std::ostream& operator<<(std::ostream& os, const Node<T> &node)
	{
		os << node.data;
		return os;
	}

	template <typename T>
	std::istream& operator>>(std::istream& is, Node<T> &node)
	{
		is >> node.data;
		return is;
	}

	#endif

**edge.h**

	#ifndef SN_GRAPH_EDGE_H
	#define SN_GRAPH_EDGE_H

	#include "node.h"
	#include <stdexcept>

	template <typename T>
	struct Edge
	{
		using node_type = Node<T>;
		node_type &n1;
		node_type &n2;
		int cost = 1;

		Edge(node_type &node1, node_type &node2)
			: n1(node1), n2(node2)
		{
		}

		Edge(node_type &node1, node_type &node2, int cost)
			: n1(node1), n2(node2), cost(cost)
		{
		}

		auto other(const node_type& node) -> node_type&
		{
			if (node == n1) {
				return n2;
			}

			else if (node == n2) {
				return n1;
			}

			throw std::invalid_argument("[Edge::other] Cannot find node.");
		}

		auto other(const node_type& node) const -> const node_type&
		{
			if (node == n1) {
				return n2;
			}

			else if (node == n2) {
				return n1;
			}

			throw std::invalid_argument("[Edge::other] Cannot find node.");
		}
	};

	#endif

**adjacency_list.h**

	#ifndef SN_GRAPH_ADJACENCY_LIST_H
	#define SN_GRAPH_ADJACENCY_LIST_H

	#include "node.h"
	#include "edge.h"
	#include <ostream>
	#include <stdexcept>
	#include <vector>

	// Assumes undirected graphs.
	template <typename T>
	class AdjacencyList
	{
	public:
		using node_type = Node<T>;
		using edge_type = Edge<T>;

		AdjacencyList()
		{
		}

		AdjacencyList(const std::vector<node_type> &vertices, const std::vector<edge_type> &edges)
		{
			create(vertices, edges);
		}

		auto getNeighbors(const Node<T> &node) const -> std::vector<Edge<T>>
		{
			auto &entry = find(node);
			return entry.neighbors;
		}

	private:
		struct Entry
		{
			node_type node;
			std::vector<Edge<T>> neighbors;

			Entry(const node_type &node) : node(node)
			{
			}
		};

		friend std::ostream& operator<<(std::ostream& os, const Entry &entry)
		{
			os << entry.node << ": ";
			for (auto &&e : entry.neighbors) {
				auto other = e.other(entry.node);
				os << "(" << other << ", " << e.cost << "), ";
			}

			return os;
		}

		std::vector <Entry> entries;

		auto create(const std::vector<node_type> &vertices, const std::vector<edge_type> &edges) -> void
		{
			// Create entries for each vertex.
			for (auto &&v : vertices) {
				entries.emplace_back(v);
			}

			// Add neighbors to each vertex.
			for (auto &e : edges) {
				auto &entry1 = find(e.n1);
				entry1.neighbors.emplace_back(e);

				auto &entry2 = find(e.n2);
				entry2.neighbors.emplace_back(e);
			}

		}

		auto find(const node_type &node) -> Entry&
		{
			for(auto &entry : entries) {
				if (node == entry.node) {
					return entry;
				}
			}

			throw std::invalid_argument("[AdjacencyList::find] Could not find node.");
		}


		auto find(const node_type &node) const -> const Entry&
		{
			for(auto &entry : entries) {
				if (node == entry.node) {
					return entry;
				}
			}

			throw std::invalid_argument("[AdjacencyList::find] Could not find node.");
		}

		friend std::ostream& operator<<(std::ostream& os, const AdjacencyList &adjacencyList)
		{
			for (auto &&entry : adjacencyList.entries) {
				os << entry << '\n';
			}
		
			return os;
		}
	};

	#endif

Below is a sample driver that uses the adjacency list. I put it in a subdirectory, so the include statements refer to a parent directory.

**test_adjacency_list.cpp**

	#include "../node.h"
	#include "../edge.h"
	#include "../adjacency_list.h"
	#include <iostream>
	#include <vector>

	void test_create()
	{
		Node<int> n1{1};
		Node<int> n2{2};
		Node<int> n3{3};
		Node<int> n4{4};

		Edge<int> e1(n1, n2);
		Edge<int> e2(n1, n3);
		Edge<int> e3(n2, n3);
		Edge<int> e4(n3, n4);

		auto adjacencyList = AdjacencyList<int> ({n1, n2, n3, n4}, {e1, e2, e3, e4});
		std::cout << adjacencyList << '\n';
	}

	int main()
	{
		test_create();
	}

Expected output:

> 1: (2, 1), (3, 1),  
2: (1, 1), (3, 1),  
3: (1, 1), (2, 1), (4, 1),  
4: (3, 1),  

**Concerns**

* My `Edge` class stores references to two `Node`s. This feels wrong, but I'm not sure how else to do it. I don't feel like an `Edge` should take ownership of the `Node`s, so I can either store references or pointers. I feel like using pointers is wrong though because I do not want a situation where one of the `Node`s is ever `nullptr`.

* My adjacency list entries store a `Node` and a vector of `Edge`s for that `Node`. This isn't how adjacency lists usually work, but I want to store the edge cost somehow. I could create a struct, `NeighborEntry`, that stores the neighbor `Node` and the cost, but I'm not sure if I'm gaining much from doing that.

* I plan to throw everything into a namespace later.


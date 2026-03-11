#include "BFS.h"

#include <map>
#include <set>
#include <queue>

#include "Shared/Graph/Graph.h"

using namespace GameAI;

BFS::BFS(Graph* const pGraph)
	: pGraph(pGraph)
{
}

std::vector<Node*> BFS::FindPath(Node* const pStartNode, Node* const pDestinationNode) const
{
	if (!pGraph) return std::vector<Node*>();

	std::queue<Node*> queue;
	queue.push(pStartNode);
	std::set<Node*> visited;
	visited.insert(pStartNode);
	std::map<Node*, Node*> parent;

	while (!queue.empty())
	{
		Node* node = queue.front();
		queue.pop();

		
		if (node == pDestinationNode)
			return ReconstructPath(parent, pStartNode, pDestinationNode);

		for (Connection* neighborConnection : pGraph->FindConnectionsFrom(node->GetId()))
		{
			Node* neighbor = pGraph->GetNode(neighborConnection->GetToId()).get();

			if (!visited.contains(neighbor))
			{
				visited.insert(neighbor);

				parent[neighbor] = node;

				queue.emplace(neighbor);
			}
		}
	}

	std::vector<Node*> path;
	return path;
}

std::vector<Node*> GameAI::BFS::ReconstructPath(std::map<Node*, Node*>& parent, Node* start, Node* goal) const
{
	Node* current = goal;
	std::vector<Node*> path;
	while (current != start)
	{
		path.push_back(current);

		current = parent[current];
	}

	path.push_back(start);

	std::reverse(path.begin(), path.end());

	return path;
}

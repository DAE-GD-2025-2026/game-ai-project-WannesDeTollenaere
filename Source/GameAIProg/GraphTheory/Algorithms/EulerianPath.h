#pragma once
#include <stack>
#include "Shared/Graph/Graph.h"

namespace GameAI
{
	enum class Eulerianity
	{
		notEulerian,
		semiEulerian,
		eulerian,
	};

	class EulerianPath final
	{
	public:
		EulerianPath(Graph* const pGraph);

		Eulerianity IsEulerian() const;
		std::vector<Node*> FindPath(Eulerianity& eulerianity) const;

	private:
		void VisitAllNodesDFS(const std::vector<Node*>& pNodes, std::vector<bool>& visited, int startIndex) const;
		bool IsConnected() const;

		Graph* m_pGraph;
	};

	inline EulerianPath::EulerianPath(Graph* const pGraph)
		: m_pGraph(pGraph)
	{
	}

	inline Eulerianity EulerianPath::IsEulerian() const
	{
		if (!IsConnected())
			return Eulerianity::notEulerian;

		int oddDegreeCount = 0;
		for (auto node : m_pGraph->GetActiveNodes())
		{
			auto connections = m_pGraph->FindConnectionsFrom(node->GetId());

			if (connections.size() % 2 != 0)
				oddDegreeCount++;
		}

		if (oddDegreeCount > 2)
			return Eulerianity::notEulerian;


		if (oddDegreeCount == 2)
			return Eulerianity::semiEulerian;

		if (oddDegreeCount == 0)
			return Eulerianity::eulerian;

		return Eulerianity::notEulerian;
	}

	inline std::vector<Node*> EulerianPath::FindPath(Eulerianity& eulerianity) const
	{
		Graph graphCopy = m_pGraph->Clone();
		std::vector<Node*> Path = {};
		std::vector<Node*> Nodes = graphCopy.GetActiveNodes();
		int currentNodeId{ Graphs::InvalidNodeId };

		if (Nodes.empty())
			return Path;

		if (eulerianity == Eulerianity::notEulerian)
		{
			return Path;
		}

		else if (eulerianity == Eulerianity::eulerian)
		{
			currentNodeId = Nodes[0]->GetId();
		}
		else if (eulerianity == Eulerianity::semiEulerian)
		{
			for (auto node : Nodes)
			{
				if (graphCopy.FindConnectionsFrom(node->GetId()).size() % 2 != 0)
				{
					currentNodeId = node->GetId();
					break;
				}
			}
		}

		std::stack<int> nodeStack;

		while (!nodeStack.empty() || !graphCopy.FindConnectionsFrom(currentNodeId).empty())
		{
			auto connections = graphCopy.FindConnectionsFrom(currentNodeId);

			if (!connections.empty())
			{
				nodeStack.push(currentNodeId);

				int neighborId = connections[0]->GetToId();

				graphCopy.RemoveConnection(currentNodeId, neighborId);
				//graphCopy.RemoveConnection(neighborId, currentNodeId); 

				currentNodeId = neighborId;
			}
			else
			{
				Path.push_back(m_pGraph->GetNode(currentNodeId).get());

				currentNodeId = nodeStack.top();
				nodeStack.pop();
			}
		}

		if (currentNodeId != Graphs::InvalidNodeId)
		{
			Path.push_back(m_pGraph->GetNode(currentNodeId).get());
		}

		std::reverse(Path.begin(), Path.end());
		return Path;
	}

	inline void EulerianPath::VisitAllNodesDFS(const std::vector<Node*>& Nodes, std::vector<bool>& visited, int startIndex) const
	{
		visited[startIndex] = true;

		int currentNodeId = Nodes[startIndex]->GetId();
		auto connections = m_pGraph->FindConnectionsFrom(currentNodeId);

		for (auto connection : connections)
		{
			int connectedNodeId = connection->GetToId();

			for (size_t i = 0; i < Nodes.size(); ++i)
			{
				if (Nodes[i]->GetId() == connectedNodeId)
				{
					if (!visited[i])
					{
						VisitAllNodesDFS(Nodes, visited, i);
					}

					break; 
				}
			}
		}
	}

	inline bool EulerianPath::IsConnected() const
	{
		std::vector<Node*> Nodes = m_pGraph->GetActiveNodes();
		if (Nodes.size() == 0)
			return false;

		//  choose a starting node


		//  start a depth-first-search traversal from the node that has at least one connection
		
		std::vector<bool> visited;
		visited.reserve(Nodes.size());

		std::fill(visited.begin(), visited.end(), false);

		VisitAllNodesDFS(Nodes, visited, 0);

		//  if a node was never visited, this graph is not connected

		auto it = find(visited.begin(), visited.end(), false);

		if (it != visited.end())
			return false;

		return true;
	}
}
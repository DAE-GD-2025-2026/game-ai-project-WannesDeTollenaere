#include "AStar.h"
#include <set>
#include <unordered_map>

using namespace GameAI;

AStar::AStar(Graph* const pGraph, HeuristicFunctions::Heuristic hFunction)
	: pGraph(pGraph)
	, HeuristicFunction(hFunction)
{
}

std::vector<Node*> AStar::FindPath(Node* const pStartNode, Node* const pGoalNode)
{
	std::vector<Node*> path{};

	if (pStartNode == nullptr || pGoalNode == nullptr)
	{
		return path;
	}

	std::vector<NodeRecord> openList{};
	std::vector<NodeRecord> closedList{};

	NodeRecord startRecord;
	startRecord.pNode = pStartNode;
	startRecord.pConnection = nullptr;
	startRecord.costSoFar = 0.f;
	startRecord.estimatedTotalCost = GetHeuristicCost(pStartNode, pGoalNode);

	openList.push_back(startRecord);

	NodeRecord currentNodeRecord;

	while (!openList.empty())
	{
		auto current = std::min_element(openList.begin(), openList.end());
		currentNodeRecord = *current;

		if (currentNodeRecord.pNode == pGoalNode)
			break;

		openList.erase(current);

		std::vector<Connection*> connections = pGraph->FindConnectionsFrom(currentNodeRecord.pNode->GetId());

		for (Connection* connection : connections)
		{
			Node* pNextNode = pGraph->GetNode(connection->GetToId()).get();

			float gCost = currentNodeRecord.costSoFar + connection->GetWeight();

			auto closedRecord = std::find_if(closedList.begin(), closedList.end(),
				[pNextNode](const NodeRecord& record) { return record.pNode == pNextNode; });

			if (closedRecord != closedList.end())
			{
				if (closedRecord->costSoFar <= gCost)
					continue; 
				else
					closedList.erase(closedRecord);
			}

			auto openRecord = std::find_if(openList.begin(), openList.end(),
				[pNextNode](const NodeRecord& record) { return record.pNode == pNextNode; });

			if (openRecord != openList.end())
			{
				if (openRecord->costSoFar <= gCost)
					continue; 
				else
					openList.erase(openRecord); 
			}

			NodeRecord newRecord;
			newRecord.pNode = pNextNode;
			newRecord.pConnection = connection;
			newRecord.costSoFar = gCost;
			newRecord.estimatedTotalCost = gCost + GetHeuristicCost(pNextNode, pGoalNode);

			openList.push_back(newRecord);
		}

		closedList.push_back(currentNodeRecord);
	}

	if (currentNodeRecord.pNode != pGoalNode)
		return path;
	

	while (currentNodeRecord.pNode != pStartNode)
	{
		path.push_back(currentNodeRecord.pNode);

		int fromNodeId = currentNodeRecord.pConnection->GetFromId();
		auto parentRecord = std::find_if(closedList.begin(), closedList.end(),
			[fromNodeId](const NodeRecord& record) { return record.pNode->GetId() == fromNodeId; });

		if (parentRecord != closedList.end())
			currentNodeRecord = *parentRecord;
		else
			break;
	}

	path.push_back(pStartNode);

	std::reverse(path.begin(), path.end());
	return path;
}
float AStar::GetHeuristicCost(Node* const pStartNode, Node* const pEndNode) const
{
	FVector2D toDestination = pGraph->GetNode(pEndNode->GetId())->GetPosition() - pGraph->GetNode(pStartNode->GetId())->GetPosition();
	return HeuristicFunction(abs(toDestination.X), abs(toDestination.Y));
}
#include "NavGraphPathfinding.h"

#include "AStar.h"
#include "PathSmoothing.h"
#include "VectorTypes.h"
#include "Shared/Graph/NavGraph/NavGraph.h"
#include "Shared/Graph/NavGraph/NavGraphNode.h"

using namespace GameAI;

std::vector<FVector2D> NavMeshPathfinding::FindPath(const FVector2D& startPos, const FVector2D& endPos,
	NavGraph* const pNavGraph, std::vector<FVector2D>& debugNodePositions, std::vector<NavLine>& debugPortals) 
{
	//Create the path to return
	std::vector<FVector2D> finalPath{};

	if (!pNavGraph) return finalPath;

	auto const* pNavMeshPoly = pNavGraph->GetNavPolygon();
	if (!pNavMeshPoly) return finalPath;

	//Get the start and endTriangle
	auto const* pStartTri = pNavMeshPoly->GetTriangleAtPosition(startPos, true);
	auto const* pEndTri = pNavMeshPoly->GetTriangleAtPosition(endPos, true);



	//We have valid start/end triangles and they are not the same
	if (!pStartTri || !pEndTri) return finalPath;

	if (pStartTri == pEndTri)
	{
		finalPath.push_back(endPos);
		return finalPath;
	}
	//=> Start looking for a path
	//Copy the graph
	std::unique_ptr<NavGraph> clonedGraph = pNavGraph->Clone();

	//Create Extra node for the Start Node (Agent's position
	auto pStartNode = std::make_unique<NavGraphNode>(startPos, -1);
	int startNodeId = clonedGraph->AddNode(std::move(pStartNode));

	// CREATE start node connections
	for (const auto& edge : pStartTri->GetEdges())
	{
		auto edgeIdxOpt = pNavMeshPoly->FindEdgeIndex(edge);
		if (edgeIdxOpt.has_value())
		{
			int connectedNodeId = clonedGraph->GetNodeIdFromEdgeIndex(edgeIdxOpt.value());
			if (connectedNodeId != Graphs::InvalidNodeId)
			{
				float dist = FVector2D::Distance(startPos, clonedGraph->GetNode(connectedNodeId)->GetPosition());
				
				Connection con{ startNodeId, connectedNodeId };
				con.SetWeight(dist);

				clonedGraph->AddConnection(std::make_unique<Connection>(con));
			}
		}
	}

	//Create extra node for the endNode
	auto pEndNode = std::make_unique<NavGraphNode>(endPos, -1);
	int endNodeId = clonedGraph->AddNode(std::move(pEndNode));

	// CREATe end node connections
	for (const auto& edge : pEndTri->GetEdges())
	{
		auto edgeIdxOpt = pNavMeshPoly->FindEdgeIndex(edge);
		if (edgeIdxOpt.has_value())
		{
			int connectedNodeId = clonedGraph->GetNodeIdFromEdgeIndex(edgeIdxOpt.value());
			if (connectedNodeId != Graphs::InvalidNodeId)
			{
				float dist = FVector2D::Distance(endPos, clonedGraph->GetNode(connectedNodeId)->GetPosition());
				Connection con{ connectedNodeId, endNodeId };
				con.SetWeight(dist);

				clonedGraph->AddConnection(std::make_unique<Connection>(con));
			}
		}
	}
	clonedGraph->SetConnectionCostsToDistances();


	//Run A star on new graph
	AStar aStarPathfinder(clonedGraph.get(), HeuristicFunctions::Euclidean);
	std::vector<Node*> aStarPath = aStarPathfinder.FindPath(clonedGraph->GetNodeAs<NavGraphNode>(startNodeId), clonedGraph->GetNodeAs<NavGraphNode>(endNodeId));

	if (aStarPath.empty()) return finalPath;

	//Debug Visualisation
	for (const Node* pNode : aStarPath)
	{
		debugNodePositions.push_back(pNode->GetPosition());
		finalPath.push_back(pNode->GetPosition());
	}
	// Extra: Run optimiser on new graph (First check if everything works without SSFA!)
	debugPortals = SSFA::FindPortals(aStarPath, *pNavGraph->GetNavPolygon());
	finalPath = SSFA::OptimizePortals(debugPortals, *pNavGraph->GetNavPolygon());
	
	return finalPath;
}

std::vector<FVector2D> NavMeshPathfinding::FindPath(const FVector2D& startPos, const FVector2D& endPos, NavGraph* const pNavGraph)
{
	std::vector<FVector2D> debugNodePositions{};
	std::vector<NavLine> debugPortals{};

	return FindPath(startPos, endPos, pNavGraph, debugNodePositions, debugPortals);
}
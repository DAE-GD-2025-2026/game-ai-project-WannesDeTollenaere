#include "NavGraph.h"

#include "NavGraphNode.h"

GameAI::NavGraph::NavGraph(std::unique_ptr<TriPolygon> && NavPoly)
	: Graph{false}
	, pNavPoly{std::move(NavPoly)}
{
	CreateNavigationGraph();
}

GameAI::NavGraph::NavGraph(const NavGraph& Other)
	: Graph(false)
{
	Nodes.reserve(Other.Nodes.size());
	for (std::unique_ptr<Node> const & OtherNode : Other.Nodes)
	{
		Nodes.push_back(std::make_unique<NavGraphNode>(*dynamic_cast<NavGraphNode*>(OtherNode.get())));
	}
        
	Connections.reserve(Other.Connections.size());
	for (std::unique_ptr<Connection> const & OtherConnection : Other.Connections)
	{
		Connections.push_back(std::make_unique<Connection>(*OtherConnection.get()));
	}
}

std::unique_ptr<GameAI::NavGraph> GameAI::NavGraph::Clone() const
{
	return std::make_unique<NavGraph>(*this);
}

int GameAI::NavGraph::GetNodeIdFromEdgeIndex(int EdgeIdx) const
{
	if (EdgeIdx >= 0)
	{
		for (auto const & pNode : Nodes)
		{
			if (reinterpret_cast<NavGraphNode*>(pNode.get())->GetEdgeIdx() == EdgeIdx)
			{
				return pNode->GetId();
			}
		}
	}
	
	return Graphs::InvalidNodeId;
}

void GameAI::NavGraph::CreateNavigationGraph()
{
    if (!pNavPoly) return;

    const auto& edges = pNavPoly->GetEdges();
    const auto& triangles = pNavPoly->GetTriangles();

    for (int edgeIdx = 0; edgeIdx < edges.size(); ++edgeIdx)
    {
        const TriPolygon::Edge& edge = edges[edgeIdx];

        // is line connected to other tri
        int sharedCount = 0;
        for (const TriPolygon::Triangle& tri : triangles)
        {
            if (tri.HasEdge(edge))
            {
                sharedCount++;
            }
        }


        // if edge more than 1 tri than valid portal
        if (sharedCount > 1)
        {
            FVector p1 = edge.GetP1(*pNavPoly);
            FVector p2 = edge.GetP2(*pNavPoly);

            FVector2D midPoint((p1.X + p2.X) / 2.0f, (p1.Y + p2.Y) / 2.0f);

            AddNode(std::make_unique<NavGraphNode>(midPoint, edgeIdx));
        }
    }


    // CREATE CONNECTIONS
    for (const TriPolygon::Triangle& tri : triangles)
    {
        std::vector<int> validNodeIds;


        for (const TriPolygon::Edge& edge : tri.GetEdges())
        {
            auto edgeIdxOpt = pNavPoly->FindEdgeIndex(edge);
            if (edgeIdxOpt.has_value())
            {
                int nodeId = GetNodeIdFromEdgeIndex(edgeIdxOpt.value());
                if (nodeId != Graphs::InvalidNodeId)
                {
                    validNodeIds.push_back(nodeId);
                }
            }
        }

        if (validNodeIds.size() == 2)
        {
            float dist = FVector2D::Distance(GetNode(validNodeIds[0])->GetPosition(), GetNode(validNodeIds[1])->GetPosition());

            Connection con1{ validNodeIds[0], validNodeIds[1] };
            con1.SetWeight(dist);

            AddConnection(std::make_unique<Connection>(con1));
        }
        else if (validNodeIds.size() == 3)
        {

            float dist01 = FVector2D::Distance(GetNode(validNodeIds[0])->GetPosition(), GetNode(validNodeIds[1])->GetPosition());
            float dist12 = FVector2D::Distance(GetNode(validNodeIds[1])->GetPosition(), GetNode(validNodeIds[2])->GetPosition());
            float dist20 = FVector2D::Distance(GetNode(validNodeIds[2])->GetPosition(), GetNode(validNodeIds[0])->GetPosition());


            Connection con1{ validNodeIds[0], validNodeIds[1] };
            con1.SetWeight(dist01);
            Connection con2{ validNodeIds[1], validNodeIds[2] };
            con2.SetWeight(dist12);
            Connection con3{ validNodeIds[2], validNodeIds[0] };
            con3.SetWeight(dist20);

            AddConnection(std::make_unique<Connection>(con1));
            AddConnection(std::make_unique<Connection>(con2));
            AddConnection(std::make_unique<Connection>(con3));

        }
    }
}

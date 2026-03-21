#pragma once
#include <vector>

#include "NavGraphPathfinding.h"
#include "Movement/Pathfinding/Navmesh/TriPolygon.h"
#include "Shared/Graph/Graph.h"
#include "Shared/Graph/NavGraph/NavGraphNode.h"

namespace GameAI
{
	class SSFA final
	{
	public:
		//=== SSFA Functions ===
		//--- References ---
		//http://digestingduck.blogspot.be/2010/03/simple-stupid-funnel-algorithm.html
		//https://gamedev.stackexchange.com/questions/68302/how-does-the-simple-stupid-funnel-algorithm-work
		static std::vector<NavLine> FindPortals(std::vector<Node*> const& Path, TriPolygon const& NavPoly)
		{
			//Container
			std::vector<NavLine> Portals = {};
			if (Path.empty()) return Portals;

			Portals.push_back({ Path[0]->GetPosition(), Path[0]->GetPosition() });

			//For each node received, get it's corresponding line
			for (size_t i = 1; i < Path.size() - 1; ++i)
			{
				NavGraphNode* pNode = static_cast<NavGraphNode*>(Path[i]);
				int edgeIdx = pNode->GetEdgeIdx();

				if (edgeIdx >= 0 && edgeIdx < NavPoly.GetEdges().size())
				{
					auto const& edge = NavPoly.GetEdges()[edgeIdx];
					FVector p1 = edge.GetP1(NavPoly);
					FVector p2 = edge.GetP2(NavPoly);

					FVector2D p1_2D(p1.X, p1.Y);
					FVector2D p2_2D(p2.X, p2.Y);

					//Redetermine it's "orientation" based on the required path (left-right vs right-left) - p1 should be right point
					FVector2D moveDir = Path[i]->GetPosition() - Path[i - 1]->GetPosition();
					FVector2D portalDir = p2_2D - p1_2D;

					// FIX: In Unreal, positive cross product means Right. If we want P1 to be Right, 
					// portalDir (P2 - P1) must point Left. Therefore we expect a NEGATIVE cross product.
					// If it is > 0, we swap.
					if (FVector2D::CrossProduct(moveDir, portalDir) > 0.0f)
					{
						std::swap(p1_2D, p2_2D);
					}

					//Store portal
					Portals.push_back({ p1_2D, p2_2D });
				}
			}

			//Add degenerate portal to force end evaluation
			if (Path.size() > 1)
			{
				Portals.push_back({ Path.back()->GetPosition(), Path.back()->GetPosition() });
			}

			return Portals;
		}

		static std::vector<FVector2D> OptimizePortals(std::vector<NavLine> const& Portals, TriPolygon const& NavPoly)
		{
			std::vector<FVector2D> Path{};
			if (Portals.empty()) return Path;

			FVector2D apexPos = Portals[0].P1;
			Path.push_back(apexPos);

			if (Portals.size() == 1) return Path;

			//P1 == right point of portal, P2 == left point of portal

			int apexIndex = 0;
			int rightLegIndex = 1;
			int leftLegIndex = 1;

			FVector2D rightLeg = Portals[1].P1 - apexPos;
			FVector2D leftLeg = Portals[1].P2 - apexPos;

			for (int portalIdx = 1; portalIdx < Portals.size(); ++portalIdx)
			{
				// Get the current portal and store it in a local variable
				NavLine currentPortal = Portals[portalIdx];

				// --- RIGHT CHECK ---
				FVector2D newRightLeg = currentPortal.P1 - apexPos;

				//1. See if moving funnel inwards - RIGHT (Moving leftwards towards center = Negative in Unreal)
				if (FVector2D::CrossProduct(rightLeg, newRightLeg) <= 0.0f)
				{
					//2. See if new line degenerates a line segment - RIGHT (Not crossing left leg = Positive)
					if (FVector2D::CrossProduct(leftLeg, newRightLeg) >= 0.0f)
					{
						rightLeg = newRightLeg;
						rightLegIndex = portalIdx;
					}
					else
					{
						//Leftleg becomes new apex point
						apexPos = apexPos + leftLeg;

						apexIndex = leftLegIndex;
						portalIdx = leftLegIndex + 1;

						leftLegIndex = portalIdx;
						rightLegIndex = portalIdx;

						Path.push_back(apexPos);

						//Calculate new legs (if not the end)
						if (portalIdx < Portals.size())
						{
							rightLeg = Portals[portalIdx].P1 - apexPos;
							leftLeg = Portals[portalIdx].P2 - apexPos;
							portalIdx--;
							continue;
						}
					}
				}

				// --- LEFT CHECK ---
				FVector2D newLeftLeg = currentPortal.P2 - apexPos;

				//1. See if moving funnel inwards - LEFT (Moving rightwards towards center = Positive in Unreal)
				if (FVector2D::CrossProduct(leftLeg, newLeftLeg) >= 0.0f)
				{
					//2. See if new line degenerates a line segment - LEFT (Not crossing right leg = Negative)
					if (FVector2D::CrossProduct(rightLeg, newLeftLeg) <= 0.0f)
					{
						leftLeg = newLeftLeg;
						leftLegIndex = portalIdx;
					}
					else
					{
						//Rightleg becomes new apex point
						apexPos = apexPos + rightLeg;

						apexIndex = rightLegIndex;
						portalIdx = rightLegIndex + 1;

						leftLegIndex = portalIdx;
						rightLegIndex = portalIdx;

						Path.push_back(apexPos);

						//Calculate new legs (if not the end)
						if (portalIdx < Portals.size())
						{
							rightLeg = Portals[portalIdx].P1 - apexPos;
							leftLeg = Portals[portalIdx].P2 - apexPos;
							portalIdx--;
							continue;
						}
					}
				}
			}

			// Add last path point
			Path.push_back(Portals.back().P1);

			return Path;
		}
	private:
		SSFA() {};
		~SSFA() {};
	};
}
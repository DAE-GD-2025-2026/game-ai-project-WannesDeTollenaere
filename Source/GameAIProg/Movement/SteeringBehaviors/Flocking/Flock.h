#pragma once

// Toggle this define to enable/disable spatial partitioning
// #define GAMEAI_USE_SPACE_PARTITIONING

#include "FlockingSteeringBehaviors.h"
#include "Movement/SteeringBehaviors/SteeringAgent.h"
#include "Movement/SteeringBehaviors/SteeringHelpers.h"
#include "Movement/SteeringBehaviors/CombinedSteering/CombinedSteeringBehaviors.h"
#include <memory>
#include "imgui.h"

class CellSpace;

class Flock final
{
public:
	Flock(
	UWorld* pWorld,
	TSubclassOf<ASteeringAgent> AgentClass,
	int FlockSize = 10,
	float WorldSize = 100.f,
	ASteeringAgent* const pAgentToEvade = nullptr,
	bool bTrimWorld = false,
	FLinearColor const& FlockColor = FLinearColor::White);

	~Flock();

	void Tick(float DeltaTime);
	void RenderDebug();
	void ImGuiRender(ImVec2 const& WindowPos, ImVec2 const& WindowSize, int NrOfFlocks, TFunction<void()> const& OnAddFlock, TFunction<void()> const& OnRemoveFlock);

	void RegisterNeighbors(ASteeringAgent* const Agent);

	int GetNrOfNeighbors() const;
	const TArray<ASteeringAgent*>& GetNeighbors() const;

	FVector2D GetAverageNeighborPos() const;
	FVector2D GetAverageNeighborVelocity() const;

	void SetTarget_Seek(FSteeringParams const & Target);

	// multiple flocks avoiding each other
	const TArray<ASteeringAgent*>& GetAgents() const { return Agents; }
	void SetOtherFlocks(TArray<Flock*> const& InOtherFlocks) { OtherFlocks = InOtherFlocks; }
	const TArray<ASteeringAgent*>& GetOtherFlockNeighbors() const { return OtherFlockNeighbors; }

private:
	// For debug rendering purposes
	UWorld* pWorld{nullptr};
	
	int FlockSize{0};
	TArray<ASteeringAgent*> Agents{};

	FLinearColor FlockColor{FLinearColor::White};
	void ApplyFlockColor(ASteeringAgent* const Agent) const;

	// Cell space
	std::unique_ptr<CellSpace> pPartitionedSpace{};
	int NrOfCellsX{10 };
	TArray<FVector2D> OldPositions{};
	bool bUseSpacePartitioning{ true };

	TArray<ASteeringAgent*> Neighbors{};

	float NeighborhoodRadius{100.f};
	int NrOfNeighbors{0};

	//vision cone in front of the boid
	bool bUseVisionCone{true};
	float VisionConeAngle{221.f};

	TArray<ASteeringAgent*> VisibleNeighbors{};
	int NrOfVisibleNeighbors{0};

	void FilterNeighborsByVisionCone(ASteeringAgent* const Agent);

	// multiple flocks which avoid each other
	TArray<Flock*> OtherFlocks{};
	TArray<ASteeringAgent*> OtherFlockNeighbors{};

	bool bAvoidOtherFlocks{true};
	float InterFlockAvoidanceRadius{200.f};

	std::unique_ptr<FlockAvoidance> pFlockAvoidanceBehavior{};

	void RegisterOtherFlockNeighbors(ASteeringAgent* const Agent);

	ASteeringAgent* pAgentToEvade{nullptr};
	
	float m_WorldSize{ 1000.f };
	bool m_bTrimWorld{ false };
	//Steering Behaviors
	std::unique_ptr<Separation> pSeparationBehavior{};
	std::unique_ptr<Cohesion> pCohesionBehavior{};
	std::unique_ptr<VelocityMatch> pVelMatchBehavior{};
	std::unique_ptr<Seek> pSeekBehavior{};
	std::unique_ptr<Wander> pWanderBehavior{};
	std::unique_ptr<Evade> pEvadeBehavior{};
	
	std::unique_ptr<BlendedSteering> pBlendedSteering{};
	std::unique_ptr<PrioritySteering> pPrioritySteering{};

	// UI and rendering
	bool DebugRenderSteering{false};
	bool DebugRenderNeighborhood{true};
	bool DebugRenderPartitions{true};

	void RenderNeighborhood();
};

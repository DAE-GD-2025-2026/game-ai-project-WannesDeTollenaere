#include "Flock.h"
#include "FlockingSteeringBehaviors.h"
#include "Shared/ImGuiHelpers.h"
#include "Movement/SteeringBehaviors/SpacePartitioning/SpacePartitioning.h"

Flock::Flock(
	UWorld* pWorld,
	TSubclassOf<ASteeringAgent> AgentClass,
	int FlockSize,
	float WorldSize,
	ASteeringAgent* const pAgentToEvade,
	bool bTrimWorld)
	: pWorld{pWorld}
	, FlockSize{ FlockSize }
	, pAgentToEvade{pAgentToEvade},
	m_bTrimWorld{bTrimWorld},
	m_WorldSize{WorldSize}
{

	// initialize the flock and the memory pool
	Agents.SetNum(FlockSize);
	Neighbors.SetNum(FlockSize); 
	OldPositions.SetNum(FlockSize);

	// Create cell space
	pPartitionedSpace = std::make_unique<CellSpace>(pWorld, WorldSize * 2.0f, WorldSize * 2.0f, NrOfCellsX, NrOfCellsX, FlockSize);

	// initialize behaviors
	pSeparationBehavior = std::make_unique<Separation>(this);
	pCohesionBehavior = std::make_unique<Cohesion>(this);
	pVelMatchBehavior = std::make_unique<VelocityMatch>(this);
	pSeekBehavior = std::make_unique<Seek>();
	pWanderBehavior = std::make_unique<Wander>();
	pEvadeBehavior = std::make_unique<Evade>();

	// initialize blended steering
	std::vector<BlendedSteering::WeightedBehavior> blendedWeights;
	blendedWeights.push_back({ pCohesionBehavior.get(), 0.3f });
	blendedWeights.push_back({ pSeparationBehavior.get(), 0.5f });
	blendedWeights.push_back({ pVelMatchBehavior.get(), 0.3f });
	blendedWeights.push_back({ pSeekBehavior.get(), 0.2f });
	blendedWeights.push_back({ pWanderBehavior.get(), 0.2f });
	pBlendedSteering = std::make_unique<BlendedSteering>(blendedWeights);

	// init priority steering
	std::vector<ISteeringBehavior*> priorityBehaviors;
	priorityBehaviors.push_back(pEvadeBehavior.get());
	priorityBehaviors.push_back(pBlendedSteering.get());
	pPrioritySteering = std::make_unique<PrioritySteering>(priorityBehaviors);

	// initalize flock agents
	for (int i = 0; i < FlockSize; ++i)
	{
		FVector spawnPos = FVector(FMath::RandRange(-WorldSize, WorldSize), FMath::RandRange(-WorldSize, WorldSize), 0.f);

		FActorSpawnParameters spawnParams;
		spawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn; // Force Spawn

		Agents[i] = pWorld->SpawnActor<ASteeringAgent>(AgentClass, spawnPos, FRotator::ZeroRotator, spawnParams);

		if (Agents[i])
		{
			Agents[i]->SetSteeringBehavior(pPrioritySteering.get());
			Agents[i]->SetActorTickEnabled(false); 

			pPartitionedSpace->AddAgent(*Agents[i]);
			OldPositions[i] = Agents[i]->GetPosition();
		}
	}
}

Flock::~Flock()
{
	for (ASteeringAgent* agent : Agents)
	{
		if (agent) agent->Destroy();
	}
}

void Flock::Tick(float DeltaTime)
{
 // update the flock
 // for every agent:
  //   register the neighbors for this agent (-> fill the memory pool with the neighbors for the currently evaluated agent)
  //   update the agent (-> the steeringbehaviors use the neighbors in the memory pool)
  //   trim the agent to the world

	if (pAgentToEvade && pEvadeBehavior)
	{
		FTargetData evadeTargetData;
		evadeTargetData.Position = pAgentToEvade->GetPosition();
		evadeTargetData.LinearVelocity = pAgentToEvade->GetLinearVelocity();
		pEvadeBehavior->SetTarget(evadeTargetData);
	}

	for (int i = 0; i < Agents.Num(); ++i)
	{
		ASteeringAgent* pAgent = Agents[i];
		if (!pAgent) continue;

		if (pPartitionedSpace)
		{
			pPartitionedSpace->UpdateAgentCell(*pAgent, OldPositions[i]);
		}

		OldPositions[i] = pAgent->GetPosition();


		if (bUseSpacePartitioning && pPartitionedSpace)
		{
			pPartitionedSpace->RegisterNeighbors(*pAgent, NeighborhoodRadius);
		}
		else
		{
			RegisterNeighbors(pAgent);
		}

		pAgent->Tick(DeltaTime);

		if (m_bTrimWorld)
		{
			FVector pos = pAgent->GetActorLocation();
			bool changed = false;

			if (pos.X > m_WorldSize) { pos.X = -m_WorldSize; changed = true; }
			else if (pos.X < -m_WorldSize) { pos.X = m_WorldSize; changed = true; }

			if (pos.Y > m_WorldSize) { pos.Y = -m_WorldSize; changed = true; }
			else if (pos.Y < -m_WorldSize) { pos.Y = m_WorldSize; changed = true; }

			if (changed) pAgent->SetActorLocation(pos);
		}
	}
}

void Flock::RenderDebug()
{
 // Render all the agents in the flock
	for (ASteeringAgent* agent : Agents)
	{
		if (agent) agent->SetDebugRenderingEnabled(DebugRenderSteering);
	}

	if (DebugRenderNeighborhood)
	{
		RenderNeighborhood();
	}
	if (DebugRenderPartitions && bUseSpacePartitioning && pPartitionedSpace)
	{
		pPartitionedSpace->RenderCells();
	}
	if (m_bTrimWorld)
	{
		FVector center = FVector::ZeroVector;
		FVector extents = FVector(m_WorldSize, m_WorldSize, 0.0f); 

		DrawDebugBox(
			pWorld,
			center,
			extents,
			FColor::Red,
			false,
			-1.f,
			0,
			5.f 
		);
	}
}

void Flock::ImGuiRender(ImVec2 const& WindowPos, ImVec2 const& WindowSize)
{
#ifdef PLATFORM_WINDOWS
#pragma region UI
	//UI
	{
		//Setup
		bool bWindowActive = true;
		ImGui::SetNextWindowPos(WindowPos);
		ImGui::SetNextWindowSize(WindowSize);
		ImGui::Begin("Gameplay Programming", &bWindowActive, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

		//Elements
		ImGui::Text("CONTROLS");
		ImGui::Indent();
		ImGui::Text("LMB: place target");
		ImGui::Text("RMB: move cam.");
		ImGui::Text("Scrollwheel: zoom cam.");
		ImGui::Unindent();

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();
		ImGui::Spacing();

		ImGui::Text("STATS");
		ImGui::Indent();
		ImGui::Text("%.3f ms/frame", 1000.0f / ImGui::GetIO().Framerate);
		ImGui::Text("%.1f FPS", ImGui::GetIO().Framerate);
		ImGui::Unindent();

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();

		ImGui::Text("Flocking");
		ImGui::Spacing();

  // implement ImGUI checkboxes for debug rendering here
		ImGui::Checkbox("Debug Render Steering", &DebugRenderSteering);
		ImGui::Checkbox("Debug Render Neighborhood", &DebugRenderNeighborhood);
		
		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();

		ImGui::Checkbox("Use Space Partitioning", &bUseSpacePartitioning);
		if (bUseSpacePartitioning)
		{
			ImGui::Checkbox("Debug Render Partitions", &DebugRenderPartitions);
		}

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();

		//ImGui::Checkbox("Trim World", &m_bTrimWorld); 
		//if (m_bTrimWorld)
		//{
		//	ImGuiHelpers::ImGuiSliderFloatWithSetter("Trim Size",
		//		m_WorldSize, 500.f, 5000.f,
		//		[this](float InVal) { m_WorldSize = InVal; }, "%.0f");
		//}

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();

		ImGui::Text("Behavior Weights");
		ImGui::Spacing();

  // implement ImGUI sliders for steering behavior weights here
		if (pBlendedSteering)
		{
			auto& weights = pBlendedSteering->GetWeightedBehaviorsRef();
			if (weights.size() >= 5)
			{
				ImGuiHelpers::ImGuiSliderFloatWithSetter("Cohesion", weights[0].Weight, 0.f, 1.f, [&weights](float InVal) { weights[0].Weight = InVal; }, "%.2f");
				ImGuiHelpers::ImGuiSliderFloatWithSetter("Separation", weights[1].Weight, 0.f, 1.f, [&weights](float InVal) { weights[1].Weight = InVal; }, "%.2f");
				ImGuiHelpers::ImGuiSliderFloatWithSetter("Velocity Match", weights[2].Weight, 0.f, 1.f, [&weights](float InVal) { weights[2].Weight = InVal; }, "%.2f");
				ImGuiHelpers::ImGuiSliderFloatWithSetter("Seek", weights[3].Weight, 0.f, 1.f, [&weights](float InVal) { weights[3].Weight = InVal; }, "%.2f");
				ImGuiHelpers::ImGuiSliderFloatWithSetter("Wander", weights[4].Weight, 0.f, 1.f, [&weights](float InVal) { weights[4].Weight = InVal; }, "%.2f");
			}
		}
		//End
		ImGui::End();
	}
#pragma endregion
#endif
}

void Flock::RenderNeighborhood()
{
 // Debugrender the neighbors for the first agent in the flock
	if (Agents.Num() > 0 && Agents[0] != nullptr)
	{
		if (bUseSpacePartitioning) pPartitionedSpace->RegisterNeighbors(*Agents[0], NeighborhoodRadius);
		else RegisterNeighbors(Agents[0]);

		FVector pos = Agents[0]->GetActorLocation();
		DrawDebugCircle(pWorld, pos, NeighborhoodRadius, 32, FColor::Green, false, -1.f, 0, 2.f, FVector(1, 0, 0), FVector(0, 1, 0), false);

		for (int i = 0; i < GetNrOfNeighbors(); ++i)
		{
			DrawDebugLine(pWorld, pos, GetNeighbors()[i]->GetActorLocation(), FColor::Green, false, -1.f, 0, 2.f);
		}

		if (bUseSpacePartitioning && pPartitionedSpace)
		{

			FVector BoxExtents = FVector(NeighborhoodRadius, NeighborhoodRadius, 0.f);

			DrawDebugBox(pWorld, pos, BoxExtents, FColor::Yellow, false, -1.f, 0, 2.0f);
		}
	}
}

void Flock::RegisterNeighbors(ASteeringAgent* const pAgent)
{
	NrOfNeighbors = 0; 
	FVector2D agentPos = pAgent->GetPosition();

	for (ASteeringAgent* pOtherAgent : Agents)
	{
		if (pAgent == pOtherAgent || !pOtherAgent) 
			continue;

		float distanceSquared = FVector2D::DistSquared(agentPos, pOtherAgent->GetPosition());
		if (distanceSquared <= (NeighborhoodRadius * NeighborhoodRadius))
		{
			Neighbors[NrOfNeighbors] = pOtherAgent;
			NrOfNeighbors++;
		}
	}
}

int Flock::GetNrOfNeighbors() const
{
	if (bUseSpacePartitioning && pPartitionedSpace)
	{
		return pPartitionedSpace->GetNrOfNeighbors();
	}
	return NrOfNeighbors;
}

const TArray<ASteeringAgent*>& Flock::GetNeighbors() const
{
	if (bUseSpacePartitioning && pPartitionedSpace)
	{
		return pPartitionedSpace->GetNeighbors();
	}
	return Neighbors;
}

FVector2D Flock::GetAverageNeighborPos() const
{
	if (GetNrOfNeighbors() == 0) return FVector2D::ZeroVector;

	FVector2D avgPosition = FVector2D::ZeroVector;
	const auto& neighbors = GetNeighbors();

	for (int i = 0; i < GetNrOfNeighbors(); ++i)
	{
		avgPosition += neighbors[i]->GetPosition();
	}

	return avgPosition / static_cast<float>(GetNrOfNeighbors());
}

FVector2D Flock::GetAverageNeighborVelocity() const
{
	if (GetNrOfNeighbors() == 0) return FVector2D::ZeroVector;

	FVector2D avgVelocity = FVector2D::ZeroVector;
	const auto& neighbors = GetNeighbors();

	for (int i = 0; i < GetNrOfNeighbors(); ++i)
	{
		avgVelocity += neighbors[i]->GetLinearVelocity();
	}

	return avgVelocity / static_cast<float>(GetNrOfNeighbors());
}

void Flock::SetTarget_Seek(FSteeringParams const& Target)
{
	if (pSeekBehavior)
	{
		pSeekBehavior->SetTarget(Target);
	}
}


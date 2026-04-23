// Fill out your copyright notice in the Description page of Project Settings.


#include "Level_FSM.h"
#include "GuardStates.h"

#include "Movement/SteeringBehaviors/Steering/SteeringBehaviors.h"
#include "Movement/SteeringBehaviors/SteeringAgent.h"
#include "FSMComponent.h"
#include "DecisionMaking/GameAIController.h"

#include <imgui.h>


// Sets default values
ALevel_FSM::ALevel_FSM()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ALevel_FSM::BeginPlay()
{
	Super::BeginPlay();

	//SeekBehaviour = std::make_unique<Seek>();

	Agent = GetWorld()->SpawnActor<ASteeringAgent>(SteeringAgentClass,
		FVector{ 0,0,90 }, FRotator::ZeroRotator);
	Agent->SetMaxLinearSpeed(450.f);


	Thief = GetWorld()->SpawnActor<ASteeringAgent>(SteeringAgentClass,
		FVector{ 890,-860,90 }, FRotator::ZeroRotator);
	Thief->SetMaxLinearSpeed(800.f);

	//Thief->SetSteeringBehavior(SeekBehaviour.get());
	Agent->SetDebugRenderingEnabled(false);

	if (AGameAIController* AIController = Cast<AGameAIController>(Agent->GetController()))
	{
		if (UFSMComponent* FSM = Cast<UFSMComponent>(AIController->GetBrainComponent()))
		{
			// Patrol path
			std::vector<FVector> PatrolPath;
			for (AActor* Waypoint : PatrolWaypoints)
			{
				if (Waypoint) 
				{
					PatrolPath.push_back(Waypoint->GetActorLocation());
				}
			}

			// States
			auto Patrol = std::make_unique<PatrolState>(PatrolPath);
			auto Chase = std::make_unique<ChaseState>();
			auto Search = std::make_unique<SearchState>();

			auto pPatrol = Patrol.get();
			auto pChase = Chase.get();
			auto pSearch = Search.get();

			FSM->AddState(std::move(Patrol));
			FSM->AddState(std::move(Chase));
			FSM->AddState(std::move(Search));

			UBlackboardComponent* BB = AIController->GetBlackboardComponent();

			BB->SetValueAsObject(TargetPlayerKey, Thief);
			// Transitions
			auto IsTargetVisible = [BB, AIController, this]() -> bool {
				AActor* Target = Cast<AActor>(BB->GetValueAsObject(TargetPlayerKey));
				if (!Target || !Agent) return false;

				return AIController->LineOfSightTo(Target);
				};

			auto IsTargetNotVisible = [IsTargetVisible]() -> bool {
				return !IsTargetVisible();
				};

			auto IsSearchingTooLong = [BB]() -> bool {
				return BB->GetValueAsFloat(SearchTimeKey) > 5.0f; 
				};

			FSM->AddTransition(pPatrol, pChase, IsTargetVisible);
			FSM->AddTransition(pChase, pSearch, IsTargetNotVisible);
			FSM->AddTransition(pSearch, pChase, IsTargetVisible);
			FSM->AddTransition(pSearch, pPatrol, IsSearchingTooLong);

			AIController->RunFiniteStateMachine();
		}
	}
}

// Called every frame
void ALevel_FSM::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (Thief)
	{

		if (AAIController* ThiefAIController = Cast<AAIController>(Thief->GetController()))
		{
			//auto pos = GetMouseWorldPos();
			FVector pos{ MouseTarget.Position, 0 };
			//if(pos.has_value())
				ThiefAIController->MoveToLocation(pos);
		}
	}
	// Debug rendering
	if (Agent)
	{
		if (AGameAIController* AIController = Cast<AGameAIController>(Agent->GetController()))
		{
			if (UFSMComponent* FSM = Cast<UFSMComponent>(AIController->GetBrainComponent()))
			{
				ImGui::Begin("Guard AI Debug");

				GameAI::FSM::State* CurrentState = FSM->GetCurrentState();
				if (CurrentState)
				{
					ImGui::Text("Current State: %s", CurrentState->GetName().c_str());
				}
				else
				{
					ImGui::TextColored(ImVec4(1, 0, 0, 1), "Current State: NONE");
				}

				ImGui::Separator();

				if (UBlackboardComponent* BB = AIController->GetBlackboardComponent())
				{
					AActor* Target = Cast<AActor>(BB->GetValueAsObject(TargetPlayerKey));
					if (Target)
					{
						ImGui::Text("Target: %s", TCHAR_TO_UTF8(*Target->GetName()));

						float Distance = FVector::Distance(Agent->GetActorLocation(), Target->GetActorLocation());
						ImGui::Text("Distance to Target: %.2f", Distance);
					}
					else
					{
						ImGui::Text("Target: None");
					}

					float SearchTime = BB->GetValueAsFloat(SearchTimeKey);
					if (SearchTime > 4.0f) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 0.5f, 0, 1));
					ImGui::Text("Search Timer: %.2f / 5.00", SearchTime);
					if (SearchTime > 4.0f) ImGui::PopStyleColor();

					FVector LastKnown = BB->GetValueAsVector(LastKnownPosKey);
					ImGui::Text("Last Known Pos: X:%.0f Y:%.0f", LastKnown.X, LastKnown.Y);
				}

				ImGui::End();
			}
		}
	}
}


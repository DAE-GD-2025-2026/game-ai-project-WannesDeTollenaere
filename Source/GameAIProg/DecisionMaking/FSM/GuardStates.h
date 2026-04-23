#pragma once
#include "FSMComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "DecisionMaking/GameAIController.h"
#include "NavigationSystem.h" 
#include "Movement/SteeringBehaviors/Steering/SteeringBehaviors.h"
#include "Movement/SteeringBehaviors/SteeringAgent.h"
#include <memory>
#include <vector>

// BLACKBOARD KEYS
const FName TargetPlayerKey = "TargetPlayer";
const FName LastKnownPosKey = "LastKnownPosition";
const FName SearchTimeKey = "SearchTimer";

class PatrolState : public GameAI::FSM::State
{
public:
	PatrolState(const std::vector<FVector>& Path) : PatrolPath(Path) {}

	void Enter(AGameAIController* Controller, UBlackboardComponent* Blackboard) override {
		if (PatrolPath.empty()) return;

		Controller->MoveToLocation(PatrolPath[CurrentPointIdx], 50.0f);
	}

	void Update(float DeltaTime, AGameAIController* Controller, UBlackboardComponent* Blackboard) override {
		if (PatrolPath.empty()) return;

		APawn* Agent = Controller->GetPawn();
		if (!Agent) return;

		float DistanceToWaypoint = FVector::Distance(Agent->GetActorLocation(), PatrolPath[CurrentPointIdx]);

		if (DistanceToWaypoint < 200.f) {
			CurrentPointIdx = (CurrentPointIdx + 1) % PatrolPath.size();

			Controller->MoveToLocation(PatrolPath[CurrentPointIdx], 50.0f);
		}
	}
	virtual const  std::string GetName() const override { return "Patrol"; }
private:
	std::vector<FVector> PatrolPath;
	int CurrentPointIdx = 0;
};


class ChaseState : public GameAI::FSM::State
{
public:
	void Enter(AGameAIController* Controller, UBlackboardComponent* Blackboard) override {
		if (AActor* Target = Cast<AActor>(Blackboard->GetValueAsObject(TargetPlayerKey))) {
			Controller->MoveToActor(Target, 100.0f); 
		}
	}

	void Update(float DeltaTime, AGameAIController* Controller, UBlackboardComponent* Blackboard) override {
		if (AActor* Target = Cast<AActor>(Blackboard->GetValueAsObject(TargetPlayerKey))) {
			Blackboard->SetValueAsVector(LastKnownPosKey, Target->GetActorLocation());
			Controller->MoveToActor(Target, 100.0f);
		}

	}

	void Exit(AGameAIController* Controller, UBlackboardComponent* Blackboard) override {
		Controller->StopMovement(); 
	}

	virtual const std::string GetName() const override { return "Chase"; }
};


class SearchState : public GameAI::FSM::State
{
public:
	SearchState() {
		WanderBehavior = std::make_unique<Wander>();

		WanderBehavior->SetWanderOffset(600.f);
		WanderBehavior->SetWanderRadius(400.f);
	}

	void Enter(AGameAIController* Controller, UBlackboardComponent* Blackboard) override {
		Blackboard->SetValueAsFloat(SearchTimeKey, 0.0f);
		bReachedLastKnown = false;

		ASteeringAgent* Agent = Cast<ASteeringAgent>(Controller->GetPawn());
		if (Agent) {
			Agent->SetSteeringBehavior(nullptr);
		}

		FVector LastPos = Blackboard->GetValueAsVector(LastKnownPosKey);
		Controller->MoveToLocation(LastPos, 50.0f);
	}

	void Update(float DeltaTime, AGameAIController* Controller, UBlackboardComponent* Blackboard) override {
		float Timer = Blackboard->GetValueAsFloat(SearchTimeKey);
		Blackboard->SetValueAsFloat(SearchTimeKey, Timer + DeltaTime);

		ASteeringAgent* Agent = Cast<ASteeringAgent>(Controller->GetPawn());
		if (!Agent) return;

		if (!bReachedLastKnown) {
			FVector LastPos = Blackboard->GetValueAsVector(LastKnownPosKey);

			if (FVector::Distance(Agent->GetActorLocation(), LastPos) < 100.0f) {
				bReachedLastKnown = true;

				Agent->SetSteeringBehavior(WanderBehavior.get());
			}
		}
	}

	void Exit(AGameAIController* Controller, UBlackboardComponent* Blackboard) override {
		if (ASteeringAgent* Agent = Cast<ASteeringAgent>(Controller->GetPawn())) {
			Agent->SetSteeringBehavior(nullptr);
		}
	}

	virtual const  std::string GetName() const override { return "Search"; }

private:
	bool bReachedLastKnown = false;
	std::unique_ptr<Wander> WanderBehavior;
};
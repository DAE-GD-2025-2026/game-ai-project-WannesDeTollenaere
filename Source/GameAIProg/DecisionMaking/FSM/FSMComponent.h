// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <vector>
#include <memory>
#include <functional>
#include <map>
#include <string>

#include "CoreMinimal.h"
#include "BrainComponent.h"
#include "FSMComponent.generated.h"

class UBlackboardComponent;
class AGameAIController;

namespace GameAI::FSM
{
	class State
	{
	public:
		virtual ~State() = default;

		virtual const std::string GetName() const { return "Undefined"; };

		virtual void Enter(AGameAIController* Controller, UBlackboardComponent* Blackboard) {}
		virtual void Update(float DeltaTime, AGameAIController* Controller, UBlackboardComponent* Blackboard) {}
		virtual void Exit(AGameAIController* Controller, UBlackboardComponent* Blackboard) {}
	};

	struct Transition
	{
		State* ToState;
		std::function<bool()> Condition;
	};

	class FSM
	{
	public:

		void AddState(std::unique_ptr<State>&& NewState)
		{
			States.push_back(std::move(NewState));
		}

		void AddTransition(State* From, State* To, std::function<bool()> Condition)
		{
			Transitions[From].push_back({ To, Condition });
		}

		void Update(float DeltaTime, AGameAIController* Controller, UBlackboardComponent* Blackboard)
		{
			if (States.empty()) return;

			// first state
			if (!CurrentState) {
				CurrentState = States.front().get();
				CurrentState->Enter(Controller, Blackboard);
			}

			// check transition
			for (const auto& Transition : Transitions[CurrentState]) {
				if (Transition.Condition()) {
					CurrentState->Exit(Controller, Blackboard);
					CurrentState = Transition.ToState;
					CurrentState->Enter(Controller, Blackboard);
					return; // state changed
				}
			}

			// update active state
			CurrentState->Update(DeltaTime, Controller, Blackboard);
		}

		State* GetCurrentState() const { return CurrentState; }

	private:
		std::vector<std::unique_ptr<State>> States;
		std::map<State*, std::vector<Transition>> Transitions;
		State* CurrentState = nullptr;
	};
}

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GAMEAIPROG_API UFSMComponent : public UBrainComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UFSMComponent();

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;
	
	virtual void StartLogic() override;
	virtual void StopLogic(const FString& Reason) override;
	
	virtual bool IsRunning() const override; 
	
	void AddState(std::unique_ptr<GameAI::FSM::State>&& NewState);
	void AddTransition(GameAI::FSM::State* From, GameAI::FSM::State* To, std::function<bool()> EvalFunc) const;

	GameAI::FSM::State* GetCurrentState() const
	{
		return FSMInstance ? FSMInstance->GetCurrentState() : nullptr;
	}
		
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

private:
	std::unique_ptr<GameAI::FSM::FSM> FSMInstance;
	bool bIsRunning{false};
};

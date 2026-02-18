// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include <memory> 
#include "CombinedSteeringBehaviors.h"
#include "GameAIProg/Shared/Level_Base.h"
#include "GameAIProg/Movement/SteeringBehaviors/Steering/SteeringBehaviors.h"
#include "GameAIProg/Movement/SteeringBehaviors/SteeringAgent.h"
#include "Level_CombinedSteering.generated.h"

UCLASS()
class GAMEAIPROG_API ALevel_CombinedSteering : public ALevel_Base
{
	GENERATED_BODY()

public:

	ALevel_CombinedSteering();
	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;

	virtual void BeginDestroy() override;

private:
	bool UseMouseTarget = false;
	bool CanDebugRender = false;

	UPROPERTY()
	ASteeringAgent* DrunkAgent = nullptr;

	UPROPERTY()
	ASteeringAgent* EvadingAgent = nullptr;

	// Drunk angents blended
	std::unique_ptr<Seek> DrunkSeekBehavior;
	std::unique_ptr<Wander> DrunkWanderBehavior;
	std::unique_ptr<BlendedSteering> DrunkBlendedBehavior;

	// evading agents priuority
	std::unique_ptr<Evade> EvadingEvadeBehavior;
	std::unique_ptr<Wander> EvadingWanderBehavior;
	std::unique_ptr<PrioritySteering> EvadingPriorityBehavior;
};
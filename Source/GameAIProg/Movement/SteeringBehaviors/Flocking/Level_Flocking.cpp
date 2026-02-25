// Fill out your copyright notice in the Description page of Project Settings.


#include "Level_Flocking.h"
#include "./Movement/SteeringBehaviors/Steering/SteeringBehaviors.h"

// Sets default values
ALevel_Flocking::ALevel_Flocking()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ALevel_Flocking::BeginPlay()
{
	Super::BeginPlay();

	TrimWorld->SetTrimWorldSize(1500.f);
	TrimWorld->bShouldTrimWorld = false;

	pAgentToEvade = GetWorld()->SpawnActor<ASteeringAgent>(SteeringAgentClass, FVector::ZeroVector, FRotator::ZeroRotator);

	if (pAgentToEvade)
	{
		pEvadeAgentBehavior = std::make_unique<Seek>();
		pAgentToEvade->SetSteeringBehavior(pEvadeAgentBehavior.get());

		pAgentToEvade->SetMaxLinearSpeed(400.f);
		pAgentToEvade->SetDebugRenderingEnabled(true);
	}

	pFlock = TUniquePtr<Flock>(
		new Flock(
			GetWorld(),
			SteeringAgentClass,
			FlockSize,
			TrimWorld->GetTrimWorldSize(),
			pAgentToEvade,
			true)
	);
}

// Called every frame
void ALevel_Flocking::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	pFlock->ImGuiRender(WindowPos, WindowSize);
	pFlock->Tick(DeltaTime);
	pFlock->RenderDebug();
	
	//FSteeringParams target{};
	//target.Position = pFlock->GetAverageNeighborPos();
	//pFlock->SetTarget_Seek(target );

	if (bUseMouseTarget && pEvadeAgentBehavior && pAgentToEvade)
	{
		pEvadeAgentBehavior->SetTarget(MouseTarget);
	}
}


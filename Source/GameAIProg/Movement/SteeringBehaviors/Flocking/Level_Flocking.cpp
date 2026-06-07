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

	TrimWorld->SetTrimWorldSize(1000.f);
	TrimWorld->bShouldTrimWorld = false;

	pAgentToEvade = GetWorld()->SpawnActor<ASteeringAgent>(SteeringAgentClass, FVector::ZeroVector, FRotator::ZeroRotator);

	if (pAgentToEvade)
	{
		pEvadeAgentBehavior = std::make_unique<Seek>();
		pAgentToEvade->SetSteeringBehavior(pEvadeAgentBehavior.get());

		pAgentToEvade->SetMaxLinearSpeed(400.f);
		pAgentToEvade->SetDebugRenderingEnabled(true);
	}

	AddFlock();
}

// Called every frame
void ALevel_Flocking::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!Flocks.IsEmpty() && Flocks[0])
	{
		Flocks[0]->ImGuiRender(WindowPos, WindowSize, Flocks.Num(),
			[this]() { AddFlock(); },
			[this]() { RemoveFlock(); });
	}

	for (int i = 0; i < Flocks.Num(); ++i)
	{
		if (!Flocks[i]) continue;

		Flocks[i]->Tick(DeltaTime);
		Flocks[i]->RenderDebug();
	}

	if (bUseMouseTarget && pEvadeAgentBehavior && pAgentToEvade)
	{
		pEvadeAgentBehavior->SetTarget(MouseTarget);
	}
}

void ALevel_Flocking::AddFlock()
{
	const uint8 hue = static_cast<uint8>((Flocks.Num() * 67) % 256);
	const FLinearColor flockColor = FLinearColor::MakeFromHSV8(hue, 200, 255);

	Flocks.Add(MakeUnique<Flock>(
		GetWorld(),
		SteeringAgentClass,
		FlockSize,
		TrimWorld->GetTrimWorldSize(),
		pAgentToEvade,
		true,
		flockColor));

	RelinkFlockAvoidance();
}

void ALevel_Flocking::RemoveFlock()
{
	if (Flocks.Num() <= 1) return;

	Flocks.RemoveAt(Flocks.Num() - 1);
	RelinkFlockAvoidance();
}

void ALevel_Flocking::RelinkFlockAvoidance()
{
	for (int i = 0; i < Flocks.Num(); ++i)
	{
		if (!Flocks[i]) continue;

		TArray<Flock*> otherFlocks;
		for (int j = 0; j < Flocks.Num(); ++j)
		{
			if (i != j && Flocks[j])
			{
				otherFlocks.Add(Flocks[j].Get());
			}
		}

		Flocks[i]->SetOtherFlocks(otherFlocks);
	}
}

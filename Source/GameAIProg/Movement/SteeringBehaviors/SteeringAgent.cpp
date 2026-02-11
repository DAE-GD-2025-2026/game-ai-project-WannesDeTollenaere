// Fill out your copyright notice in the Description page of Project Settings.

#include "SteeringAgent.h"
#include "DrawDebugHelpers.h"


// Sets default values
ASteeringAgent::ASteeringAgent()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ASteeringAgent::BeginPlay()
{
	Super::BeginPlay();
	m_OriginalMaxSpeed = GetMaxLinearSpeed();
}

void ASteeringAgent::BeginDestroy()
{
	Super::BeginDestroy();
}

// Called every frame
void ASteeringAgent::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (SteeringBehavior)
	{
		SteeringOutput output = SteeringBehavior->CalculateSteering(DeltaTime, *this);
		AddMovementInput(FVector{ output.LinearVelocity, 0.f });

		// TODO: implement angular velocity handling

		// DEBUG LINES
		if (GetDebugRenderingEnabled())
		{
			FVector ActorLocation = GetActorLocation();

			// green: LinearVelocity 
			FVector DesiredLinearVelocity = FVector(output.LinearVelocity, 0.f);
			DrawDebugLine(GetWorld(), ActorLocation, ActorLocation + DesiredLinearVelocity, FColor::Green, false, -1.0f, 0, 2.0f);

			// blue: angularvelocity
			FVector DesiredAngularVector = FVector::UpVector * output.AngularVelocity * 100.0f;
			DrawDebugLine(GetWorld(), ActorLocation, ActorLocation + DesiredAngularVector, FColor::Blue, false, -1.0f, 0, 2.0f);

			// purple actual velocity
			FVector ActualVelocity = GetVelocity();
			DrawDebugLine(GetWorld(), ActorLocation, ActorLocation + ActualVelocity, FColor::Purple, false, -1.0f, 0, 2.0f);
		}
	}
}

// Called to bind functionality to input
void ASteeringAgent::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void ASteeringAgent::SetSteeringBehavior(ISteeringBehavior* NewSteeringBehavior)
{
	SteeringBehavior = NewSteeringBehavior;
}


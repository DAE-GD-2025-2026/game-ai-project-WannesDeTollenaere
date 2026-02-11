#include "SteeringBehaviors.h"
#include "GameAIProg/Movement/SteeringBehaviors/SteeringAgent.h"

//SEEK

SteeringOutput Seek::CalculateSteering(float DeltaT, ASteeringAgent& Agent)
{
	SteeringOutput steering{};

	steering.LinearVelocity = Target.Position - Agent.GetPosition();

	return steering;
}

// FLEE

SteeringOutput Flee::CalculateSteering(float DeltaT, ASteeringAgent& Agent)
{
	SteeringOutput steering{};

	steering.LinearVelocity =  Agent.GetPosition()- Target.Position;

	return steering;
}


// ARRIVE
SteeringOutput Arrive::CalculateSteering(float DeltaT, ASteeringAgent& Agent)
{
	SteeringOutput steering{ Seek::CalculateSteering(DeltaT, Agent) };

	const float dist = steering.LinearVelocity.Length();

	if (dist > m_SlowRadius)
		Agent.SetMaxLinearSpeed(Agent.GetOriginalMaxSpeed());
	else if (dist < m_TargetRadius)
		Agent.SetMaxLinearSpeed(0.f);
	else
		Agent.SetMaxLinearSpeed(((dist-m_TargetRadius) / (m_SlowRadius - m_TargetRadius)) * Agent.GetOriginalMaxSpeed());

	FVector Center = FVector(Agent.GetPosition().X, Agent.GetPosition().Y, 0.0f);

	//Drawcircles
	DrawDebugCircle(
		Agent.GetWorld(),
		Center,                     
		m_SlowRadius,             
		32,                         // Segments
		FColor::Blue,               // color
		false,                      
		-1.0f,                      // LifeTime
		0,                          // DepthPriority
		2.0f,                       // Thickness
		FVector(1.0f, 0.0f, 0.0f),  // X-axis
		FVector(0.0f, 1.0f, 0.0f),  // y-axis
		false                       
	);
	DrawDebugCircle(
		Agent.GetWorld(),
		Center,
		m_TargetRadius,
		32,                         // Segments
		FColor::Red,               // color
		false,
		-1.0f,                      // LifeTime
		0,                          // DepthPriority
		2.0f,                       // Thickness
		FVector(1.0f, 0.0f, 0.0f),  // X-axis
		FVector(0.0f, 1.0f, 0.0f),  // y-axis
		false
	);
	return steering;
}

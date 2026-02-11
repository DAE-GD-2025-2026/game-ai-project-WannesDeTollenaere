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

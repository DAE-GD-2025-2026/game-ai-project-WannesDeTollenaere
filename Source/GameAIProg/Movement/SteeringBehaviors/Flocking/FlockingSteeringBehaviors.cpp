#include "FlockingSteeringBehaviors.h"
#include "Flock.h"
#include "../SteeringAgent.h"
#include "../SteeringHelpers.h"

// ******************
// COHESION (FLOCKING)
SteeringOutput Cohesion::CalculateSteering(float deltaT, ASteeringAgent& pAgent)
{
    if (pFlock->GetNrOfNeighbors() == 0)
        return SteeringOutput{};

    Target.Position = pFlock->GetAverageNeighborPos();
    return Seek::CalculateSteering(deltaT, pAgent);
}

// ******************
// SEPARATION (FLOCKING)
SteeringOutput Separation::CalculateSteering(float deltaT, ASteeringAgent& pAgent)
{
    SteeringOutput steering{};
    const auto& neighbors = pFlock->GetNeighbors();
    int nrNeighbors = pFlock->GetNrOfNeighbors();

    if (nrNeighbors == 0)
        return steering;

    for (int i = 0; i < nrNeighbors; ++i)
    {
        FVector2D vectorToAgent = pAgent.GetPosition() - neighbors[i]->GetPosition();
        float distance = vectorToAgent.Length();

         steering.LinearVelocity += (vectorToAgent.GetSafeNormal() / distance);

    }

    return steering;
}

// ******************
// VELOCITY MATCH (FLOCKING)
SteeringOutput VelocityMatch::CalculateSteering(float deltaT, ASteeringAgent& pAgent)
{
    SteeringOutput steering{};
    if (pFlock->GetNrOfNeighbors() == 0)
        return steering;

    steering.LinearVelocity = pFlock->GetAverageNeighborVelocity();
    return steering;
}

// ******************
// FLOCK AVOIDANCE (FLOCKING VARIATION)
SteeringOutput FlockAvoidance::CalculateSteering(float deltaT, ASteeringAgent& pAgent)
{
    SteeringOutput steering{};
    const auto& otherFlockNeighbors = pFlock->GetOtherFlockNeighbors();

    for (ASteeringAgent* pOtherAgent : otherFlockNeighbors)
    {
        FVector2D vectorToAgent = pAgent.GetPosition() - pOtherAgent->GetPosition();
        float distance = vectorToAgent.Length();

        steering.LinearVelocity += (vectorToAgent.GetSafeNormal() / distance);
    }

    return steering;
}
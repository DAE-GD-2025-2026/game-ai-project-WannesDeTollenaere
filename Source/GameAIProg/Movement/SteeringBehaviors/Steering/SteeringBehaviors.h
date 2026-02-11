#pragma once

#include <Movement/SteeringBehaviors/SteeringHelpers.h>
#include "Kismet/KismetMathLibrary.h"

class ASteeringAgent;

// SteeringBehavior base, all steering behaviors should derive from this.
class ISteeringBehavior
{
public:
	ISteeringBehavior() = default;
	virtual ~ISteeringBehavior() = default;

	// Override to implement your own behavior
	virtual SteeringOutput CalculateSteering(float DeltaT, ASteeringAgent & Agent) = 0;

	void SetTarget(const FTargetData& NewTarget) { Target = NewTarget; }
	
	template<class T, std::enable_if_t<std::is_base_of_v<ISteeringBehavior, T>>* = nullptr>
	T* As()
	{ return static_cast<T*>(this); }

protected:
	FTargetData Target;
};

// --- SEEK ---
class Seek : public ISteeringBehavior
{
public:
	Seek() = default;

	virtual SteeringOutput CalculateSteering(float DeltaT, ASteeringAgent& Agent) override;
};
// --- FLEE ---
class Flee : public ISteeringBehavior
{
public:
	Flee() = default;

	SteeringOutput CalculateSteering(float DeltaT, ASteeringAgent& Agent) override;
};
// --- ARRIVE ---
class Arrive : public Seek
{
public:
	Arrive() = default;

	SteeringOutput CalculateSteering(float DeltaT, ASteeringAgent& Agent) override;

private:
	float m_SlowRadius{ 500.f };
	float m_TargetRadius{100.f};
};

// --- FACE ---
class Face : public ISteeringBehavior
{
public:
	Face() = default;

	SteeringOutput CalculateSteering(float DeltaT, ASteeringAgent& Agent) override;
private:
	const float m_SlowDownAngle = 45.0f; 
	const float m_TargetRadius = 1.0f;
};
// --- Pursuit ---
class Pursuit : public ISteeringBehavior
{
public:
	Pursuit() = default;

	SteeringOutput CalculateSteering(float DeltaT, ASteeringAgent& Agent) override;
private:
	const float m_TimeAhead = 1.0f;
};
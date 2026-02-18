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

	if (Agent.GetDebugRenderingEnabled())
	{
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
	}
	return steering;
}

// FACE
SteeringOutput Face::CalculateSteering(float DeltaT, ASteeringAgent& Agent)
{
	SteeringOutput Output;

	const FVector Direction = FVector{ Target.Position,0.f } - Agent.GetActorLocation();

	if (Direction.IsNearlyZero())
	{
		return Output;
	}

	const float TargetYaw = Direction.Rotation().Yaw;

	const float DeltaYaw = FMath::FindDeltaAngleDegrees(Agent.GetRotation(), TargetYaw);
	const float AbsDelta = FMath::Abs(DeltaYaw);

	if (AbsDelta <= m_TargetRadius)
	{
		return Output;
	}

	const float MaxAngularSpeed = Agent.GetMaxAngularSpeed();
	float TargetSpeed = MaxAngularSpeed;

	if (AbsDelta < m_SlowDownAngle)
	{
		TargetSpeed = MaxAngularSpeed * (AbsDelta / m_SlowDownAngle);
	}

	Output.AngularVelocity = TargetSpeed * FMath::Sign(DeltaYaw);
	Output.LinearVelocity = FVector2D::Zero();

	return Output;
}

// PURSUIT
SteeringOutput Pursuit::CalculateSteering(float DeltaT, ASteeringAgent& Agent)
{
	SteeringOutput steering{};

	FVector2D targetPos = (Target.Position + Target.LinearVelocity * m_TimeAhead);
	steering.LinearVelocity = targetPos - Agent.GetPosition();
	Agent.SetMaxLinearSpeed(200.f);

	if (Agent.GetDebugRenderingEnabled())
	{
		DrawDebugCircle(
			Agent.GetWorld(),
			FVector{ targetPos,0.f },
			20.f,
			32,                         // Segments
			FColor::Magenta,               // color
			false,
			-1.0f,                      // LifeTime
			0,                          // DepthPriority
			2.0f,                       // Thickness
			FVector(1.0f, 0.0f, 0.0f),  // X-axis
			FVector(0.0f, 1.0f, 0.0f),  // y-axis
			false
		);
	}
	return steering;
}

//EVADE
SteeringOutput Evade::CalculateSteering(float DeltaT, ASteeringAgent& Agent)
{	
	if ((Target.Position - Agent.GetPosition()).Length() > m_EvasionRadius)
	{
		SteeringOutput Out = {};
		Out.IsValid = false; 
		return Out;
	}

	SteeringOutput steering{ Pursuit::CalculateSteering(DeltaT, Agent) };
	steering.LinearVelocity = -steering.LinearVelocity;

	return steering;
}

SteeringOutput Wander::CalculateSteering(float DeltaT, ASteeringAgent& Agent)
{

	const FVector AgentLocation = Agent.GetActorLocation();
	FVector AgentForward = Agent.GetActorForwardVector();

	AgentForward.Z = 0;
	AgentForward.Normalize();

	const FVector CircleCenter = AgentLocation + (AgentForward * m_Offset);

	float RandomBinomial = (FMath::FRand() * 2.0f) - 1.0f; // -1..1
	m_WanderAngle += RandomBinomial * m_MaxAngleChange;

	float X = m_Radius * FMath::Cos(m_WanderAngle);
	float Y = m_Radius * FMath::Sin(m_WanderAngle);

	FVector CirclePoint;
	
	CirclePoint.X = CircleCenter.X + X;
	CirclePoint.Y = CircleCenter.Y + Y;
	CirclePoint.Z = CircleCenter.Z; 

	
	Target.Position = FVector2D{ CirclePoint.X,CirclePoint.Y };

	
	if (Agent.GetDebugRenderingEnabled())
	{
		DrawDebugCircle(Agent.GetWorld(), CircleCenter, m_Radius, 32, FColor::Red, false, -1.f, 0, 2.f, FVector(1, 0, 0), FVector(0, 1, 0), false);
		//DrawDebugLine(Agent.GetWorld(), CircleCenter, CirclePoint, FColor::Blue, false, -1.f, 0, 2.f);
		DrawDebugPoint(Agent.GetWorld(), CirclePoint, 10.f, FColor::Green, false, -1.f);
	}

	return Seek::CalculateSteering(DeltaT, Agent);
}
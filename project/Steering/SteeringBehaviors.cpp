//Precompiled Header [ALWAYS ON TOP IN CPP]
#include "../stdafx.h"

//Includes
#include "SteeringBehaviors.h"

void ISteeringBehavior::SetRunning(bool isRunning)
{
    m_IsRunning = isRunning;
}

void ISteeringBehavior::RunStaminaCheck(const AgentInfo &agent)
{
    if (!m_IsRunning) return;
    if (agent.Stamina <= 0.f)
    {
        m_IsRunning = false;
    }
}

//SEEK
//****
SteeringOutput Seek::CalculateSteering(const AgentInfo &agent)
{
    RunStaminaCheck(agent);
    SteeringOutput steering = {};
    steering.RunMode = m_IsRunning;
    if (Elite::Distance(m_Target.Position, agent.Position) < 0.5f)
    {
        steering.LinearVelocity = Elite::ZeroVector2;
        return steering;
    }
    steering.LinearVelocity = m_Target.Position - agent.Position;
    steering.LinearVelocity.Normalize();
    steering.LinearVelocity *= agent.MaxLinearSpeed;

    return steering;
}

//FLEE
//****
SteeringOutput Flee::CalculateSteering(const AgentInfo &agent)
{
    RunStaminaCheck(agent);
    SteeringOutput steering = Seek::CalculateSteering(agent);
    steering.RunMode = m_IsRunning;
    steering.LinearVelocity *= -1;
    return steering;
}

//FACE
//****
SteeringOutput Face::CalculateSteering(const AgentInfo &agent)
{
    SteeringOutput steering = {};
    steering.AutoOrient = false;
    steering.RunMode = m_IsRunning;

    const Elite::Vector2 direction = m_Target.Position - agent.Position;
    const float targetOrientation = Elite::VectorToOrientation(direction);
    const float currentOrientation = agent.Orientation;
    float angleDifference = targetOrientation - currentOrientation;

    // Normalize the angle to the range [-pi, pi] so the shortest path is taken
    while (angleDifference > M_PI) angleDifference -= 2 * M_PI;
    while (angleDifference < -M_PI) angleDifference += 2 * M_PI;
    if (abs(angleDifference) <= Elite::ToRadians(5.f))
    {
        steering.AngularVelocity = 0.f;
        return steering;
    }

    const int sign = (angleDifference > 0) ? 1 : -1;

    steering.AngularVelocity = sign * agent.MaxAngularSpeed;

    return steering;
}

SteeringOutput FleeWhileFacing::CalculateSteering(const AgentInfo &agent)
{
    RunStaminaCheck(agent);
    Face faceSteering{};
    faceSteering.SetTarget(m_Target);
    SteeringOutput faceSteeringResult = faceSteering.CalculateSteering(agent);

    SteeringOutput fleeSteering = Flee::CalculateSteering(agent);

    SteeringOutput steering = fleeSteering;
    steering.AngularVelocity = faceSteeringResult.AngularVelocity;
    steering.RunMode = m_IsRunning;
    steering.AutoOrient = false;
    return steering;
}

//PURSUIT
//****
SteeringOutput Pursuit::CalculateSteering(const AgentInfo &agent)
{
    RunStaminaCheck(agent);
    const float distanceToTarget = Elite::Distance(agent.Position, m_Target.Position);
    const float expectedTime = distanceToTarget / agent.MaxLinearSpeed;
    const Elite::Vector2 predictedTargetPos = m_Target.Position + m_Target.LinearVelocity * expectedTime;
    m_Target.Position = predictedTargetPos;

    SteeringOutput steering = Seek::CalculateSteering(agent);
    steering.RunMode = m_IsRunning;

    return steering;
}

Evade::Evade(float evasionRadius): Pursuit(),m_EvasionRadius(evasionRadius) {}

SteeringOutput Evade::CalculateSteering(const AgentInfo &agent)
{
    RunStaminaCheck(agent);
    const float distanceToTarget = Elite::Distance(agent.Position, m_Target.Position);

    if (distanceToTarget > m_EvasionRadius)
    {
        SteeringOutput steering = {};
        return steering;
    }
    SteeringOutput steering = Pursuit::CalculateSteering(agent);
    steering.RunMode = m_IsRunning;

    steering.LinearVelocity *= -1;
    return steering;
}

SteeringOutput Wander::CalculateSteering(const AgentInfo &agent)
{
    RunStaminaCheck(agent);
    const float randomAngle = (m_MaxAngleChange * static_cast<float>(rand()) / RAND_MAX)
                              - (m_MaxAngleChange * static_cast<float>(rand()) / RAND_MAX) + m_WanderAngle;
    m_WanderAngle = randomAngle;
    const Elite::Vector2 circleCenter = agent.Position + m_OffsetDistance * agent.LinearVelocity.GetNormalized();
    const Elite::Vector2 targetPos = circleCenter + m_Radius * Elite::Vector2(sin(randomAngle), cos(randomAngle));
    m_Target = targetPos;
    SteeringOutput steering = Seek::CalculateSteering(agent);
    steering.RunMode = m_IsRunning;

    return steering;
}

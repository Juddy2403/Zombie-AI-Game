#include "../stdafx.h"
#include "CombinedSteeringBehaviors.h"

BlendedSteering::BlendedSteering(const std::vector<WeightedBehavior> &weightedBehaviors)
    : m_WeightedBehaviors(weightedBehaviors)
{
}

void BlendedSteering::SetTarget(const TargetData &target)
{
    for (auto &behavior : m_WeightedBehaviors)
    {
        // target should not be set for wander
        //if (reinterpret_cast<Wander*>(behavior.pBehavior) != nullptr) continue;

        behavior.pBehavior->SetTarget(target);
    }
}

void BlendedSteering::SetRunning(bool isRunning)
{
    for (auto &behavior : m_WeightedBehaviors)
    {
        behavior.pBehavior->SetRunning(isRunning);
    }
};

void BlendedSteering::SetTargetForIdx(const int idx, const TargetData &target) const
{
    assert(idx < m_WeightedBehaviors.size() && "Idx invalid for set target! - blended steering");
    m_WeightedBehaviors[idx].pBehavior->SetTarget(target);
}

void BlendedSteering::ResetWeight()
{
    for (auto &behavior : m_WeightedBehaviors)
    {
        behavior.weight = 0.f;
    }
}

//****************
//BLENDED STEERING
SteeringOutput BlendedSteering::CalculateSteering(const AgentInfo &agent)
{
    SteeringOutput blendedSteering = {};
    //This is for the extra attributes other than velocity (eg: isRunning).
    //They'll be taken from the max weight steering
    SteeringOutput outputSteering = {};
    float maxWeight = 0.f;
    float totalWeight = 0.f;
    for (const WeightedBehavior &behavior: m_WeightedBehaviors)
    {
        const SteeringOutput steering = behavior.pBehavior->CalculateSteering(agent);
        if (behavior.weight > maxWeight)
        {
            maxWeight = behavior.weight;
            outputSteering = steering;
        }
        blendedSteering.LinearVelocity += steering.LinearVelocity * behavior.weight;
        blendedSteering.AngularVelocity += steering.AngularVelocity * behavior.weight;
        totalWeight += behavior.weight;
    }
    if (totalWeight > 0.f)
    {
        blendedSteering /= totalWeight;
    }
    blendedSteering.AngularVelocity = blendedSteering.AngularVelocity > 1.f ? 1.f : blendedSteering.AngularVelocity < -1.f ? -1.f : blendedSteering.AngularVelocity;
    outputSteering.AngularVelocity = blendedSteering.AngularVelocity * agent.MaxAngularSpeed;
    outputSteering.LinearVelocity = blendedSteering.LinearVelocity.GetNormalized() * agent.MaxLinearSpeed;

    return outputSteering;
}

//*****************
//PRIORITY STEERING
SteeringOutput PrioritySteering::CalculateSteering(const AgentInfo &agent)
{
    assert((m_ValidIdx < m_PriorityBehaviors.size() && m_ValidIdx >=0) && "Idx invalid for steering! - priority steering");

    SteeringOutput steering = {};
    steering = m_PriorityBehaviors[m_ValidIdx]->CalculateSteering(agent);
    return steering;
}

void PrioritySteering::SetRunningForIdx(const int idx, const bool isRunning) const
{
    assert((idx < m_PriorityBehaviors.size() && idx >=0) && "Idx invalid for steering! - priority steering SetRunningForIdx");
    m_PriorityBehaviors[idx]->SetRunning(isRunning);
}

void PrioritySteering::SetTargetForIdx(const int idx, const TargetData &target) const
{
    assert(idx < m_PriorityBehaviors.size() && "Idx invalid for set target! - blended steering");
    m_PriorityBehaviors[idx]->SetTarget(target);
}

ISteeringBehavior* PrioritySteering::GetBehaviorForIdx(int idx) const
{
    if (idx < 0 || idx >= m_PriorityBehaviors.size())
        throw std::out_of_range("Index out of range for priority behaviors");
    return m_PriorityBehaviors[idx];
}

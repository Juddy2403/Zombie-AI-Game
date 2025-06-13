#pragma once
#include "SteeringBehaviors.h"

//****************
//BLENDED STEERING
class BlendedSteering final : public ISteeringBehavior
{
public:
    struct WeightedBehavior
    {
        ISteeringBehavior *pBehavior = nullptr;
        float weight = 0.f;

        WeightedBehavior(ISteeringBehavior *const pBehavior, float weight) : pBehavior(pBehavior),weight(weight) {};
    };

    explicit BlendedSteering(const std::vector<WeightedBehavior> &weightedBehaviors);
    void SetTarget(const TargetData& target) override;
    void SetRunning(bool isRunning) override;

    void AddBehaviour(const WeightedBehavior &weightedBehavior) { m_WeightedBehaviors.push_back(weightedBehavior); }

    // set target for specific behavior
    void SetTargetForIdx(int idx, const TargetData &target) const;

    // sets all behaviors weight to 0
    void ResetWeight();

    SteeringOutput CalculateSteering(const AgentInfo &agent) override;

    // returns a reference to the weighted behaviors, can be used to adjust weighting. Is not intended to alter the behaviors themselves.
    std::vector<WeightedBehavior> &GetWeightedBehaviorsRef() { return m_WeightedBehaviors; }

private:
    std::vector<WeightedBehavior> m_WeightedBehaviors = {};
};

//*****************
//PRIORITY STEERING
class PrioritySteering final : public ISteeringBehavior
{
public:
    explicit PrioritySteering(const std::vector<ISteeringBehavior *> &priorityBehaviors) : m_PriorityBehaviors(priorityBehaviors) {}

    void AddBehaviour(ISteeringBehavior *const pBehavior) { m_PriorityBehaviors.push_back(pBehavior); }

    SteeringOutput CalculateSteering(const AgentInfo &agent) override;

    void SetValidSteeringIdx(int idx) { m_ValidIdx = idx; }
    void SetRunningForIdx(int idx, bool isRunning) const;

    // set target for specific behavior
    void SetTargetForIdx(int idx, const TargetData &target) const;
    [[nodiscard]] ISteeringBehavior* GetBehaviorForIdx(int idx) const;
    [[nodiscard]] int GetValidIdx() const { return m_ValidIdx; }
private:
    std::vector<ISteeringBehavior *> m_PriorityBehaviors = {};
    int m_ValidIdx = 0;
    // made private because targets need to be set on the individual behaviors, not the combined behavior
    using ISteeringBehavior::SetTarget;
};

#pragma once
#include <optional>
class Flee;
class Evade;
class Face;
class MapSearchSystem;
class IExamInterface;
struct SteeringOutput;
class FleeWhileFacing;
class Seek;
class Wander;
class PrioritySteering;
class BlendedSteering;

namespace Elite
{
    class BehaviorTree;
    class Blackboard;
}

class Agent final {
public:
    explicit Agent(IExamInterface* pInterface);
    ~Agent();
    void UpdateDebug(float dt);
    void Update(float dt);
    void RenderDebug(float dt) const;
    SteeringOutput GetSteeringOutput(float dt);
private:
    Elite::Vector2 m_MouseTarget;
    std::optional<Elite::Vector2> m_CurrTarget{};
	IExamInterface* m_pInterface = nullptr;
    MapSearchSystem* m_pMapSearch = nullptr;
    [[nodiscard]] Elite::Blackboard* CreateBlackboard() const;
    void CreateBehaviorTree();
    // Checks the FOV every frame for enemies
    void SetChaseData(float dt);
    void HandleRadarMode(float dt, SteeringOutput& steeringOutput) const;

    Elite::Blackboard* m_pBlackboard = nullptr;
    const float m_MaxChaseTime = 5.f;
    float m_CurrChaseTime = 0.f;

    Elite::BehaviorTree* m_pBehaviorTree = nullptr;
    BlendedSteering* m_pBlendedSeekAndWanderSteeringBehavior;
    BlendedSteering* m_pBlendedSeekAndEvadeSteeringBehavior;
    PrioritySteering* m_pPrioritySteeringBehavior;
    Wander* m_pWanderSteeringBehavior;
    Seek* m_pSeekSteeringBehavior;
    Face* m_pFaceBehavior;
    Evade* m_pEvadeBehavior;
    FleeWhileFacing* m_pFleeWhileFacingSteeringBehavior;
};




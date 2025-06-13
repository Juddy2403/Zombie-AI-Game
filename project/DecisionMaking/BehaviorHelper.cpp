#include "../stdafx.h"
#include "BehaviorHelper.h"
#include <cassert>
#include "BehaviorTree.h"
#include "Blackboard.h"
#include "IExamInterface.h"
#include "../IndexMaps.h"
#include "../Steering/CombinedSteeringBehaviors.h"
#include "../Steering/SteeringBehaviors.h"

void BT_Helpers::SetSteeringSeekTarget(Elite::Blackboard * const pBlackboard, Elite::Vector2 target, bool runMode,
    bool wanderMode)
{
    IExamInterface *pInterface;
    pBlackboard->GetData("interface", pInterface);
    assert(pInterface && "Interface not found in blackboard");

    PrioritySteering *pSteering;
    pBlackboard->GetData("prioritySteering", pSteering);
    assert(pSteering && "Steering not found in blackboard");
    const Elite::Vector2 nextPointInPath = pInterface->NavMesh_GetClosestPathPoint(target);
    int steeringIdx = wanderMode
                          ? AgentIndexMaps::SteeringSlot.at(SteeringBehaviorType::SeekAndWander)
                          : AgentIndexMaps::SteeringSlot.at(SteeringBehaviorType::Seek);
    pSteering->SetValidSteeringIdx(steeringIdx);
    // Run only when stamina is high enough
    if (runMode && pInterface->Agent_GetInfo().Stamina >= 9.5f) pSteering->SetRunningForIdx(steeringIdx, true);
    pSteering->SetTargetForIdx(steeringIdx, nextPointInPath);
}

void BT_Helpers::SetSteeringEvade(Elite::Blackboard * const pBlackboard, Elite::Vector2 target)
{
    IExamInterface *pInterface;
    pBlackboard->GetData("interface", pInterface);
    assert(pInterface && "Interface not found in blackboard");

    Elite::Vector2 lastEnemyPos;
    pBlackboard->GetData("lastEnemyPos", lastEnemyPos);

    PrioritySteering *pSteering;
    pBlackboard->GetData("prioritySteering", pSteering);
    assert(pSteering && "Steering not found in blackboard");
    const Elite::Vector2 nextPointInPath = pInterface->NavMesh_GetClosestPathPoint(target);
    int steeringIdx;
    // if there are enemies in FOV, evade the enemy
    if (lastEnemyPos.DistanceSquared(pInterface->Agent_GetInfo().Position) < 100.f)
    {
        steeringIdx = AgentIndexMaps::SteeringSlot.at(SteeringBehaviorType::SeekAndEvade);
        pSteering->SetTargetForIdx(steeringIdx, nextPointInPath);
        auto* blendedSeekAndEvadeSteering = dynamic_cast<BlendedSteering *>(pSteering->GetBehaviorForIdx(steeringIdx));
        blendedSeekAndEvadeSteering->GetWeightedBehaviorsRef()[1].pBehavior->SetTarget(lastEnemyPos);
    }
    else
    {
        steeringIdx = AgentIndexMaps::SteeringSlot.at(SteeringBehaviorType::Seek);
        pSteering->SetTargetForIdx(steeringIdx, nextPointInPath);
    }
    pSteering->SetValidSteeringIdx(steeringIdx);
    if (pInterface->Agent_GetInfo().Stamina >= 5.f) pSteering->SetRunningForIdx(steeringIdx, true);
}

void BT_Helpers::SetSteeringFaceTarget(Elite::Blackboard * const pBlackboard, Elite::Vector2 target)
{
    IExamInterface *pInterface;
    pBlackboard->GetData("interface", pInterface);
    assert(pInterface && "Interface not found in blackboard");

    PrioritySteering *pSteering;
    pBlackboard->GetData("prioritySteering", pSteering);
    assert(pSteering && "Steering not found in blackboard");

    int steeringIdx = AgentIndexMaps::SteeringSlot.at(SteeringBehaviorType::Face);
    pSteering->SetValidSteeringIdx(steeringIdx);
    pSteering->SetTargetForIdx(steeringIdx, target);
}

void BT_Helpers::SetSteeringFleeTarget(Elite::Blackboard * const pBlackboard, Elite::Vector2 target)
{
    IExamInterface *pInterface;
    pBlackboard->GetData("interface", pInterface);
    assert(pInterface && "Interface not found in blackboard");

    PrioritySteering *pSteering;
    pBlackboard->GetData("prioritySteering", pSteering);
    assert(pSteering && "Steering not found in blackboard");

    int steeringIdx = AgentIndexMaps::SteeringSlot.at(SteeringBehaviorType::Flee);
    pSteering->SetValidSteeringIdx(steeringIdx);
    if (pInterface->Agent_GetInfo().Stamina >= 3.f) pSteering->SetRunningForIdx(steeringIdx, true);
    pSteering->SetTargetForIdx(steeringIdx, target);
}

Elite::Vector2 BT_Helpers::FindClosestCornerInHouse(const Elite::Vector2 agentPos, HouseInfo houseInfo)
{
    const Elite::Vector2 houseCenter = houseInfo.Center;
    const Elite::Vector2 houseSize = houseInfo.Size;
    const float offset = -4.f; // Offset to avoid being too close to the house walls
    Elite::Vector2 sign;
    // seek the closest house corner
    sign.x = (houseCenter.x > agentPos.x) ? -1 : 1;
    sign.y = (houseCenter.y > agentPos.y) ? -1 : 1;
    return {
        houseCenter.x + (houseSize.x * 0.5f + offset) * sign.x,
        houseCenter.y + (houseSize.y * 0.5f + offset) * sign.y
    };
}

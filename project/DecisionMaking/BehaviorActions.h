#pragma once
#include "BehaviorTree.h"

class BT_Actions final
{
public:
    // Purge zone actions
    static Elite::BehaviorState FleePurgeZone(Elite::Blackboard *const pBlackboard);

    // Enemy actions
    static Elite::BehaviorState SetIsBeingChased(Elite::Blackboard *const pBlackboard);
    static Elite::BehaviorState EvadeInHouseInFOV(Elite::Blackboard *const pBlackboard);
    static Elite::BehaviorState EvadeToClosestRememberedHouse(Elite::Blackboard *const pBlackboard);
    static Elite::BehaviorState FleeEnemy(Elite::Blackboard *const pBlackboard);
    static Elite::BehaviorState SetEnemyBehindPos(Elite::Blackboard *const pBlackboard);
    static Elite::BehaviorState SetRunModeTrue(Elite::Blackboard *pBlackboard);

    // Attack actions
    static Elite::BehaviorState FaceAndFleeEnemy(Elite::Blackboard *const pBlackboard);
    static Elite::BehaviorState Shoot(Elite::Blackboard *const pBlackboard);
    static Elite::BehaviorState ShotLastFrameActions(Elite::Blackboard *const pBlackboard);
    static Elite::BehaviorState DiscardWeapon(Elite::Blackboard *const pBlackboard);

    // Item actions
    static Elite::BehaviorState SeekFirstItemInSeekList(Elite::Blackboard *const pBlackboard);
    static Elite::BehaviorState SeekTargetItem(Elite::Blackboard *const pBlackboard);
    static Elite::BehaviorState SeekGarbageInGrabRange(Elite::Blackboard *const pBlackboard);
    static Elite::BehaviorState GrabItem(Elite::Blackboard *const pBlackboard);
    static Elite::BehaviorState RemoveGarbage(Elite::Blackboard *const pBlackboard);
    static Elite::BehaviorState CheckItemNeeds(Elite::Blackboard *const pBlackboard);
    static Elite::BehaviorState SetPossibleItemTarget(Elite::Blackboard *const pBlackboard);
    static Elite::BehaviorState UseItemIfNeeded(Elite::Blackboard *const pBlackboard);
    static Elite::BehaviorState CheckoutItems(Elite::Blackboard *const pBlackboard);

    // House actions
    static Elite::BehaviorState CheckoutHouse(Elite::Blackboard *const pBlackboard);
    static Elite::BehaviorState GoToNextTarget(Elite::Blackboard *const pBlackboard);
    static Elite::BehaviorState GoToClosestHouse(Elite::Blackboard *const pBlackboard);
    static Elite::BehaviorState SetNextTarget(Elite::Blackboard *const pBlackboard);

    // Steering actions
    static Elite::BehaviorState GoIntoRadarMode(Elite::Blackboard *const pBlackboard);
    static Elite::BehaviorState Wander(Elite::Blackboard *const pBlackboard);

    // Debug actions
    static Elite::BehaviorState SetDebugSteering(Elite::Blackboard *const pBlackboard);


};

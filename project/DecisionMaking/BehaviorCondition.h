#pragma once

namespace Elite
{
    class Blackboard;
}

class BT_Conditions final
{
public:
    // Purge zone conditions
    static bool IsInPurgeZone(Elite::Blackboard *const pBlackboard);

    // Enemy conditions
    static bool CanGoForKill(Elite::Blackboard *const pBlackboard);
    static bool IsFacingEnemy(Elite::Blackboard *const pBlackboard);
    static bool RanOutOfBullets(Elite::Blackboard *const pBlackboard);
    static bool IsEnemyInHouse(Elite::Blackboard *const pBlackboard);

    // House conditions
    static bool HasHouseInFOV(Elite::Blackboard *const pBlackboard);
    static bool HasUncheckedHouseInFOV(Elite::Blackboard *const pBlackboard);
    static bool RemembersAnyHouse(Elite::Blackboard *const pBlackboard);
    static bool IsInHouse(Elite::Blackboard *const pBlackboard);

    // Item conditions
    static bool HasItemInFOV(Elite::Blackboard *const pBlackboard);
    static bool HasGarbageInFOVAndOneEmptySlot(Elite::Blackboard *const pBlackboard);
    static bool IsItemInGrabRange(Elite::Blackboard *const pBlackboard);
    static bool IsGarbageInGrabRange(Elite::Blackboard *const pBlackboard);
    static bool IsTargetItemSet(Elite::Blackboard *const pBlackboard);
    static bool IsSeekListNotEmpty(Elite::Blackboard *const pBlackboard);

};

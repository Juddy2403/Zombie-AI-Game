#include "../stdafx.h"
#include "BehaviorCondition.h"
#include "BehaviorHelper.h"
#include <cassert>
#include "BehaviorTree.h"
#include "Blackboard.h"
#include "IExamInterface.h"
#include "../IndexMaps.h"
#include "../MapSearchSystem.h"
#include "../Steering/SteeringBehaviors.h"

#pragma region Purge
bool BT_Conditions::IsInPurgeZone(Elite::Blackboard * const pBlackboard)
{
    if (DEBUG_MODE) std::cout << "IsInPurgeZone\n";
    IExamInterface *pInterface;

    pBlackboard->GetData("interface", pInterface);
    assert(pInterface && "Interface not found in blackboard");

    return pInterface->FOV_GetStats().NumPurgeZones > 0;
}
#pragma endregion

#pragma region Enemy
bool BT_Conditions::CanGoForKill(Elite::Blackboard * const pBlackboard)
{
    if (DEBUG_MODE) std::cout << "CanGoForKill\n";

    IExamInterface *pInterface;

    pBlackboard->GetData("interface", pInterface);
    assert(pInterface && "Interface not found in blackboard");

    const int pistolSlot = AgentIndexMaps::InventorySlot.at(eItemType::PISTOL);
    const int rifleSlot = AgentIndexMaps::InventorySlot.at(eItemType::SHOTGUN);
    ItemInfo pistol;
    const bool hasPistol = pInterface->Inventory_GetItem(pistolSlot, pistol);
    ItemInfo rifle;
    const bool hasRifle = pInterface->Inventory_GetItem(rifleSlot, rifle);

    if (hasPistol || hasRifle) return true;
    //more logic should be added here to check for the enemy type and ammo
    return false;
}

bool BT_Conditions::IsFacingEnemy(Elite::Blackboard * const pBlackboard)
{
    if (DEBUG_MODE) std::cout << "IsFacingEnemy\n";

    IExamInterface *pInterface;

    pBlackboard->GetData("interface", pInterface);
    assert(pInterface && "Interface not found in blackboard");

    if (pInterface->FOV_GetStats().NumEnemies == 0) return false;

    float agentOrientation = pInterface->Agent_GetInfo().Orientation;
    Elite::Vector2 agentPos = pInterface->Agent_GetInfo().Position;
    Elite::Vector2 enemyPos = pInterface->GetEnemiesInFOV()[0].Location;
    float toEnemyOrientation = Elite::VectorToOrientation(enemyPos - agentPos);
    if (abs(toEnemyOrientation - agentOrientation) <= Elite::ToRadians(5)) return true;
    return false;
}

bool BT_Conditions::RanOutOfBullets(Elite::Blackboard * const pBlackboard)
{
    if (DEBUG_MODE) std::cout << "RanOutOfBullets\n";

    IExamInterface *pInterface;

    pBlackboard->GetData("interface", pInterface);
    assert(pInterface && "Interface not found in blackboard");

    const int pistolSlot = AgentIndexMaps::InventorySlot.at(eItemType::PISTOL);
    const int rifleSlot = AgentIndexMaps::InventorySlot.at(eItemType::SHOTGUN);
    ItemInfo pistol;
    const bool hasPistol = pInterface->Inventory_GetItem(pistolSlot, pistol);
    ItemInfo rifle;
    const bool hasRifle = pInterface->Inventory_GetItem(rifleSlot, rifle);

    if (hasPistol && pistol.Value <= 0) return true;
    if (hasRifle && rifle.Value <= 0) return true;
    return false;
}

bool BT_Conditions::IsEnemyInHouse(Elite::Blackboard * const pBlackboard)
{
    if (DEBUG_MODE) std::cout << "IsInHouse\n";

    IExamInterface *pInterface;
    pBlackboard->GetData("interface", pInterface);
    assert(pInterface && "Interface not found in blackboard");

    if (pInterface->FOV_GetStats().NumHouses == 0) return false;

    Elite::Vector2 lastEnemyPos(0, 0);
    pBlackboard->GetData("lastEnemyPos", lastEnemyPos);

    const auto house = pInterface->GetHousesInFOV()[0];
    bool isInHouse = MapSearchSystem::IsPointInHouse(lastEnemyPos, house);
    if (isInHouse) return true;
    MapSearchSystem *pMapSearch;
    pBlackboard->GetData("mapSearch", pMapSearch);
    assert(pMapSearch && "MapSearch not found in blackboard");
    HouseInfo closestHouse;
    pMapSearch->GetClosestHouse(pInterface->Agent_GetInfo().Position, closestHouse);
    isInHouse = MapSearchSystem::IsPointInHouse(lastEnemyPos, closestHouse);
    return isInHouse;
}

#pragma endregion

#pragma region House

bool BT_Conditions::HasHouseInFOV(Elite::Blackboard * const pBlackboard)
{
    if (DEBUG_MODE) std::cout << "HasHouseInFOV\n";

    IExamInterface *pInterface;

    pBlackboard->GetData("interface", pInterface);
    assert(pInterface && "Interface not found in blackboard");

    return pInterface->FOV_GetStats().NumHouses > 0;
}

bool BT_Conditions::HasUncheckedHouseInFOV(Elite::Blackboard * const pBlackboard)
{
    if (DEBUG_MODE) std::cout << "HasUncheckedHouseInFOV\n";

    IExamInterface *pInterface;
    pBlackboard->GetData("interface", pInterface);
    assert(pInterface && "Interface not found in blackboard");

    if (pInterface->FOV_GetStats().NumHouses == 0) return false;

    MapSearchSystem *pMapSearch;
    pBlackboard->GetData("mapSearch", pMapSearch);
    assert(pMapSearch && "MapSearch not found in blackboard");

    return !pMapSearch->HasCheckedHouse(pInterface->GetHousesInFOV()[0]);
}

bool BT_Conditions::RemembersAnyHouse(Elite::Blackboard * const pBlackboard)
{
    if (DEBUG_MODE) std::cout << "RemembersAnyHouse\n";

    MapSearchSystem *pMapSearch;
    pBlackboard->GetData("mapSearch", pMapSearch);
    assert(pMapSearch && "MapSearch not found in blackboard");

    return pMapSearch->RemembersAnyHouses();
}

bool BT_Conditions::IsInHouse(Elite::Blackboard * const pBlackboard)
{
    if (DEBUG_MODE) std::cout << "IsInHouse\n";

    IExamInterface *pInterface;
    pBlackboard->GetData("interface", pInterface);
    assert(pInterface && "Interface not found in blackboard");

    if (pInterface->FOV_GetStats().NumHouses == 0) return false;

    const Elite::Vector2 agentPos = pInterface->Agent_GetInfo().Position;
    const auto house = pInterface->GetHousesInFOV()[0];
    // Check if the agent is inside the house bounds
    bool isInHouse = MapSearchSystem::IsPointInHouse(agentPos, house);
    return isInHouse;
}

#pragma endregion

#pragma region Item

bool BT_Conditions::HasItemInFOV(Elite::Blackboard * const pBlackboard)
{
    if (DEBUG_MODE) std::cout << "HasItemInFOV\n";

    IExamInterface *pInterface;

    pBlackboard->GetData("interface", pInterface);
    assert(pInterface && "Interface not found in blackboard");

    return pInterface->FOV_GetStats().NumItems > 0;
}

bool BT_Conditions::HasGarbageInFOVAndOneEmptySlot(Elite::Blackboard * const pBlackboard)
{
    if (DEBUG_MODE) std::cout << "HasGarbageInFOV\n";

    IExamInterface *pInterface;

    pBlackboard->GetData("interface", pInterface);
    assert(pInterface && "Interface not found in blackboard");

    bool hasGarbage = false;
    for(const ItemInfo &item: pInterface->GetItemsInFOV())
    {
        if (item.Type == eItemType::GARBAGE)
        {
            hasGarbage = true;
            break;
        }
    }
    if (!hasGarbage) return false;
    bool hasSpace = false;
    for (int i{}; i<= 4; ++i)
    {
        ItemInfo dummyItem;
        if (!pInterface->Inventory_GetItem(i, dummyItem))
        {
            hasSpace = true;
            break;
        }
    }
    return hasSpace;
}

bool BT_Conditions::IsItemInGrabRange(Elite::Blackboard * const pBlackboard)
{
    if (DEBUG_MODE) std::cout << "IsItemInGrabRange\n";

    IExamInterface *pInterface;
    pBlackboard->GetData("interface", pInterface);
    assert(pInterface && "Interface not found in blackboard");

    std::vector<ItemInfo> seekList;
    pBlackboard->GetData("itemSeekList", seekList);

    if (seekList.empty()) return false;

    const ItemInfo itemInFov = seekList[0];

    const float distToItemSqr = (pInterface->Agent_GetInfo().Position - itemInFov.Location).MagnitudeSquared();
    return distToItemSqr < pInterface->Agent_GetInfo().GrabRange * pInterface->Agent_GetInfo().GrabRange;
}

bool BT_Conditions::IsGarbageInGrabRange(Elite::Blackboard * const pBlackboard)
{
    if (DEBUG_MODE) std::cout << "IsGarbageInGrabRange\n";

    IExamInterface *pInterface;
    pBlackboard->GetData("interface", pInterface);
    assert(pInterface && "Interface not found in blackboard");

    for (const auto& item: pInterface->GetItemsInFOV())
    {
        if (item.Type == eItemType::GARBAGE)
        {
            const float distToItemSqr = (pInterface->Agent_GetInfo().Position - item.Location).MagnitudeSquared();
            return distToItemSqr < pInterface->Agent_GetInfo().GrabRange * pInterface->Agent_GetInfo().GrabRange;
        }
    }
    return false;
}

bool BT_Conditions::IsTargetItemSet(Elite::Blackboard * const pBlackboard)
{
    if (DEBUG_MODE) std::cout << "IsTargetItemSet\n";

    std::optional<eItemType> targetItemType;
    pBlackboard->GetData("targetItemType", targetItemType);

    return targetItemType.has_value();
}

bool BT_Conditions::IsSeekListNotEmpty(Elite::Blackboard * const pBlackboard)
{
    std::vector<ItemInfo> seekList;
    pBlackboard->GetData("itemSeekList", seekList);
    if (DEBUG_MODE) std::cout << "IsSeekListNotEmpty\n";
    return !seekList.empty();
}

#pragma endregion

#include "../stdafx.h"
#include "BehaviorActions.h"
#include "BehaviorHelper.h"
#include <cassert>
#include "BehaviorTree.h"
#include "Blackboard.h"
#include "IExamInterface.h"
#include "../IndexMaps.h"
#include "../MapSearchSystem.h"
#include "../Steering/CombinedSteeringBehaviors.h"
#include "../Steering/SteeringBehaviors.h"

#pragma region PurgeZone

Elite::BehaviorState BT_Actions::FleePurgeZone(Elite::Blackboard * const pBlackboard)
{
    if (DEBUG_MODE) std::cout << "FleePurgeZone\n";
    IExamInterface *pInterface;

    pBlackboard->GetData("interface", pInterface);
    assert(pInterface && "Interface not found in blackboard");

    int numPurgeZones = pInterface->FOV_GetStats().NumPurgeZones;
    auto purgeZones = pInterface->GetPurgeZonesInFOV();
    Elite::Vector2 fleeDir;
    switch (numPurgeZones)
    {
        case 2:
        {
            // Two zones: flee perpendicular to the line between centers
            const Elite::Vector2 diff = purgeZones[1].Center - purgeZones[0].Center;
            Elite::Vector2 perp = Elite::Vector2::Perpendicular(diff).GetNormalized();

            // Choose the perpendicular direction that points away from zones
            const Elite::Vector2 toAgent = pInterface->Agent_GetInfo().Position - (
                                               purgeZones[0].Center + purgeZones[1].Center) * 0.5f;
            if (perp.Dot(toAgent) < 0)
                perp = -perp;
            fleeDir = perp;
            break;
        }
        case 3:
        {
            // Three zones: flee away from average center, but perpendicular to at least one zone-to-zone vector
            const Elite::Vector2 avgCenter =
                    (purgeZones[0].Center + purgeZones[1].Center + purgeZones[2].Center) / 3.0f;

            // Use perpendicular to line between first two zones
            const Elite::Vector2 diff = purgeZones[1].Center - purgeZones[0].Center;
            Elite::Vector2 perp = Elite::Vector2::Perpendicular(diff).GetNormalized();

            // Ensure it’s facing away from the center
            const Elite::Vector2 toAgent = (pInterface->Agent_GetInfo().Position - avgCenter).GetNormalized();
            if (perp.Dot(toAgent) < 0)
                perp = -perp;
            fleeDir = perp;
            break;
        }
        // More than 3 zones or just one: flee from the first zone
        default:
        {
            fleeDir = pInterface->Agent_GetInfo().Position - purgeZones[0].Center;
            fleeDir.Normalize();
            break;
        }
    }
    const Elite::Vector2 target = fleeDir * purgeZones[0].Radius * purgeZones[0].Radius;
    BT_Helpers::SetSteeringSeekTarget(pBlackboard, target, true);

    return Elite::BehaviorState::Success;
}

#pragma endregion

#pragma region Enemy

Elite::BehaviorState BT_Actions::SetIsBeingChased(Elite::Blackboard * const pBlackboard)
{
    if (DEBUG_MODE) std::cout << "SetIsBeingChased\n";
    pBlackboard->ChangeData("isBeingChased", true);
    return Elite::BehaviorState::Success;
}

Elite::BehaviorState BT_Actions::EvadeInHouseInFOV(Elite::Blackboard * const pBlackboard)
{
    if (DEBUG_MODE) std::cout << "EvadeInHouseInFOV\n";
    IExamInterface *pInterface;

    pBlackboard->GetData("interface", pInterface);
    assert(pInterface && "Interface not found in blackboard");

    const Elite::Vector2 agentPos = pInterface->Agent_GetInfo().Position;

    const Elite::Vector2 target = BT_Helpers::FindClosestCornerInHouse(agentPos, pInterface->GetHousesInFOV()[0]);

    BT_Helpers::SetSteeringEvade(pBlackboard, target);
    return Elite::BehaviorState::Success;
}

Elite::BehaviorState BT_Actions::EvadeToClosestRememberedHouse(Elite::Blackboard * const pBlackboard)
{
    if (DEBUG_MODE) std::cout << "EvadeToClosestRememberedHouse\n";
    MapSearchSystem *pMapSearch;
    IExamInterface *pInterface;

    pBlackboard->GetData("interface", pInterface);
    assert(pInterface && "Interface not found in blackboard");
    pBlackboard->GetData("mapSearch", pMapSearch);
    assert(pMapSearch && "MapSearch not found in blackboard");

    const Elite::Vector2 agentPos = pInterface->Agent_GetInfo().Position;
    HouseInfo houseInfo;
    pMapSearch->GetClosestHouse(agentPos, houseInfo);
    const Elite::Vector2 target = BT_Helpers::FindClosestCornerInHouse(agentPos, houseInfo);

    BT_Helpers::SetSteeringEvade(pBlackboard, target);
    return Elite::BehaviorState::Success;
}

Elite::BehaviorState BT_Actions::FleeEnemy(Elite::Blackboard * const pBlackboard)
{
    if (DEBUG_MODE) std::cout << "FleeEnemy\n";

    if (DEBUG_MODE) std::cout << "EvadeToTarget\n";
    IExamInterface *pInterface;
    pBlackboard->GetData("interface", pInterface);
    assert(pInterface && "Interface not found in blackboard");

    // find a random point around the agent in a radius of 30 units
    const Elite::Vector2 agentPos = pInterface->Agent_GetInfo().Position;
    const float fleeRadius = 200.f;
    const Elite::Vector2 fleeDir = Elite::OrientationToVector(Elite::ToRadians(rand() % 360));
    const Elite::Vector2 target = agentPos + fleeDir * fleeRadius;

    BT_Helpers::SetSteeringEvade(pBlackboard, target);
    return Elite::BehaviorState::Success;
}

Elite::BehaviorState BT_Actions::SetEnemyBehindPos(Elite::Blackboard * const pBlackboard)
{
    if (DEBUG_MODE) std::cout << "SetEnemyBehindPos\n";

    Elite::Vector2 lastEnemyPos(0, 0);
    IExamInterface *pInterface;
    pBlackboard->GetData("interface", pInterface);
    assert(pInterface && "Interface not found in blackboard");

    // Set the last enemy position to be behind the agent
    const Elite::Vector2 agentPos = pInterface->Agent_GetInfo().Position;
    const Elite::Vector2 agentForward = Elite::OrientationToVector( pInterface->Agent_GetInfo().Orientation);
    const float agentSize = pInterface->Agent_GetInfo().AgentSize;
    lastEnemyPos = agentPos - agentForward * agentSize * 2.f;

    pBlackboard->ChangeData("lastEnemyPos", lastEnemyPos);
    return Elite::BehaviorState::Success;
}

Elite::BehaviorState BT_Actions::SetRunModeTrue(Elite::Blackboard * const pBlackboard)
{
    if (DEBUG_MODE) std::cout << "SetRunModeTrue\n";

    PrioritySteering *pSteering;
    pBlackboard->GetData("prioritySteering", pSteering);
    assert(pSteering && "Steering not found in blackboard");
    pSteering->SetRunningForIdx(pSteering->GetValidIdx(), true);
    return Elite::BehaviorState::Success;
}


#pragma endregion

#pragma region Attack

Elite::BehaviorState BT_Actions::FaceAndFleeEnemy(Elite::Blackboard * const pBlackboard)
{
    if (DEBUG_MODE) std::cout << "FaceAndFleeEnemy\n";
    Elite::Vector2 lastEnemyPos(0, 0);
    PrioritySteering *pSteering;

    pBlackboard->GetData("lastEnemyPos", lastEnemyPos);

    IExamInterface *pInterface;
    pBlackboard->GetData("interface", pInterface);
    assert(pInterface && "Interface not found in blackboard");
    const float distSqrToEnemy = pInterface->Agent_GetInfo().Position.DistanceSquared(lastEnemyPos);

    pBlackboard->GetData("prioritySteering", pSteering);
    assert(pSteering && "Steering not found in blackboard");

    int steeringIdx = AgentIndexMaps::SteeringSlot.at(SteeringBehaviorType::FleeWhileFacing);
    pSteering->SetValidSteeringIdx(steeringIdx);
    pSteering->SetTargetForIdx(steeringIdx, lastEnemyPos);
    if (distSqrToEnemy <= 9.f) pSteering->SetRunningForIdx(steeringIdx, true);
    else if (distSqrToEnemy >= 36.f) pSteering->SetRunningForIdx(steeringIdx, false);

    return Elite::BehaviorState::Success;
}

Elite::BehaviorState BT_Actions::Shoot(Elite::Blackboard * const pBlackboard)
{
    if (DEBUG_MODE) std::cout << "Shoot\n";
    IExamInterface *pInterface;

    pBlackboard->GetData("interface", pInterface);
    assert(pInterface && "Interface not found in blackboard");

    int pistolSlot = AgentIndexMaps::InventorySlot.at(eItemType::PISTOL);
    int rifleSlot = AgentIndexMaps::InventorySlot.at(eItemType::SHOTGUN);
    ItemInfo pistol;
    bool hasPistol = pInterface->Inventory_GetItem(pistolSlot, pistol);
    ItemInfo rifle;
    bool hasRifle = pInterface->Inventory_GetItem(rifleSlot, rifle);

    if (hasRifle && (pInterface->FOV_GetStats().NumEnemies > 1 || !hasPistol))
    {
        pInterface->Inventory_UseItem(rifleSlot);
        pBlackboard->ChangeData("shotLastFrame", true);

        return Elite::BehaviorState::Success;
    }
    if (hasPistol)
    {
        pInterface->Inventory_UseItem(pistolSlot);
        pBlackboard->ChangeData("shotLastFrame", true);
        return Elite::BehaviorState::Success;
    }

    return Elite::BehaviorState::Failure;
}

Elite::BehaviorState BT_Actions::ShotLastFrameActions(Elite::Blackboard * const pBlackboard)
{
    if (DEBUG_MODE) std::cout << "CheckIfShotLastFrame\n";
    bool shotLastFrame;

    pBlackboard->GetData("shotLastFrame", shotLastFrame);

    if (!shotLastFrame) return Elite::BehaviorState::Success;

    IExamInterface *pInterface;

    pBlackboard->GetData("interface", pInterface);
    assert(pInterface && "Interface not found in blackboard");

    // If no enemy in sight, then the enemy was killed after being shot
    if (pInterface->FOV_GetStats().NumEnemies == 0)
    {
        pBlackboard->ChangeData("isBeingChased", false);
    }
    pBlackboard->ChangeData("shotLastFrame", false);
    return Elite::BehaviorState::Failure;
}

Elite::BehaviorState BT_Actions::DiscardWeapon(Elite::Blackboard * const pBlackboard)
{
    if (DEBUG_MODE) std::cout << "DiscardWeapon\n";
    IExamInterface *pInterface;

    pBlackboard->GetData("interface", pInterface);
    assert(pInterface && "Interface not found in blackboard");

    int pistolSlot = AgentIndexMaps::InventorySlot.at(eItemType::PISTOL);
    int rifleSlot = AgentIndexMaps::InventorySlot.at(eItemType::SHOTGUN);
    ItemInfo pistol;
    bool hasPistol = pInterface->Inventory_GetItem(pistolSlot, pistol);
    ItemInfo rifle;
    bool hasRifle = pInterface->Inventory_GetItem(rifleSlot, rifle);

    if (hasPistol)
    {
        if (pistol.Value <= 0) pInterface->Inventory_RemoveItem(pistolSlot);
    }
    if (hasRifle)
    {
        if (rifle.Value <= 0) pInterface->Inventory_RemoveItem(rifleSlot);
    }
    return Elite::BehaviorState::Success;
}

#pragma endregion

#pragma region Items

Elite::BehaviorState BT_Actions::SeekFirstItemInSeekList(Elite::Blackboard * const pBlackboard)
{
    if (DEBUG_MODE) std::cout << "SeekFirstItemInSeekList\n";
    IExamInterface *pInterface;

    pBlackboard->GetData("interface", pInterface);
    assert(pInterface && "Interface not found in blackboard");
    std::vector<ItemInfo> seekList;
    pBlackboard->GetData("itemSeekList", seekList);

    if (seekList.empty()) return Elite::BehaviorState::Failure;

    const Elite::Vector2 itemPos = seekList[0].Location;
    const Elite::Vector2 itemToAgent = (pInterface->Agent_GetInfo().Position - itemPos);
    const Elite::Vector2 itemToAgentDir = itemToAgent.GetNormalized();
    const Elite::Vector2 target = itemPos + itemToAgentDir * pInterface->Agent_GetInfo().GrabRange * 0.2f;
    // If the item is within grab range, face it instead of seeking
    if (itemToAgent.MagnitudeSquared() <= pInterface->Agent_GetInfo().GrabRange * pInterface->Agent_GetInfo().
        GrabRange * 0.3f)
    {
        BT_Helpers::SetSteeringFaceTarget(pBlackboard, itemPos);
        return Elite::BehaviorState::Success;
    }

    BT_Helpers::SetSteeringSeekTarget(pBlackboard, target);
    return Elite::BehaviorState::Success;
}

Elite::BehaviorState BT_Actions::SeekTargetItem(Elite::Blackboard * const pBlackboard)
{
    if (DEBUG_MODE) std::cout << "SeekTargetItem\n";
    IExamInterface *pInterface;

    pBlackboard->GetData("interface", pInterface);
    assert(pInterface && "Interface not found in blackboard");

    MapSearchSystem *pMapSearch;
    pBlackboard->GetData("mapSearch", pMapSearch);
    assert(pMapSearch && "MapSearch not found in blackboard");
    std::optional < eItemType > targetItemType;
    pBlackboard->GetData("targetItemType", targetItemType);

    Elite::Vector2 itemPos;
    if (!pMapSearch->GetItemClosestLocation(pInterface->Agent_GetInfo().Position, targetItemType.value(), itemPos))
    {
        return Elite::BehaviorState::Failure; // No item found
    }
    const Elite::Vector2 itemToAgent = (pInterface->Agent_GetInfo().Position - itemPos);
    if (itemToAgent.MagnitudeSquared() <= pInterface->Agent_GetInfo().GrabRange * pInterface->Agent_GetInfo().
        GrabRange)
    {
        BT_Helpers::SetSteeringFaceTarget(pBlackboard, itemPos);
        return Elite::BehaviorState::Success;
    }
    const Elite::Vector2 target = itemPos + itemToAgent.GetNormalized() * pInterface->Agent_GetInfo().GrabRange *
                                  0.3f;
    BT_Helpers::SetSteeringSeekTarget(pBlackboard, target);
    return Elite::BehaviorState::Success;
}

Elite::BehaviorState BT_Actions::SeekGarbageInGrabRange(Elite::Blackboard * const pBlackboard)
{
    if (DEBUG_MODE) std::cout << "SeekTargetItem\n";
    IExamInterface *pInterface;

    pBlackboard->GetData("interface", pInterface);
    assert(pInterface && "Interface not found in blackboard");

    for (const auto &item: pInterface->GetItemsInFOV())
    {
        if (item.Type == eItemType::GARBAGE)
        {
            // If the item is within grab range, face it instead of seeking
            const Elite::Vector2 itemPos = item.Location;
            const Elite::Vector2 itemToAgent = (pInterface->Agent_GetInfo().Position - itemPos);
            if (itemToAgent.MagnitudeSquared() <= pInterface->Agent_GetInfo().GrabRange * pInterface->
                Agent_GetInfo().
                GrabRange)
            {
                BT_Helpers::SetSteeringFaceTarget(pBlackboard, itemPos);
                return Elite::BehaviorState::Success;
            }
            const Elite::Vector2 target = itemPos + itemToAgent.GetNormalized() * pInterface->Agent_GetInfo().
                                          GrabRange *
                                          0.3f;
            BT_Helpers::SetSteeringSeekTarget(pBlackboard, target);
            return Elite::BehaviorState::Success;
        }
    }
    return Elite::BehaviorState::Failure;
}

Elite::BehaviorState BT_Actions::GrabItem(Elite::Blackboard * const pBlackboard)
{
    if (DEBUG_MODE) std::cout << "GrabItem\n";
    IExamInterface *pInterface;

    pBlackboard->GetData("interface", pInterface);
    assert(pInterface && "Interface not found in blackboard");

    ItemInfo item{};
    if (pInterface->GrabNearestItem(item))
    {
        int itemSlot = AgentIndexMaps::InventorySlot.at(item.Type);
        if (item.Type == eItemType::FOOD)
        {
            ItemInfo dummyItem;
            bool hasSlot1 = !pInterface->Inventory_GetItem(itemSlot, dummyItem);
            if (!hasSlot1) ++itemSlot; // Food takes 2 slots, so we check the next slot
        }
        if (pInterface->Inventory_AddItem(itemSlot, item))
        {
            MapSearchSystem *pMapSearch;
            pBlackboard->GetData("mapSearch", pMapSearch);
            assert(pMapSearch && "MapSearch not found in blackboard");
            std::optional < eItemType > targetItemType;
            pBlackboard->GetData("targetItemType", targetItemType);
            // if the target item has been grabbed
            if (targetItemType.has_value() && targetItemType.value() == item.Type)
            {
                targetItemType.reset();
                pBlackboard->ChangeData("targetItemType", targetItemType);
            }
            pMapSearch->PickedUpItem(item);
            std::vector<ItemInfo> seekList;
            pBlackboard->GetData("itemSeekList", seekList);
            // Remove the item from the seek list if it was there
            auto newEnd = std::ranges::remove_if(seekList,
                                                 [&item](const ItemInfo &i)
                                                 {
                                                     return i.Type == item.Type && i.Location == item.Location;
                                                 }).begin();
            seekList.erase(newEnd, seekList.end());
            pBlackboard->ChangeData("itemSeekList", seekList);
            return Elite::BehaviorState::Success;
        }
    }
    return Elite::BehaviorState::Failure;
}

Elite::BehaviorState BT_Actions::RemoveGarbage(Elite::Blackboard * const pBlackboard)
{
    if (DEBUG_MODE) std::cout << "GrabItem\n";
    IExamInterface *pInterface;

    pBlackboard->GetData("interface", pInterface);
    assert(pInterface && "Interface not found in blackboard");

    ItemInfo item{};
    if (pInterface->GrabNearestItem(item))
    {
        if (item.Type == eItemType::GARBAGE)
        {
            int firstEmptySlot = -1;
            for (int i{}; i <= 4; ++i)
            {
                ItemInfo dummyItem;
                if (!pInterface->Inventory_GetItem(i, dummyItem))
                {
                    firstEmptySlot = i;
                    break;
                }
            }
            if (firstEmptySlot == -1)
            {
                // No empty slot found, so we can't remove the garbage
                return Elite::BehaviorState::Failure;
            }
            pInterface->Inventory_AddItem(firstEmptySlot, item);
            pInterface->Inventory_RemoveItem(firstEmptySlot);
            return Elite::BehaviorState::Success;
        }
    }
    return Elite::BehaviorState::Failure;
}

Elite::BehaviorState BT_Actions::CheckItemNeeds(Elite::Blackboard * const pBlackboard)
{
    if (DEBUG_MODE) std::cout << "CheckItemNeeds\n";
    IExamInterface *pInterface;
    pBlackboard->GetData("interface", pInterface);
    assert(pInterface && "Interface not found in blackboard");

    std::map<eItemType, bool> itemNeedList;
    pBlackboard->GetData("itemNeedList", itemNeedList);

    for (eItemType itemType: AgentIndexMaps::PriorityItemList)
    {
        ItemInfo dummyItem;
        int itemSlot = AgentIndexMaps::InventorySlot.at(itemType);
        bool hasItem = pInterface->Inventory_GetItem(itemSlot, dummyItem);
        if (itemType == eItemType::FOOD && !hasItem)
        {
            // Food takes 2 slots, so we check the next slot too
            hasItem = pInterface->Inventory_GetItem(itemSlot + 1, dummyItem);
        }
        if (!hasItem) itemNeedList[itemType] = true;
        else itemNeedList[itemType] = false;
    }
    pBlackboard->ChangeData("itemNeedList", itemNeedList);
    return Elite::BehaviorState::Success;
}

Elite::BehaviorState BT_Actions::SetPossibleItemTarget(Elite::Blackboard * const pBlackboard)
{
    if (DEBUG_MODE) std::cout << "SetPossibleItemTarget\n";
    std::optional < eItemType > targetItemType;
    pBlackboard->GetData("targetItemType", targetItemType);

    std::map<eItemType, bool> itemNeedList;
    pBlackboard->GetData("itemNeedList", itemNeedList);

    MapSearchSystem *pMapSearch;
    pBlackboard->GetData("mapSearch", pMapSearch);
    assert(pMapSearch && "MapSearch not found in blackboard");

    for (eItemType itemType: AgentIndexMaps::PriorityItemList)
    {
        if (itemNeedList[itemType] && pMapSearch->KnowsAnyItemLocation(itemType))
        {
            targetItemType = itemType;
            pBlackboard->ChangeData("targetItemType", targetItemType);
            return Elite::BehaviorState::Success;
        }
    }
    // No item needs, clear target
    targetItemType.reset();
    pBlackboard->ChangeData("targetItemType", targetItemType);
    return Elite::BehaviorState::Success;
}

Elite::BehaviorState BT_Actions::UseItemIfNeeded(Elite::Blackboard * const pBlackboard)
{
    if (DEBUG_MODE) std::cout << "UseItemIfNeeded\n";
    IExamInterface *pInterface;
    pBlackboard->GetData("interface", pInterface);
    assert(pInterface && "Interface not found in blackboard");

    ItemInfo item;
    bool hasMedkit = pInterface->Inventory_GetItem(AgentIndexMaps::InventorySlot.at(eItemType::MEDKIT), item);
    if (hasMedkit && 10.f - pInterface->Agent_GetInfo().Health > item.Value)
    {
        if (DEBUG_MODE) std::cout << "Using Medkit\n";
        pInterface->Inventory_UseItem(AgentIndexMaps::InventorySlot.at(eItemType::MEDKIT));
        pInterface->Inventory_RemoveItem(AgentIndexMaps::InventorySlot.at(eItemType::MEDKIT));
    }

    int foodSlot = AgentIndexMaps::InventorySlot.at(eItemType::FOOD);
    bool hasFood = pInterface->Inventory_GetItem(foodSlot, item);
    if (!hasFood)
    {
        // Check next slot too
        hasFood = pInterface->Inventory_GetItem(++foodSlot, item);
    }
    if (hasFood && 10.f - pInterface->Agent_GetInfo().Energy > item.Value)
    {
        if (DEBUG_MODE) std::cout << "Using Food\n";
        pInterface->Inventory_UseItem(foodSlot);
        pInterface->Inventory_RemoveItem(foodSlot);
    }

    return Elite::BehaviorState::Success;
}

Elite::BehaviorState BT_Actions::CheckoutItems(Elite::Blackboard * const pBlackboard)
{
    if (DEBUG_MODE) std::cout << "AddItemToSeekList\n";

    IExamInterface *pInterface;
    pBlackboard->GetData("interface", pInterface);
    assert(pInterface && "Interface not found in blackboard");

    std::vector<ItemInfo> seekList;
    pBlackboard->GetData("itemSeekList", seekList);

    MapSearchSystem *pMapSearch;
    pBlackboard->GetData("mapSearch", pMapSearch);
    assert(pMapSearch && "MapSearch not found in blackboard");

    std::map<eItemType, int> itemsInSeekList;
    for (const auto &item: seekList)
    {
        if (itemsInSeekList.contains(item.Type)) itemsInSeekList[item.Type]++;
        else itemsInSeekList[item.Type] = 1;
    }

    for (auto &item: pInterface->GetItemsInFOV())
    {
        if (item.Type == eItemType::GARBAGE) continue; // Don't add garbage to seek list
        int itemSlot = AgentIndexMaps::InventorySlot.at(item.Type);
        ItemInfo dummyItem;
        bool hasSpace = !pInterface->Inventory_GetItem(itemSlot, dummyItem);
        if (item.Type == eItemType::FOOD)
        {
            if (!hasSpace) hasSpace = !pInterface->Inventory_GetItem(itemSlot + 1, dummyItem);
            if (!hasSpace || (itemsInSeekList.contains(item.Type) && itemsInSeekList[item.Type] >=2))
            {
                pMapSearch->RememberItemLocation(item);
            }
            else
            {
                seekList.push_back(item);
                if (itemsInSeekList.contains(item.Type))itemsInSeekList[item.Type]++;
                else itemsInSeekList[item.Type] = 1;
            }
        }
        else
        {
            if (!hasSpace || itemsInSeekList.contains(item.Type))
            {
                pMapSearch->RememberItemLocation(item);
            }
            else
            {
                seekList.push_back(item);
                if (itemsInSeekList.contains(item.Type)) itemsInSeekList[item.Type]++;
                else itemsInSeekList[item.Type] = 1;
            }
        }
    }
    pBlackboard->ChangeData("itemSeekList", seekList);
    return Elite::BehaviorState::Success;
}

#pragma endregion

#pragma region House

Elite::BehaviorState BT_Actions::CheckoutHouse(Elite::Blackboard * const pBlackboard)
{
    if (DEBUG_MODE) std::cout << "CheckoutHouse\n";

    IExamInterface *pInterface;
    pBlackboard->GetData("interface", pInterface);
    assert(pInterface && "Interface not found in blackboard");

    MapSearchSystem *pMapSearch;
    pBlackboard->GetData("mapSearch", pMapSearch);
    assert(pMapSearch && "MapSearch not found in blackboard");

    pMapSearch->FoundHouse(pInterface->GetHousesInFOV()[0]);
    return Elite::BehaviorState::Success;
}

Elite::BehaviorState BT_Actions::GoToNextTarget(Elite::Blackboard * const pBlackboard)
{
    if (DEBUG_MODE) std::cout << "GoToNextTarget\n";

    std::optional<Elite::Vector2> target;
    pBlackboard->GetData("currentTarget", target);

    BT_Helpers::SetSteeringSeekTarget(pBlackboard, target.value(), true, true);

    return Elite::BehaviorState::Success;
}

Elite::BehaviorState BT_Actions::GoToClosestHouse(Elite::Blackboard * const pBlackboard)
{
    if (DEBUG_MODE) std::cout << "GoToClosestHouse\n";

    IExamInterface *pInterface;
    pBlackboard->GetData("interface", pInterface);
    assert(pInterface && "Interface not found in blackboard");

    MapSearchSystem *pMapSearch;
    pBlackboard->GetData("mapSearch", pMapSearch);
    assert(pMapSearch && "MapSearch not found in blackboard");
    HouseInfo housePos;
    if (!pMapSearch->GetClosestHouse(pInterface->Agent_GetInfo().Position, housePos))
        return Elite::BehaviorState::Failure;
    Elite::Vector2 target = BT_Helpers::FindClosestCornerInHouse(pInterface->Agent_GetInfo().Position, housePos);
    BT_Helpers::SetSteeringSeekTarget(pBlackboard, target, false, true);
    return Elite::BehaviorState::Success;
}

Elite::BehaviorState BT_Actions::SetNextTarget(Elite::Blackboard * const pBlackboard)
{
    if (DEBUG_MODE) std::cout << "SetNextTarget\n";

    IExamInterface *pInterface;
    pBlackboard->GetData("interface", pInterface);
    assert(pInterface && "Interface not found in blackboard");

    MapSearchSystem *pMapSearch;
    pBlackboard->GetData("mapSearch", pMapSearch);
    assert(pMapSearch && "MapSearch not found in blackboard");

    std::optional<Elite::Vector2> target;
    pBlackboard->GetData("currentTarget", target);

    Elite::Vector2 targetPos;
    if (!pMapSearch->GetCurrentTarget(pInterface->Agent_GetInfo().Position, targetPos))
    {
        target.reset();
        pBlackboard->ChangeData("currentTarget", target);
        return Elite::BehaviorState::Failure; // No target found
    }
    target = targetPos;
    pBlackboard->ChangeData("currentTarget", target);
    return Elite::BehaviorState::Success;
}

#pragma endregion

#pragma region Steering

Elite::BehaviorState BT_Actions::GoIntoRadarMode(Elite::Blackboard * const pBlackboard)
{
    if (DEBUG_MODE) std::cout << "GoIntoRadarMode\n";
    pBlackboard->ChangeData("radarMode", true);
    return Elite::BehaviorState::Success;
}

Elite::BehaviorState BT_Actions::Wander(Elite::Blackboard * const pBlackboard)
{
    PrioritySteering *pSteering;
    pBlackboard->GetData("prioritySteering", pSteering);
    assert(pSteering && "Steering not found in blackboard");

    int steeringIdx = AgentIndexMaps::SteeringSlot.at(SteeringBehaviorType::Wander);
    pSteering->SetValidSteeringIdx(steeringIdx);

    return Elite::BehaviorState::Success;
}

#pragma endregion

#pragma region Debug

Elite::BehaviorState BT_Actions::SetDebugSteering(Elite::Blackboard * const pBlackboard)
{
    if (DEBUG_MODE) std::cout << "SetDebugSteering\n";
    PrioritySteering *pSteering;
    pBlackboard->GetData("prioritySteering", pSteering);
    assert(pSteering && "Steering not found in blackboard");

    int steeringIdx = AgentIndexMaps::SteeringSlot.at(SteeringBehaviorType::SeekAndWander);
    pSteering->SetValidSteeringIdx(steeringIdx);
    return Elite::BehaviorState::Success;
}

#pragma endregion
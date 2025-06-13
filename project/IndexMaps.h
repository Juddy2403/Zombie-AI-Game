#pragma once
#include <map>
#include "Exam_HelperStructs.h"

enum class eItemType;

enum class SteeringBehaviorType
{
    SeekAndWander ,
    Seek,
    FleeWhileFacing,
    Face,
    Wander,
    SeekAndEvade,
    Flee
};

namespace AgentIndexMaps
{
    // Index maps for the Agent's Steering Behaviors
    const std::map<SteeringBehaviorType, int> SteeringSlot = {
        {SteeringBehaviorType::SeekAndWander, 0},
        {SteeringBehaviorType::Seek, 1},
        {SteeringBehaviorType::FleeWhileFacing, 2},
        {SteeringBehaviorType::Face, 3},
        {SteeringBehaviorType::Wander, 4},
        {SteeringBehaviorType::SeekAndEvade, 5},
        {SteeringBehaviorType::Flee, 6}
    };

    // Index maps for the Agent's inventory slots
    const std::map<eItemType, int> InventorySlot = {
        {eItemType::PISTOL, 0},
        {eItemType::SHOTGUN, 1},
        {eItemType::FOOD, 2},
        {eItemType::MEDKIT, 4},
    };

    const std::vector<eItemType> PriorityItemList
    {
        eItemType::MEDKIT,
        eItemType::FOOD,
        eItemType::PISTOL,
        eItemType::SHOTGUN
    };

}

#pragma once
struct HouseInfo;
namespace Elite { class Blackboard; }

constexpr bool DEBUG_MODE = false; // Prints current BT actions and conditions to console

class BT_Helpers final
{
public:
    static void SetSteeringSeekTarget(Elite::Blackboard *const pBlackboard, Elite::Vector2 target, bool runMode = false, bool wanderMode = false);
    static void SetSteeringEvade(Elite::Blackboard *const pBlackboard, Elite::Vector2 target);
    static void SetSteeringFaceTarget(Elite::Blackboard *const pBlackboard, Elite::Vector2 target);
    static void SetSteeringFleeTarget(Elite::Blackboard *const pBlackboard, Elite::Vector2 target);
    static Elite::Vector2 FindClosestCornerInHouse(const Elite::Vector2 agentPos, HouseInfo houseInfo);
};

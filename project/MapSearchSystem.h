#pragma once
#include <map>
#include <optional>
#include <set>

class IExamInterface;
struct SetHouseInfo;
struct ItemInfo;
struct AgentInfo;
struct HouseInfo;
enum class eItemType;


class MapSearchSystem {
public:
    explicit MapSearchSystem(const AgentInfo& agentInfo);

    void RenderDebug(IExamInterface* const pInterface) const;

    // Returns true if the house has already been checked
    [[nodiscard]] bool HasCheckedHouse(const HouseInfo& house) const;

    void Update(float dt);

    // Adds the house to the list and updates house search targets
    void FoundHouse(const HouseInfo& house);

    [[nodiscard]] bool IsDoneCheckingMap() const;
    // Returns true if the house has been found
    bool GetClosestHouse(const Elite::Vector2& agentPosition, HouseInfo &outTarget) const;
    [[nodiscard]] bool RemembersAnyHouses() const;

    bool GetCurrentTarget(const Elite::Vector2& agentPosition, Elite::Vector2& outTarget);
    bool GetRandomOutOfHouseTarget(Elite::Vector2& outTarget);
    void ReachedTarget(const Elite::Vector2& target);

    void RememberItemLocation(const ItemInfo& itemInfo);
    bool GetItemClosestLocation(const Elite::Vector2 &agentPosition, const eItemType &itemType, Elite::Vector2 &outTarget) const;
    void PickedUpItem(const ItemInfo& itemInfo);

    [[nodiscard]] bool RemembersItem(const ItemInfo& item) const;
    [[nodiscard]] bool KnowsAnyItemLocation(const eItemType& itemType) const;

    static bool IsPointInHouse(const Elite::Vector2& point, const HouseInfo& house, float offset = 0);
    [[nodiscard]] bool IsPointInAnyFoundHouse(const Elite::Vector2 &point) const;

private:
    void AddVillageSearchTargets(const HouseInfo &house);
    void AddHouseSearchTargets(const HouseInfo &house);

    void CleanupObsoleteTargets();

    // Returns the closest location to the agent that has not been explored yet
    // This is for both inner and outer radius search targets
    // Returns true if a target was found
    bool GetCurrentExploreTarget(const Elite::Vector2& agentPosition, Elite::Vector2& outTarget) const;
    // Returns the closest location to the agent that has not been explored yet
    // Returns true if a target was found
    bool GetCurrentHouseExploreTarget(const Elite::Vector2& agentPosition, Elite::Vector2& outTarget);
    // Returns the closest location to the agent that has not been explored yet
    // Returns true if a target was found
    bool GetCurrentVillageExploreTarget(const Elite::Vector2& agentPosition, Elite::Vector2& outTarget);

    static Elite::Vector2 GetClosestPosFromVec(const Elite::Vector2 &agentPosition,const std::set<Elite::Vector2> &vec);

    std::set<Elite::Vector2> m_InnerRadiusSearchTargets{};
    std::set<Elite::Vector2> m_OuterRadiusSearchTargets{};
    std::set<Elite::Vector2> m_VillageSearchTargets{};
    std::set<Elite::Vector2> m_HouseSearchTargets{};

    // For debugging purposes
    std::vector<HouseInfo> m_VillagesInfo{};

    std::map<eItemType, std::set<Elite::Vector2>> m_FoundItemLocationMap{};
    std::set<SetHouseInfo> m_FoundHouses{};
    // how many houses have been found in the current village
    int m_CurrVillageHousesCount = 0;

    std::optional<Elite::Vector2> m_CurrentTarget;
    const float m_TargetSearchInterval = 3.f;
    float m_CurrTargetSearchTime = 0.f;
    const float m_TargetRefreshInterval = 5.f;
    float m_CurrTargetRefreshTime = 0.f;

    const float m_InnerSearchRadius = 30.f;
    const float m_InnerAngleStep = 90.f;
    const float m_OuterSearchRadius = 200.f;
    const float m_OuterAngleStep = 45.f;
    // The hypotenuse of the agent's sight (since the fov angle is 90)
    float m_AgentSightHyp;
};

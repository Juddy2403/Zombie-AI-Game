#include "stdafx.h"
#include "MapSearchSystem.h"
#include "Exam_HelperStructs.h"
#include "HouseInfoSet.h"
#include "IExamInterface.h"

MapSearchSystem::MapSearchSystem(const AgentInfo &agentInfo)
{
    m_CurrentTarget = std::nullopt;
    m_AgentSightHyp = 2.f * agentInfo.FOV_Range * tanf(agentInfo.FOV_Angle / 2.f);
    // add inner radius search targets: in a radius of m_InnerSearchRadius with a center in (0,0) every m_InnerAngleStep degrees
    for (float angle = 0.f; angle < 360.f; angle += m_InnerAngleStep)
    {
        Elite::Vector2 target = Elite::Vector2(m_InnerSearchRadius * cosf(Elite::ToRadians(angle)),
                                               m_InnerSearchRadius * sinf(Elite::ToRadians(angle)));
        m_InnerRadiusSearchTargets.emplace(target);
    }

    // add outer radius search targets: in a radius of m_OuterSearchRadius with a center in (0,0) every m_OuterAngleStep degrees
    for (float angle = 0.f; angle < 360.f; angle += m_OuterAngleStep)
    {
        Elite::Vector2 target = Elite::Vector2(m_OuterSearchRadius * cosf(Elite::ToRadians(angle)),
                                               m_OuterSearchRadius * sinf(Elite::ToRadians(angle)));
        m_OuterRadiusSearchTargets.emplace(target);
    }
}

void MapSearchSystem::RenderDebug(IExamInterface *const pInterface) const
{
    // m_pInterface->Draw_SolidCircle for all the search targets
    for (const auto &target: m_InnerRadiusSearchTargets)
    {
        pInterface->Draw_SolidCircle(target, 1.f, {0, 0}, {1, 0, 0});
    }
    for (const auto &target: m_OuterRadiusSearchTargets)
    {
        pInterface->Draw_SolidCircle(target, 1.f, {0, 0}, {0, 1, 0});
    }
    for (const auto &target: m_VillageSearchTargets)
    {
        pInterface->Draw_SolidCircle(target, 1.f, {0, 0}, {0, 0, 1});
    }
    for (const auto &target: m_HouseSearchTargets)
    {
        pInterface->Draw_SolidCircle(target, 1.f, {0, 0}, {1, 1, 0});
    }
    for (const auto &house: m_FoundHouses)
    {
        const std::vector<Elite::Vector2> points = {
            {house.Center + Elite::Vector2(-house.Size.x * 0.5f, -house.Size.y * 0.5f)},
            {house.Center + Elite::Vector2(house.Size.x * 0.5f, -house.Size.y * 0.5f)},
            {house.Center + Elite::Vector2(house.Size.x * 0.5f, house.Size.y * 0.5f)},
            {house.Center + Elite::Vector2(-house.Size.x * 0.5f, house.Size.y * 0.5f)}
        };
        pInterface->Draw_Polygon(
            points.data(),
            4,
            {1, 0, 1}
        );
    }
    for (const auto &village: m_VillagesInfo)
    {
        const std::vector<Elite::Vector2> points = {
            {village.Center + Elite::Vector2(-village.Size.x * 0.5f, -village.Size.y * 0.5f)},
            {village.Center + Elite::Vector2(village.Size.x * 0.5f, -village.Size.y * 0.5f)},
            {village.Center + Elite::Vector2(village.Size.x * 0.5f, village.Size.y * 0.5f)},
            {village.Center + Elite::Vector2(-village.Size.x * 0.5f, village.Size.y * 0.5f)}
        };
        pInterface->Draw_Polygon(
            points.data(),
            4,
            {1, 1, 0}
        );
    }
}

bool MapSearchSystem::HasCheckedHouse(const HouseInfo &house) const
{
    return std::ranges::any_of(m_FoundHouses, [&house](const HouseInfo &h) { return h.Center == house.Center; });
}

void MapSearchSystem::Update(float dt)
{
    m_CurrTargetSearchTime += dt;
    m_CurrTargetRefreshTime += dt;
    if (m_CurrTargetSearchTime >= m_TargetSearchInterval)
    {
        m_CurrentTarget.reset();
        m_CurrTargetSearchTime -= m_TargetSearchInterval;
    }
    if (m_CurrTargetRefreshTime >= m_TargetSearchInterval)
    {
        CleanupObsoleteTargets();
        m_CurrTargetRefreshTime -= m_TargetSearchInterval;
    }
}

void MapSearchSystem::AddVillageSearchTargets(const HouseInfo &house)
{
    // Using signs to determine corner positions
    std::vector<Elite::Vector2> signs = {{-1, -1}, {1, -1}, {-1, 1}, {1, 1}}; // corners: TL, TR, BL, BR

    for (const auto &sign: signs)
    {
        constexpr float offset = 8.0f;
        const float halfWidth = house.Size.x * 0.5f;
        const float halfHeight = house.Size.y * 0.5f;

        m_VillageSearchTargets.emplace(
            house.Center.x + (halfWidth + offset) * sign.x,
            house.Center.y + (halfHeight + offset) * sign.y
        );
    }
}

void MapSearchSystem::AddHouseSearchTargets(const HouseInfo &house)
{
    // Using signs to determine corner positions
    std::vector<Elite::Vector2> signs{};
    // If the house is smaller than the agent's sight in one dimension, we only need two points
    if (house.Size.x <= m_AgentSightHyp)
    {
        signs = {{-0.f, -1.f}, {0.0f, 1.f}};
    } else if (house.Size.y <= m_AgentSightHyp)
    {
        signs = {{-1.0f, 0.f}, {1.0f, 0.f}};
    } else
    {
        signs = {{-1.0f, -1.0f}, {1.0f, -1.0f}, {-1.0f, 1.0f}, {1.0f, 1.0f}};
    }

    for (const auto &sign: signs)
    {
        constexpr float offset = -8.0f;
        const float halfWidth = house.Size.x * 0.5f;
        const float halfHeight = house.Size.y * 0.5f;

        m_HouseSearchTargets.emplace(
            house.Center.x + (halfWidth + offset) * sign.x,
            house.Center.y + (halfHeight + offset) * sign.y
        );
    }
}

void MapSearchSystem::FoundHouse(const HouseInfo &house)
{
    m_FoundHouses.emplace(house);
    AddHouseSearchTargets(house);

    // The inner village is more spread, this is extra check to make sure we dont miss it
    if ((m_InnerRadiusSearchTargets.empty() && m_VillageSearchTargets.empty()) || m_VillagesInfo.empty())
    {
        m_CurrVillageHousesCount = 0; // Reset the current village houses count
        m_VillagesInfo.push_back(house);
    } else
    {
        m_VillagesInfo.back().Center += house.Center;
        // Update the village center with the new house center
        m_VillagesInfo.back().Center /= 2.f; // Average the center position
        m_VillagesInfo.back().Size += house.Size; // Update the village size with the new house size
    }

    ++m_CurrVillageHousesCount;
    if (m_CurrVillageHousesCount >= 4)
    {
        m_CurrVillageHousesCount = 0; // Reset for the next village
        m_VillageSearchTargets.clear(); // Clear the village search targets for the next village
    } else AddVillageSearchTargets(house);

    if (m_VillagesInfo.size() >= 6 || m_FoundHouses.size() >= 20)
    {
        // we found all houses, no need to search anymore
        m_InnerRadiusSearchTargets.clear();
        m_OuterRadiusSearchTargets.clear();
    }
}

bool MapSearchSystem::IsDoneCheckingMap() const
{
    return m_OuterRadiusSearchTargets.empty() && m_InnerRadiusSearchTargets.empty();
}

bool MapSearchSystem::GetClosestHouse(const Elite::Vector2 &agentPosition, HouseInfo &outTarget) const
{
    if (m_FoundHouses.empty())
    {
        return false; // No houses found
    }
    auto closestHouse = std::ranges::min_element(m_FoundHouses,
                                                 [&agentPosition](const SetHouseInfo &a, const SetHouseInfo &b)
                                                 {
                                                     return (a.Center - agentPosition).MagnitudeSquared() < (
                                                                b.Center - agentPosition).MagnitudeSquared();
                                                 });

    outTarget = *closestHouse;
    return true;
}

bool MapSearchSystem::RemembersAnyHouses() const
{
    return !m_FoundHouses.empty();
}

bool MapSearchSystem::GetCurrentTarget(const Elite::Vector2 &agentPosition, Elite::Vector2 &outTarget)
{
    if (m_CurrentTarget.has_value())
    {
        outTarget = m_CurrentTarget.value();
        return true; // Current target is already set
    }
    if (GetCurrentHouseExploreTarget(agentPosition, outTarget))
    {
        m_CurrentTarget = outTarget;
        return true; // Found a target in the inner or outer radius
    }
    if (GetCurrentVillageExploreTarget(agentPosition, outTarget))
    {
        m_CurrentTarget = outTarget;
        return true; // Found a target in the village
    }
    if (GetCurrentExploreTarget(agentPosition, outTarget))
    {
        m_CurrentTarget = outTarget;
        return true; // Found a target in the inner or outer radius
    }
    // If no targets found, we ll recheck all houses
    for (const auto &house: m_FoundHouses)
    {
        m_HouseSearchTargets.emplace(house.Center);
    }
    if (GetCurrentHouseExploreTarget(agentPosition, outTarget))
    {
        m_CurrentTarget = outTarget;
        return true; // Found a target in the house search targets
    }
    return false; // No targets found
}

void MapSearchSystem::ReachedTarget(const Elite::Vector2 &target)
{
    m_CurrentTarget.reset();
    if (!m_HouseSearchTargets.empty()) m_HouseSearchTargets.erase(target);
    else if (!m_VillageSearchTargets.empty()) m_VillageSearchTargets.erase(target);
    else if (!m_InnerRadiusSearchTargets.empty()) m_InnerRadiusSearchTargets.erase(target);
    else if (!m_OuterRadiusSearchTargets.empty()) m_OuterRadiusSearchTargets.erase(target);
}

Elite::Vector2 MapSearchSystem::GetClosestPosFromVec(const Elite::Vector2 &agentPosition,
                                                     const std::set<Elite::Vector2> &vec)
{
    if (vec.empty()) return {};
    const auto it = std::ranges::min_element(vec, [&agentPosition](const Elite::Vector2 &a, const Elite::Vector2 &b)
    {
        return (a - agentPosition).MagnitudeSquared() < (b - agentPosition).MagnitudeSquared();
    });
    return (it != vec.end()) ? *it : Elite::Vector2{};
}

bool MapSearchSystem::GetCurrentExploreTarget(const Elite::Vector2 &agentPosition, Elite::Vector2 &outTarget) const
{
    if (m_InnerRadiusSearchTargets.empty() && m_OuterRadiusSearchTargets.empty())
    {
        return false; // No targets found
    }

    Elite::Vector2 closestTarget;
    // If inner radius targets are empty, use outer radius targets
    if (m_InnerRadiusSearchTargets.empty())
    {
        if (agentPosition.x <= 5 && agentPosition.y <= 5) closestTarget = *m_OuterRadiusSearchTargets.begin();
            // If the agent is at the origin, just return the first target
        else closestTarget = GetClosestPosFromVec(agentPosition, m_OuterRadiusSearchTargets);
    } else
    {
        // Use inner radius targets
        if (agentPosition.x <= 5 && agentPosition.y <= 5) closestTarget = *m_InnerRadiusSearchTargets.begin();
            // If the agent is at the origin, just return the first target
        else closestTarget = GetClosestPosFromVec(agentPosition, m_InnerRadiusSearchTargets);
    }
    outTarget = closestTarget;
    return true;
}

bool MapSearchSystem::GetCurrentHouseExploreTarget(const Elite::Vector2 &agentPosition, Elite::Vector2 &outTarget)
{
    if (m_HouseSearchTargets.empty()) return false;
    outTarget = GetClosestPosFromVec(agentPosition, m_HouseSearchTargets);
    return true;
}

bool MapSearchSystem::GetCurrentVillageExploreTarget(const Elite::Vector2 &agentPosition, Elite::Vector2 &outTarget)
{
    if (m_VillageSearchTargets.empty()) return false;
    outTarget = GetClosestPosFromVec(agentPosition, m_VillageSearchTargets);
    if (m_VillageSearchTargets.empty()) m_CurrVillageHousesCount = 0;
    // Reset the village houses count if we cleared the targets
    return true;
}


void MapSearchSystem::RememberItemLocation(const ItemInfo &itemInfo)
{
    m_FoundItemLocationMap[itemInfo.Type].emplace(itemInfo.Location);
}

bool MapSearchSystem::GetItemClosestLocation(const Elite::Vector2 &agentPosition, const eItemType &itemType,
                                             Elite::Vector2 &outTarget) const
{
    if (!m_FoundItemLocationMap.contains(itemType) || m_FoundItemLocationMap.at(itemType).empty())
    {
        return false; // No locations found for this item type
    }
    const auto &locations = m_FoundItemLocationMap.at(itemType);
    outTarget = GetClosestPosFromVec(agentPosition, locations);
    return true;
}

void MapSearchSystem::PickedUpItem(const ItemInfo &itemInfo)
{
    m_FoundItemLocationMap[itemInfo.Type].erase(itemInfo.Location);
}

bool MapSearchSystem::RemembersItem(const ItemInfo &item) const
{
    if (!m_FoundItemLocationMap.contains(item.Type) || m_FoundItemLocationMap.at(item.Type).empty())
    {
        return false; // No locations found for this item type
    }
    return m_FoundItemLocationMap.at(item.Type).contains(item.Location); // Check if the item location is known
}

bool MapSearchSystem::KnowsAnyItemLocation(const eItemType &itemType) const
{
    return m_FoundItemLocationMap.contains(itemType) && !m_FoundItemLocationMap.at(itemType).empty();
}

bool MapSearchSystem::IsPointInHouse(const Elite::Vector2 &point, const HouseInfo &house, float offset)
{
    const float halfWidth = house.Size.x * 0.5f + offset;
    const float halfHeight = house.Size.y * 0.5f + offset;
    return point.x >= (house.Center.x - halfWidth) &&
           point.x <= (house.Center.x + halfWidth) &&
           point.y >= (house.Center.y - halfHeight) &&
           point.y <= (house.Center.y + halfHeight);
}

// In MapSearchSystem.cpp

bool MapSearchSystem::IsPointInAnyFoundHouse(const Elite::Vector2 &point) const
{
    return std::ranges::any_of(m_FoundHouses, [this, &point](const HouseInfo &house)
    {
        return IsPointInHouse(point, house, 4.f); // Using an offset of 4.f to avoid being too close to the house walls
    });
}

void MapSearchSystem::CleanupObsoleteTargets()
{
    auto removeIfInHouse = [this](std::set<Elite::Vector2> &targets)
    {
        for (auto it = targets.begin(); it != targets.end();)
        {
            if (IsPointInAnyFoundHouse(*it))
                it = targets.erase(it);
            else
                ++it;
        }
    };
    removeIfInHouse(m_VillageSearchTargets);
    removeIfInHouse(m_InnerRadiusSearchTargets);
    removeIfInHouse(m_OuterRadiusSearchTargets);
}

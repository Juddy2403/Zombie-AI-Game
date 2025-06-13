#pragma once
#include "Exam_HelperStructs.h"

struct SetHouseInfo : public HouseInfo
{
    // Inherit constructors
    using HouseInfo::HouseInfo;

    // Construct from base HouseInfo
    explicit SetHouseInfo(const HouseInfo& h) { Center = h.Center; Size = h.Size; }

    // Strict weak ordering for set/map
    bool operator<(const SetHouseInfo& other) const
    {
        if (Center < other.Center) return true;
        if (other.Center < Center) return false;
        return Size < other.Size;
    }

    bool operator==(const SetHouseInfo& other) const
    {
        return Center == other.Center && Size == other.Size;
    }

    bool operator!=(const SetHouseInfo& other) const
    {
        return !(*this == other);
    }
};

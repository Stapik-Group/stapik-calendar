#pragma once

#include <string>
#include <tuple>

#include "EntryColor.hpp"
#include "EntryId.hpp"

struct CalendarEntry
{
    std::string name;
    std::string link;
    EntryColor color;
    // Optional, empty when not given. The time is "HH:MM" (see entry_time::normalize).
    std::string location;
    std::string time;
    EntryId id;

    bool operator==(const CalendarEntry&) const = default;

    [[nodiscard]] bool hasSameContentAs(const CalendarEntry& other) const
    {
        return std::tie(name, link, color, location, time) == std::tie(other.name, other.link, other.color, other.location, other.time);
    }
};

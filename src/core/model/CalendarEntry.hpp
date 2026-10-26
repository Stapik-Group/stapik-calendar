#pragma once

#include <nlohmann/json.hpp>

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
    nlohmann::json unknownFields = nlohmann::json::object();

    bool operator==(const CalendarEntry&) const = default;

    [[nodiscard]] bool hasSameContentAs(const CalendarEntry& other) const
    {
        return std::tie(name, link, color, location, time, unknownFields) == std::tie(other.name, other.link, other.color, other.location, other.time, other.unknownFields);
    }
};

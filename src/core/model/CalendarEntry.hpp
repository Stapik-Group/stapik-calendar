#pragma once

#include <string>

#include "EntryColor.hpp"

struct CalendarEntry
{
    std::string name;
    std::string link;
    EntryColor color;
    // Optional, empty when not given. The time is "HH:MM" (see entry_time::normalize).
    std::string location;
    std::string time;

    bool operator==(const CalendarEntry&) const = default;
};

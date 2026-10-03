#pragma once

#include <string>

#include "EntryColor.hpp"

struct CalendarEntry
{
    std::string name;
    std::string link;
    EntryColor color = EntryColor::Default;

    bool operator==(const CalendarEntry&) const = default;
};

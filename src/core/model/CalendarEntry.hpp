#pragma once

#include <string>

#include "EntryColor.hpp"

struct CalendarEntry
{
    std::string name;
    std::string link;
    EntryColor color;

    bool operator==(const CalendarEntry&) const = default;
};

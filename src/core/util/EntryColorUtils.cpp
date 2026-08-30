#include "EntryColorUtils.hpp"

namespace
{
    constexpr std::array ALL_COLORS {
        EntryColor::Default, EntryColor::Red, EntryColor::Green, EntryColor::Blue,
        EntryColor::Yellow, EntryColor::Purple, EntryColor::Orange
    };
}

std::string EntryColorUtils::toString(const EntryColor color)
{
    switch (color)
    {
        using enum EntryColor;
        case Red: return "red";
        case Green: return "green";
        case Blue: return "blue";
        case Yellow: return "yellow";
        case Purple: return "purple";
        case Orange: return "orange";
        case Default:
        default:
            return "default";
    }
}

EntryColor EntryColorUtils::fromString(const std::string& value)
{
    using enum EntryColor;
    if (value == "red") return Red;
    if (value == "green") return Green;
    if (value == "blue") return Blue;
    if (value == "yellow") return Yellow;
    if (value == "purple") return Purple;
    if (value == "orange") return Orange;
    return Default;
}

std::string EntryColorUtils::cssClass(const EntryColor color)
{
    if (color == EntryColor::Default)
        return "";
    return "calendar-entry-color-" + toString(color);
}

const std::array<EntryColor, 7>& EntryColorUtils::allColors()
{
    return ALL_COLORS;
}
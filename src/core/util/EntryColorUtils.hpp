#pragma once
#include <array>
#include <string>
#include "../model/EntryColor.hpp"

class EntryColorUtils
{
public:
    static std::string toString(EntryColor color);
    static EntryColor fromString(const std::string& value);
    static std::string cssClass(EntryColor color);
    static const std::array<EntryColor, 7>& allColors();
};
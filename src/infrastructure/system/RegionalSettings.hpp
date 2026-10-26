#pragma once

#include <chrono>

class RegionalSettings
{
public:
    [[nodiscard]] static std::chrono::weekday firstDayOfWeek();
};

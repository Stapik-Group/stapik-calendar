#pragma once
#include <chrono>

class DateUtils
{
public:
    static std::chrono::year_month_day dateOf(std::chrono::system_clock::time_point time);
    static std::chrono::year_month_day today();
    static std::chrono::year_month todayYearMonth();
};

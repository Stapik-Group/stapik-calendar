#pragma once
#include <chrono>

class DateUtils
{
public:
    static std::chrono::year_month todayYearMonth();
    static std::chrono::year_month_day today();
};

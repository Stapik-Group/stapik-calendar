#pragma once
#include <chrono>

class DateUtils
{
public:
    static std::chrono::year_month_day dateOf(std::chrono::system_clock::time_point time);
    static std::chrono::year_month_day today();
    static std::chrono::year_month todayYearMonth();

    [[nodiscard]] static std::chrono::year_month_day addDays(std::chrono::year_month_day date, int days);
    [[nodiscard]] static std::chrono::year_month_day addMonths(std::chrono::year_month_day date, int months);
    [[nodiscard]] static std::chrono::year_month_day addYears(std::chrono::year_month_day date, int years);
    [[nodiscard]] static std::chrono::year_month_day inMonth(std::chrono::year_month_day date, std::chrono::year_month yearMonth);
};

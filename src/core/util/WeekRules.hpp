#pragma once

#include <chrono>

class WeekRules
{
public:
    static constexpr int DAYS_IN_WEEK = 7;

    explicit WeekRules(std::chrono::weekday firstDay);

    [[nodiscard]] std::chrono::weekday firstDay() const;
    [[nodiscard]] std::chrono::weekday weekdayInColumn(int column) const;
    [[nodiscard]] int columnOf(std::chrono::weekday weekday) const;
    [[nodiscard]] std::chrono::year_month_day startOfWeek(std::chrono::year_month_day date) const;

    [[nodiscard]] static bool isWeekend(std::chrono::weekday weekday);

private:
    std::chrono::weekday m_firstDay;
};

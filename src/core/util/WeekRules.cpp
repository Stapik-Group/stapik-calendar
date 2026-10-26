#include "WeekRules.hpp"

WeekRules::WeekRules(const std::chrono::weekday firstDay) :
    m_firstDay(firstDay) {}

std::chrono::weekday WeekRules::firstDay() const
{
    return m_firstDay;
}

std::chrono::weekday WeekRules::weekdayInColumn(const int column) const
{
    return std::chrono::weekday{(m_firstDay.c_encoding() + static_cast<unsigned>(column)) % DAYS_IN_WEEK};
}

int WeekRules::columnOf(const std::chrono::weekday weekday) const
{
    return static_cast<int>((weekday.c_encoding() + DAYS_IN_WEEK - m_firstDay.c_encoding()) % DAYS_IN_WEEK);
}

std::chrono::year_month_day WeekRules::startOfWeek(const std::chrono::year_month_day date) const
{
    const std::chrono::sys_days day{date};
    return std::chrono::year_month_day{day - std::chrono::days{columnOf(std::chrono::weekday{day})}};
}

bool WeekRules::isWeekend(const std::chrono::weekday weekday)
{
    return weekday == std::chrono::Saturday || weekday == std::chrono::Sunday;
}

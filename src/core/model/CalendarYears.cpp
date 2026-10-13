#include "CalendarYears.hpp"

namespace calendar_years
{
    int yearOf(const std::chrono::year_month_day date)
    {
        return static_cast<int>(date.year());
    }

    std::map<int, CalendarEntries> splitByYear(const CalendarEntries& entries)
    {
        std::map<int, CalendarEntries> years;

        for (const auto& [date, dayEntries] : entries)
        {
            if (!dayEntries.empty())
                years[yearOf(date)].emplace(date, dayEntries);
        }

        return years;
    }

    CalendarEntries entriesOfYear(const CalendarEntries& entries, const int year)
    {
        CalendarEntries result;

        const auto first = entries.lower_bound(std::chrono::year_month_day{ std::chrono::year{ year }, std::chrono::January, std::chrono::day{ 1 } });
        const auto last = entries.upper_bound(std::chrono::year_month_day{ std::chrono::year{ year }, std::chrono::December, std::chrono::day{ 31 } });

        for (auto it = first; it != last; ++it)
        {
            if (!it->second.empty())
                result.emplace(it->first, it->second);
        }

        return result;
    }

    void replaceYear(CalendarEntries& entries, const int year, const CalendarEntries& replacement)
    {
        const auto first = entries.lower_bound(std::chrono::year_month_day{ std::chrono::year{ year }, std::chrono::January, std::chrono::day{ 1 } });
        const auto last = entries.upper_bound(std::chrono::year_month_day{ std::chrono::year{ year }, std::chrono::December, std::chrono::day{ 31 } });
        entries.erase(first, last);

        for (const auto& [date, dayEntries] : replacement)
        {
            if (yearOf(date) == year && !dayEntries.empty())
                entries.emplace(date, dayEntries);
        }
    }
}

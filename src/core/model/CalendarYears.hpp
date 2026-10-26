#pragma once

#include "CalendarEntries.hpp"

#include <map>

// The calendar is stored and (later) synchronized one year at a time.
namespace calendar_years
{
    [[nodiscard]] int yearOf(std::chrono::year_month_day date);

    // Entries grouped by the year of their date. Days without entries are dropped.
    [[nodiscard]] std::map<int, CalendarEntries> splitByYear(const CalendarEntries& entries);

    [[nodiscard]] CalendarEntries entriesOfYear(const CalendarEntries& entries, int year);

    [[nodiscard]] bool haveSameContent(const CalendarEntries& left, const CalendarEntries& right);

    // Replaces all entries of `year` in `entries` with `replacement` (which should contain only that year).
    void replaceYear(CalendarEntries& entries, int year, const CalendarEntries& replacement);
}

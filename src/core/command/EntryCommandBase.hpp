#pragma once

#include "CalendarCommand.hpp"
#include "../model/CalendarEntries.hpp"

class EntryCommandBase : public CalendarCommand
{
protected:
    EntryCommandBase(CalendarEntries& entries, std::chrono::year_month_day date);
    CalendarEntries& m_entries;
    std::chrono::year_month_day m_date;
};
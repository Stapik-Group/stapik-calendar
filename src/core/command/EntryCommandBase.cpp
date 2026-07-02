#include "EntryCommandBase.hpp"

EntryCommandBase::EntryCommandBase(CalendarEntries& entries, const std::chrono::year_month_day date) :
    m_entries(entries),
    m_date(date) {}
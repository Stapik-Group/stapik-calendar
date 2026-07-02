#pragma once

#include "EntryCommandBase.hpp"

#include <cstddef>
#include <vector>

class IndexedEntryCommand : public EntryCommandBase
{
protected:
    IndexedEntryCommand(CalendarEntries& entries, std::chrono::year_month_day date, std::size_t entryIndex);
    [[nodiscard]] std::vector<CalendarEntry>& dayEntries() const;
    std::size_t m_entryIndex;
};
#include "IndexedEntryCommand.hpp"

IndexedEntryCommand::IndexedEntryCommand(CalendarEntries& entries,
    const std::chrono::year_month_day date,
    const std::size_t entryIndex) :
    EntryCommandBase(entries, date),
    m_entryIndex(entryIndex) {}

std::vector<CalendarEntry>& IndexedEntryCommand::dayEntries() const
{
    return m_entries.at(m_date);
}
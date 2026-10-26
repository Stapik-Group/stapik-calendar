#include "IndexedEntryCommand.hpp"

IndexedEntryCommand::IndexedEntryCommand(CalendarEntries& entries,
    const std::chrono::year_month_day date,
    const std::size_t entryIndex) :
    EntryCommandBase(entries, date),
    m_entryIndex(entryIndex) {}

std::vector<CalendarEntry>& IndexedEntryCommand::dayEntries() const
{
    return calendarEntries().at(entryDate());
}

std::size_t IndexedEntryCommand::entryIndex() const
{
    return m_entryIndex;
}
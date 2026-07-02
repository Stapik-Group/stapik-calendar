#include "EditEntryCommand.hpp"

EditEntryCommand::EditEntryCommand(CalendarEntries& entries,
    const std::chrono::year_month_day date,
    const std::size_t entryIndex,
    CalendarEntry newEntry) :
    IndexedEntryCommand(entries, date, entryIndex),
    m_newEntry(std::move(newEntry)),
    m_oldEntry(dayEntries().at(entryIndex)) {}

void EditEntryCommand::execute()
{
    dayEntries().at(m_entryIndex) = m_newEntry;
}

void EditEntryCommand::undo()
{
    dayEntries().at(m_entryIndex) = m_oldEntry;
}
#include "EditEntryCommand.hpp"

EditEntryCommand::EditEntryCommand(Entries& entries, std::chrono::year_month_day date, int entryIndex, CalendarEntry newEntry):
    m_entries(entries),
    m_date(date),
    m_entryIndex(entryIndex),
    m_newEntry(std::move(newEntry)),
    m_oldEntry(m_entries.at(date)[entryIndex])
{}

void EditEntryCommand::execute()
{
    m_entries[m_date][m_entryIndex] = m_newEntry;
}

void EditEntryCommand::undo()
{
    m_entries[m_date][m_entryIndex] = m_oldEntry;
}
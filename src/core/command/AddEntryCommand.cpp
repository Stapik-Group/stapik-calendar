#include "AddEntryCommand.hpp"

AddEntryCommand::AddEntryCommand(CalendarEntries& entries, const std::chrono::year_month_day date, CalendarEntry entry) :
    EntryCommandBase(entries, date),
    m_entry(std::move(entry)) {}

void AddEntryCommand::execute()
{
    m_entries[m_date].push_back(m_entry);
}

void AddEntryCommand::undo()
{
    m_entries[m_date].pop_back();
}
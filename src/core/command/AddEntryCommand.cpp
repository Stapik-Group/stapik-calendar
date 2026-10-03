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
    auto& dayEntries = m_entries[m_date];
    dayEntries.pop_back();

    if (dayEntries.empty())
        m_entries.erase(m_date);
}

std::string AddEntryCommand::description() const
{
    return describe("command.entry.add", m_entry.name);
}
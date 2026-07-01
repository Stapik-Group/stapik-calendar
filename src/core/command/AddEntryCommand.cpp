#include "AddEntryCommand.hpp"

AddEntryCommand::AddEntryCommand(Entries &entries, const std::chrono::year_month_day date, CalendarEntry entry) :
    m_entries(entries),
    m_date(date),
    m_entry(std::move(entry)) {}

void AddEntryCommand::execute()
{
    m_entries[m_date].push_back(m_entry);
}

void AddEntryCommand::undo()
{
    if (auto& dayEntries = m_entries[m_date]; !dayEntries.empty())
    {
        dayEntries.pop_back();
    } else
    {
        m_entries.erase(m_date);
    }
}

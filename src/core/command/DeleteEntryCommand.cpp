#include "DeleteEntryCommand.hpp"

DeleteEntryCommand::DeleteEntryCommand(Entries& entries, const std::chrono::year_month_day date, const int entryIndex) :
    m_entries(entries),
    m_date(date),
    m_entryIndex(entryIndex),
    m_deletedEntry(m_entries.at(date)[entryIndex]) {}

void DeleteEntryCommand::execute()
{
    auto& dayEntries = m_entries[m_date];
    dayEntries.erase(dayEntries.begin() + m_entryIndex);

    if (dayEntries.empty())
        m_entries.erase(m_date);
}

void DeleteEntryCommand::undo()
{
    auto& dayEntries = m_entries[m_date];
    dayEntries.insert(dayEntries.begin() + m_entryIndex, m_deletedEntry);
}
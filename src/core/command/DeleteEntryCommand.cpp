#include "DeleteEntryCommand.hpp"

DeleteEntryCommand::DeleteEntryCommand(CalendarEntries& entries,
    const std::chrono::year_month_day date,
    const std::size_t entryIndex) :
    IndexedEntryCommand(entries, date, entryIndex),
    m_deletedEntry(dayEntries().at(entryIndex)) {}

void DeleteEntryCommand::execute()
{
    auto& entries = dayEntries();
    entries.erase(entries.begin() + static_cast<std::vector<CalendarEntry>::difference_type>(m_entryIndex));

    if (entries.empty())
        m_entries.erase(m_date);
}

void DeleteEntryCommand::undo()
{
    auto& entries = m_entries[m_date];
    entries.insert(entries.begin() + static_cast<std::vector<CalendarEntry>::difference_type>(m_entryIndex), m_deletedEntry);
}
#include "MoveEntryCommand.hpp"
#include <vector>

MoveEntryCommand::MoveEntryCommand(CalendarEntries& entries,
    const std::chrono::year_month_day sourceDate,
    const std::size_t sourceIndex,
    const std::chrono::year_month_day destDate) :
    m_entries(entries),
    m_sourceDate(sourceDate),
    m_sourceIndex(sourceIndex),
    m_destDate(destDate),
    m_movedEntry(entries.at(sourceDate).at(sourceIndex)) {}

void MoveEntryCommand::execute()
{
    auto& sourceEntries = m_entries.at(m_sourceDate);
    sourceEntries.erase(sourceEntries.begin() + static_cast<std::vector<CalendarEntry>::difference_type>(m_sourceIndex));
    if (sourceEntries.empty())
        m_entries.erase(m_sourceDate);

    m_entries[m_destDate].push_back(m_movedEntry);
}

void MoveEntryCommand::undo()
{
    auto& destEntries = m_entries.at(m_destDate);
    destEntries.pop_back();
    if (destEntries.empty())
        m_entries.erase(m_destDate);

    auto& sourceEntries = m_entries[m_sourceDate];
    sourceEntries.insert(sourceEntries.begin() + static_cast<std::vector<CalendarEntry>::difference_type>(m_sourceIndex), m_movedEntry);
}
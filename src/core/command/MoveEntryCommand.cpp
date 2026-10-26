#include "MoveEntryCommand.hpp"
#include <vector>

MoveEntryCommand::MoveEntryCommand(CalendarEntries& entries,
    const std::chrono::year_month_day sourceDate,
    const std::size_t sourceIndex,
    const std::chrono::year_month_day destDate) :
    EntryCommandBase(entries, sourceDate),
    m_sourceIndex(sourceIndex),
    m_destDate(destDate),
    m_movedEntry(entries.at(sourceDate).at(sourceIndex)) {}

void MoveEntryCommand::execute()
{
    auto& sourceEntries = calendarEntries().at(entryDate());
    sourceEntries.erase(sourceEntries.begin() + static_cast<std::vector<CalendarEntry>::difference_type>(m_sourceIndex));
    if (sourceEntries.empty())
        calendarEntries().erase(entryDate());

    calendarEntries()[m_destDate].push_back(m_movedEntry);
}

void MoveEntryCommand::undo()
{
    auto& destEntries = calendarEntries().at(m_destDate);
    destEntries.pop_back();
    if (destEntries.empty())
        calendarEntries().erase(m_destDate);

    auto& sourceEntries = calendarEntries()[entryDate()];
    sourceEntries.insert(sourceEntries.begin() + static_cast<std::vector<CalendarEntry>::difference_type>(m_sourceIndex), m_movedEntry);
}

std::string MoveEntryCommand::description() const
{
    return describe("command.entry.move", m_movedEntry.name);
}
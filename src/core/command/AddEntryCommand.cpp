#include "AddEntryCommand.hpp"

AddEntryCommand::AddEntryCommand(CalendarEntries& entries, const std::chrono::year_month_day date, CalendarEntry entry) :
    EntryCommandBase(entries, date),
    m_entry(std::move(entry)) {}

void AddEntryCommand::execute()
{
    calendarEntries()[entryDate()].push_back(m_entry);
}

void AddEntryCommand::undo()
{
    auto& dayEntries = calendarEntries()[entryDate()];
    dayEntries.pop_back();

    if (dayEntries.empty())
        calendarEntries().erase(entryDate());
}

std::string AddEntryCommand::description() const
{
    return describe("command.entry.add", m_entry.name);
}
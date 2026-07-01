#pragma once

#include "CalendarCommand.hpp"
#include "../model/CalendarEntry.hpp"

#include <chrono>
#include <map>


class AddEntryCommand : public CalendarCommand
{
public:
    using Entries = std::map<std::chrono::year_month_day, std::vector<CalendarEntry>>;
    AddEntryCommand(Entries& entries, std::chrono::year_month_day date, CalendarEntry entry);
    void execute() override;
    void undo() override;
private:
    Entries& m_entries;
    std::chrono::year_month_day m_date;
    CalendarEntry m_entry;
};

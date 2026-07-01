#pragma once

#include "CalendarCommand.hpp"
#include "../model/CalendarEntry.hpp"

#include <chrono>
#include <map>
#include <vector>

class DeleteEntryCommand : public CalendarCommand
{
public:
    using Entries = std::map<std::chrono::year_month_day, std::vector<CalendarEntry>>;
    DeleteEntryCommand(Entries& entries, std::chrono::year_month_day date, int entryIndex);
    void execute() override;
    void undo() override;
private:
    Entries& m_entries;
    std::chrono::year_month_day m_date;
    int m_entryIndex;
    CalendarEntry m_deletedEntry;
};

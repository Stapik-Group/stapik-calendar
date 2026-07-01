#pragma once

#include "CalendarCommand.hpp"
#include "../model/CalendarEntry.hpp"

#include <chrono>
#include <map>
#include <vector>

class EditEntryCommand : public CalendarCommand
{
public:
    using Entries = std::map<std::chrono::year_month_day, std::vector<CalendarEntry>>;
    EditEntryCommand(Entries& entries, std::chrono::year_month_day date, int entryIndex, CalendarEntry newEntry);
    void execute() override;
    void undo() override;
private:
    Entries& m_entries;
    std::chrono::year_month_day m_date;
    int m_entryIndex;
    CalendarEntry m_newEntry;
    CalendarEntry m_oldEntry;
};

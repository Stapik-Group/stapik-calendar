#pragma once

#include "IndexedEntryCommand.hpp"

class EditEntryCommand : public IndexedEntryCommand
{
public:
    EditEntryCommand(CalendarEntries& entries, std::chrono::year_month_day date, std::size_t entryIndex, CalendarEntry newEntry);
    void execute() override;
    void undo() override;
private:
    CalendarEntry m_newEntry;
    CalendarEntry m_oldEntry;
};
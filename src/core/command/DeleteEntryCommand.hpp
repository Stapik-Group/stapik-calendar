#pragma once

#include "IndexedEntryCommand.hpp"

class DeleteEntryCommand : public IndexedEntryCommand
{
public:
    DeleteEntryCommand(CalendarEntries& entries, std::chrono::year_month_day date, std::size_t entryIndex);
    void execute() override;
    void undo() override;
private:
    CalendarEntry m_deletedEntry;
};
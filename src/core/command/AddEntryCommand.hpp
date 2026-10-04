#pragma once

#include "EntryCommandBase.hpp"

class AddEntryCommand : public EntryCommandBase
{
public:
    AddEntryCommand(CalendarEntries& entries, std::chrono::year_month_day date, CalendarEntry entry);
    void execute() override;
    void undo() override;
    [[nodiscard]] std::string description() const override;
private:
    CalendarEntry m_entry;
};
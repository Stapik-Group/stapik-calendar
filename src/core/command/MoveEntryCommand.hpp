#pragma once

#include "EntryCommandBase.hpp"
#include "../model/CalendarEntry.hpp"

#include <cstddef>

class MoveEntryCommand : public EntryCommandBase
{
public:
    MoveEntryCommand(CalendarEntries& entries,
                      std::chrono::year_month_day sourceDate,
                      std::size_t sourceIndex,
                      std::chrono::year_month_day destDate);
    void execute() override;
    void undo() override;
    [[nodiscard]] std::string description() const override;
private:
    std::size_t m_sourceIndex;
    std::chrono::year_month_day m_destDate;
    CalendarEntry m_movedEntry;
};
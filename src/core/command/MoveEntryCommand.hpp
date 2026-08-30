#pragma once
#include "CalendarCommand.hpp"
#include "../model/CalendarEntries.hpp"
#include "../model/CalendarEntry.hpp"
#include <cstddef>

class MoveEntryCommand : public CalendarCommand
{
public:
    MoveEntryCommand(CalendarEntries& entries,
                      std::chrono::year_month_day sourceDate,
                      std::size_t sourceIndex,
                      std::chrono::year_month_day destDate);
    void execute() override;
    void undo() override;
private:
    CalendarEntries& m_entries;
    std::chrono::year_month_day m_sourceDate;
    std::size_t m_sourceIndex;
    std::chrono::year_month_day m_destDate;
    CalendarEntry m_movedEntry;
};
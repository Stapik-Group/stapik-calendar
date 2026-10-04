#pragma once

#include "stapik/command/ICommand.hpp"
#include "../model/CalendarEntries.hpp"

#include <chrono>
#include <string>

class EntryCommandBase : public stapik::command::ICommand
{
protected:
    EntryCommandBase(CalendarEntries& entries, std::chrono::year_month_day date);

    [[nodiscard]] static std::string describe(const std::string& key, const std::string& entryName);

    CalendarEntries& m_entries;
    std::chrono::year_month_day m_date;
};

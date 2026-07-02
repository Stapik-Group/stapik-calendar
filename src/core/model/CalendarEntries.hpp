#pragma once

#include "CalendarEntry.hpp"

#include <chrono>
#include <map>
#include <vector>

using CalendarEntries = std::map<std::chrono::year_month_day, std::vector<CalendarEntry>>;
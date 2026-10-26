#include "DateUtils.hpp"

#include "stapik/locale/DateFormatter.hpp"

#include <algorithm>

namespace
{
    std::chrono::year_month_day withClampedDay(const std::chrono::year_month yearMonth, const std::chrono::day day)
    {
        const auto lastDay = std::chrono::year_month_day{yearMonth / std::chrono::last}.day();
        return yearMonth / std::min(day, lastDay);
    }
}

std::chrono::year_month_day DateUtils::dateOf(const std::chrono::system_clock::time_point time)
{
    using namespace std::chrono;
    return year_month_day{floor<days>(stapik::locale::toLocalTime(time))};
}

std::chrono::year_month_day DateUtils::today()
{
    return dateOf(std::chrono::system_clock::now());
}

std::chrono::year_month DateUtils::todayYearMonth()
{
    const auto date = today();
    return std::chrono::year_month{date.year(), date.month()};
}

std::chrono::year_month_day DateUtils::addDays(const std::chrono::year_month_day date, const int days)
{
    return std::chrono::year_month_day{std::chrono::sys_days{date} + std::chrono::days{days}};
}

std::chrono::year_month_day DateUtils::addMonths(const std::chrono::year_month_day date, const int months)
{
    return withClampedDay(std::chrono::year_month{date.year(), date.month()} + std::chrono::months{months}, date.day());
}

std::chrono::year_month_day DateUtils::addYears(const std::chrono::year_month_day date, const int years)
{
    return withClampedDay(std::chrono::year_month{date.year() + std::chrono::years{years}, date.month()}, date.day());
}

std::chrono::year_month_day DateUtils::inMonth(const std::chrono::year_month_day date, const std::chrono::year_month yearMonth)
{
    return withClampedDay(yearMonth, date.day());
}

#include "DateUtils.hpp"

#include "stapik/locale/DateFormatter.hpp"

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

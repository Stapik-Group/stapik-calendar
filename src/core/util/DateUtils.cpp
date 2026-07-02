#include "DateUtils.hpp"

std::chrono::year_month DateUtils::todayYearMonth()
{
    using namespace std::chrono;
    const auto today = year_month_day{floor<days>(system_clock::now())};
    return year_month{today.year(), today.month()};
}

std::chrono::year_month_day DateUtils::today()
{
    using namespace std::chrono;
    return year_month_day{floor<days>(system_clock::now())};
}

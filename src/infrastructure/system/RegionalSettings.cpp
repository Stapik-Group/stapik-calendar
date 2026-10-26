#include "RegionalSettings.hpp"

#include <clocale>
#include <optional>
#include <string_view>

#if defined(_WIN32)
#include <windows.h>
#elif defined(__GLIBC__)
#include <langinfo.h>
#endif

namespace
{
    constexpr auto DEFAULT_FIRST_DAY = std::chrono::Monday;

#if defined(_WIN32)
    std::optional<std::chrono::weekday> detectFirstDay()
    {
        constexpr unsigned MONDAY_BASED_LAST_VALUE = 6;
        constexpr int BUFFER_LENGTH = sizeof(DWORD) / sizeof(wchar_t);

        DWORD value = 0;
        const int written = GetLocaleInfoEx(LOCALE_NAME_USER_DEFAULT, LOCALE_IFIRSTDAYOFWEEK | LOCALE_RETURN_NUMBER,
                                            reinterpret_cast<LPWSTR>(&value), BUFFER_LENGTH);
        if (written == 0 || value > MONDAY_BASED_LAST_VALUE)
            return std::nullopt;

        return std::chrono::weekday{(value + 1) % 7};
    }
#elif defined(__GLIBC__)
    bool isNeutralLocale()
    {
        const std::string_view name = std::setlocale(LC_TIME, nullptr);
        return name == "C" || name == "POSIX";
    }

    std::optional<std::chrono::weekday> detectFirstDay()
    {
        constexpr std::uintptr_t MONDAY_ORIGIN = 19971201;

        if (isNeutralLocale())
            return std::nullopt;

        const auto origin = reinterpret_cast<std::uintptr_t>(nl_langinfo(_NL_TIME_WEEK_1STDAY));
        const unsigned firstWeekday = static_cast<unsigned char>(nl_langinfo(_NL_TIME_FIRST_WEEKDAY)[0]);

        if (constexpr std::uintptr_t SUNDAY_ORIGIN = 19971130; origin != SUNDAY_ORIGIN && origin != MONDAY_ORIGIN)
            return std::nullopt;

        const unsigned originWeekday = origin == MONDAY_ORIGIN ? 1 : 0;
        return std::chrono::weekday{(originWeekday + firstWeekday - 1) % 7};
    }
#else
    std::optional<std::chrono::weekday> detectFirstDay()
    {
        return std::nullopt;
    }
#endif
}

std::chrono::weekday RegionalSettings::firstDayOfWeek()
{
    return detectFirstDay().value_or(DEFAULT_FIRST_DAY);
}

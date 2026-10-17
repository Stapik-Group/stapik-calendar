#include "EntryTime.hpp"

#include <algorithm>
#include <cctype>
#include <cstdio>

namespace
{
    constexpr int MAX_HOUR = 23;
    constexpr int MAX_MINUTE = 59;
    constexpr std::size_t COMPACT_SHORT_LENGTH = 3;
    constexpr std::size_t COMPACT_LONG_LENGTH = 4;
    constexpr std::size_t MAX_PART_LENGTH = 2;

    bool isBlank(const char c)
    {
        return std::isspace(static_cast<unsigned char>(c)) != 0;
    }

    bool isDigit(const char c)
    {
        return std::isdigit(static_cast<unsigned char>(c)) != 0;
    }

    bool allDigits(const std::string_view text)
    {
        return !text.empty() && std::ranges::all_of(text, isDigit);
    }

    int toNumber(const std::string_view digits)
    {
        int value = 0;
        for (const char c : digits)
            value = value * 10 + (c - '0');
        return value;
    }
}

std::optional<std::string> entry_time::normalize(std::string_view text)
{
    while (!text.empty() && isBlank(text.front()))
        text.remove_prefix(1);
    while (!text.empty() && isBlank(text.back()))
        text.remove_suffix(1);

    if (text.empty())
        return std::string{};

    std::string_view hourPart;
    std::string_view minutePart;

    if (const auto separator = text.find_first_of(":."); separator != std::string_view::npos)
    {
        hourPart = text.substr(0, separator);
        minutePart = text.substr(separator + 1);
    }
    else if (allDigits(text) && (text.size() == COMPACT_SHORT_LENGTH || text.size() == COMPACT_LONG_LENGTH))
    {
        hourPart = text.substr(0, text.size() - MAX_PART_LENGTH);
        minutePart = text.substr(text.size() - MAX_PART_LENGTH);
    }
    else
    {
        return std::nullopt;
    }

    if (!allDigits(hourPart) || !allDigits(minutePart) || hourPart.size() > MAX_PART_LENGTH || minutePart.size() != MAX_PART_LENGTH)
        return std::nullopt;

    const int hour = toNumber(hourPart);
    const int minute = toNumber(minutePart);
    if (hour > MAX_HOUR || minute > MAX_MINUTE)
        return std::nullopt;

    char buffer[8];
    std::snprintf(buffer, sizeof(buffer), "%02d:%02d", hour, minute);
    return std::string(buffer);
}

#include "EntryDragPayload.hpp"
#include <charconv>
#include <format>
#include <string_view>

namespace
{
    constexpr char SEPARATOR = ':';
    constexpr std::string_view COPY_FLAG = "1";
    constexpr std::string_view MOVE_FLAG = "0";
}

Glib::ustring EntryDragPayload::serialize(const int cellIndex, const EntryId& entryId, const bool isCopy)
{
    return std::format("{}{}{}{}{}", cellIndex, SEPARATOR, entryId, SEPARATOR, isCopy ? COPY_FLAG : MOVE_FLAG);
}

std::optional<std::tuple<int, EntryId, bool>> EntryDragPayload::deserialize(const Glib::ustring& payload)
{
    const std::string_view raw = payload.raw();
    const auto firstSeparator = raw.find(SEPARATOR);
    const auto lastSeparator = raw.rfind(SEPARATOR);
    if (firstSeparator == std::string_view::npos || firstSeparator == lastSeparator)
        return std::nullopt;

    const auto cellText = raw.substr(0, firstSeparator);
    const auto entryId = raw.substr(firstSeparator + 1, lastSeparator - firstSeparator - 1);
    const auto copyFlag = raw.substr(lastSeparator + 1);
    if (entryId.empty() || (copyFlag != COPY_FLAG && copyFlag != MOVE_FLAG))
        return std::nullopt;

    int cellIndex = 0;
    const auto* cellTextEnd = cellText.data() + cellText.size();
    if (const auto [parsedEnd, error] = std::from_chars(cellText.data(), cellTextEnd, cellIndex); error != std::errc{} || parsedEnd != cellTextEnd)
        return std::nullopt;

    return std::make_tuple(cellIndex, EntryId(entryId), copyFlag == COPY_FLAG);
}

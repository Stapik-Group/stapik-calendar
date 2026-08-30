#include "EntryDragPayload.hpp"
#include <charconv>

namespace
{
    constexpr char SEPARATOR = ':';
}

Glib::ustring EntryDragPayload::serialize(const int cellIndex, const int entryIndex, const bool isCopy)
{
    return std::to_string(cellIndex) + SEPARATOR + std::to_string(entryIndex) + SEPARATOR + (isCopy ? "1" : "0");
}

std::optional<std::tuple<int, int, bool>> EntryDragPayload::deserialize(const Glib::ustring& payload)
{
    const auto& raw = payload.raw();
    const auto pos1 = raw.find(SEPARATOR);
    if (pos1 == std::string::npos)
        return std::nullopt;
    const auto pos2 = raw.find(SEPARATOR, pos1 + 1);
    if (pos2 == std::string::npos)
        return std::nullopt;

    int cellIndex = 0;
    int entryIndex = 0;

    if (std::from_chars(raw.data(), raw.data() + pos1, cellIndex).ec != std::errc{})
        return std::nullopt;
    if (std::from_chars(raw.data() + pos1 + 1, raw.data() + pos2, entryIndex).ec != std::errc{})
        return std::nullopt;

    const bool isCopy = raw.ends_with('1');
    return std::make_tuple(cellIndex, entryIndex, isCopy);
}
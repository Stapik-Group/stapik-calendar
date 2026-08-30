#include "EntryDragPayload.hpp"
#include <charconv>

namespace
{
    constexpr char SEPARATOR = ':';
}

Glib::ustring EntryDragPayload::serialize(const int cellIndex, const int entryIndex)
{
    return std::to_string(cellIndex) + SEPARATOR + std::to_string(entryIndex);
}

std::optional<std::pair<int, int>> EntryDragPayload::deserialize(const Glib::ustring& payload)
{
    const auto raw = payload.raw();
    const auto pos = raw.find(SEPARATOR);
    if (pos == std::string::npos)
        return std::nullopt;

    int cellIndex = 0;
    int entryIndex = 0;
    const auto* begin1 = raw.data();
    const auto* end1 = raw.data() + pos;
    const auto* begin2 = raw.data() + pos + 1;
    const auto* end2 = raw.data() + raw.size();

    if (std::from_chars(begin1, end1, cellIndex).ec != std::errc{})
        return std::nullopt;
    if (std::from_chars(begin2, end2, entryIndex).ec != std::errc{})
        return std::nullopt;

    return std::make_pair(cellIndex, entryIndex);
}
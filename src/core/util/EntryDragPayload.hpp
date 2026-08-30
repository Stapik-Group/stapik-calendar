#pragma once
#include <glibmm/ustring.h>
#include <optional>
#include <utility>

class EntryDragPayload
{
public:
    static Glib::ustring serialize(int cellIndex, int entryIndex);
    static std::optional<std::pair<int, int>> deserialize(const Glib::ustring& payload);
};
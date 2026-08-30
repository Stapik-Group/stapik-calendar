#pragma once
#include <glibmm/ustring.h>
#include <optional>
#include <utility>

class EntryDragPayload
{
public:
    static Glib::ustring serialize(int cellIndex, int entryIndex, bool isCopy);
    static std::optional<std::tuple<int, int, bool>> deserialize(const Glib::ustring& payload);
};
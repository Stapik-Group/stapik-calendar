#pragma once
#include "../model/EntryId.hpp"

#include <glibmm/ustring.h>
#include <optional>
#include <tuple>

class EntryDragPayload
{
public:
    static Glib::ustring serialize(int cellIndex, const EntryId& entryId, bool isCopy);
    static std::optional<std::tuple<int, EntryId, bool>> deserialize(const Glib::ustring& payload);
};

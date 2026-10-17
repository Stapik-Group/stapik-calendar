#pragma once

#include <optional>
#include <string>
#include <string_view>

// The optional time of an entry, kept as text "HH:MM" (24 hours). Such text sorts in the order of the clock.
namespace entry_time
{
    // Accepts "9:30", "09:30", "9.30" and "930"; an empty text means "no time" and stays empty.
    // Returns nullopt for anything that is not a valid time of day.
    [[nodiscard]] std::optional<std::string> normalize(std::string_view text);
}

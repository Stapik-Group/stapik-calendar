#pragma once

#include "../../core/model/EntryColor.hpp"

#include <string>

// Common themes style every category color (and the default one) through `.stapik-category-<id>`.
[[nodiscard]] inline std::string entryColorCssClass(const EntryColor color)
{
    return color.has_value() ? stapik::domain::categoryColorCssClass(*color) : "stapik-category-default";
}

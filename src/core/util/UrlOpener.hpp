#pragma once

#include <optional>
#include <string>
#include <string_view>

class UrlOpener
{
public:
    static bool open(const std::string& link);
    [[nodiscard]] static std::optional<std::string> normalize(std::string_view link);
};

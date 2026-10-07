#pragma once

#include <filesystem>
#include <string_view>

class UserGuide
{
public:
    static bool open(std::string_view languageCode);
    [[nodiscard]] static std::filesystem::path pathFor(std::string_view languageCode);
};

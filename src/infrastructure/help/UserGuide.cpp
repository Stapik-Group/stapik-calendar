#include "UserGuide.hpp"

#include "stapik/log/Log.hpp"
#include "stapik/storage/AppPaths.hpp"

#include <giomm/appinfo.h>
#include <glibmm/convert.h>
#include <glibmm/error.h>

#include <algorithm>
#include <cctype>
#include <string>

namespace
{
    constexpr auto GUIDE_DIRECTORY = "guide";
    constexpr auto FALLBACK_LANGUAGE = "en";

    bool isSafeLanguageCode(const std::string_view code)
    {
        return !code.empty() && std::ranges::all_of(code, [](const char c)
        {
            return std::isalnum(static_cast<unsigned char>(c)) != 0 || c == '-' || c == '_';
        });
    }

    std::filesystem::path guideFile(const std::string_view languageCode)
    {
        return AppPaths::resourcesDir() / GUIDE_DIRECTORY / ("guide-" + std::string(languageCode) + ".html");
    }
}

std::filesystem::path UserGuide::pathFor(const std::string_view languageCode)
{
    if (isSafeLanguageCode(languageCode))
    {
        if (auto path = guideFile(languageCode); std::filesystem::exists(path))
            return path;
    }

    return guideFile(FALLBACK_LANGUAGE);
}

bool UserGuide::open(const std::string_view languageCode)
{
    const auto path = std::filesystem::absolute(pathFor(languageCode));
    if (!std::filesystem::exists(path))
    {
        stapik::log::warning("The user guide was not found: {}", path.string());
        return false;
    }

    try
    {
        return Gio::AppInfo::launch_default_for_uri(Glib::filename_to_uri(path.string()));
    }
    catch (const Glib::Error& error)
    {
        stapik::log::warning("Cannot open the user guide {}: {}", path.string(), error.what());
        return false;
    }
}

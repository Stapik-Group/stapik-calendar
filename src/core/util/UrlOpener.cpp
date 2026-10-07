#include "UrlOpener.hpp"

#include "stapik/log/Log.hpp"

#include <giomm/appinfo.h>
#include <glibmm/error.h>

#include <algorithm>
#include <cctype>

namespace
{
    constexpr std::string_view SCHEME_SEPARATOR = "://";
    constexpr std::string_view HTTP_SCHEME = "http";
    constexpr std::string_view HTTPS_SCHEME = "https";
    constexpr std::string_view DEFAULT_SCHEME_PREFIX = "https://";

    bool isBlank(const char character)
    {
        return std::isspace(static_cast<unsigned char>(character)) != 0;
    }

    bool isControl(const char character)
    {
        return std::iscntrl(static_cast<unsigned char>(character)) != 0;
    }

    bool equalsIgnoreCase(const std::string_view left, const std::string_view right)
    {
        return std::ranges::equal(left, right, [](const char a, const char b)
        {
            return std::tolower(static_cast<unsigned char>(a)) == std::tolower(static_cast<unsigned char>(b));
        });
    }
}

std::optional<std::string> UrlOpener::normalize(std::string_view link)
{
    while (!link.empty() && isBlank(link.front()))
        link.remove_prefix(1);
    while (!link.empty() && isBlank(link.back()))
        link.remove_suffix(1);

    if (link.empty() || std::ranges::any_of(link, [](const char c) { return isBlank(c) || isControl(c); }))
        return std::nullopt;

    const auto separator = link.find(SCHEME_SEPARATOR);
    if (separator == std::string_view::npos)
        return std::string(DEFAULT_SCHEME_PREFIX) + std::string(link);

    if (const auto scheme = link.substr(0, separator); !equalsIgnoreCase(scheme, HTTP_SCHEME) && !equalsIgnoreCase(scheme, HTTPS_SCHEME))
        return std::nullopt;

    return std::string(link);
}

bool UrlOpener::open(const std::string& link)
{
    const auto url = normalize(link);
    if (!url.has_value())
    {
        stapik::log::warning("Not opening the entry link, it is not a valid http(s) address: {}", link);
        return false;
    }

    try
    {
        return Gio::AppInfo::launch_default_for_uri(*url);
    }
    catch (const Glib::Error& error)
    {
        stapik::log::warning("Cannot open {}: {}", *url, error.what());
        return false;
    }
}

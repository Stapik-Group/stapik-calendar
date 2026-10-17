#include "UrlOpener.hpp"

#include "stapik/log/Log.hpp"

#include <giomm/appinfo.h>
#include <glibmm/error.h>

#include <algorithm>
#include <cctype>

#ifdef _WIN32
#include <windows.h>
#include <shellapi.h>
#endif

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

#ifdef _WIN32
    // GIO on Windows often has no registered handler for http(s), so ask the shell, which knows the default browser.
    bool openWithShell(const std::string& url)
    {
        const int length = MultiByteToWideChar(CP_UTF8, 0, url.c_str(), -1, nullptr, 0);
        if (length <= 0)
            return false;

        std::wstring wideUrl(static_cast<size_t>(length), L'\0');
        MultiByteToWideChar(CP_UTF8, 0, url.c_str(), -1, wideUrl.data(), length);

        // ShellExecuteW returns a value greater than 32 on success.
        const auto result = reinterpret_cast<INT_PTR>(
            ShellExecuteW(nullptr, L"open", wideUrl.c_str(), nullptr, nullptr, SW_SHOWNORMAL));
        return result > 32;
    }
#endif

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

#ifdef _WIN32
    if (openWithShell(*url))
        return true;

    stapik::log::warning("The shell could not open {}, trying GIO", *url);
#endif

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

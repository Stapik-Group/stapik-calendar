#include "UrlOpener.hpp"

#include "stapik/log/Log.hpp"

#include <giomm/appinfo.h>
#include <glibmm/error.h>

#include <algorithm>
#include <cctype>
#include <format>
#include <optional>

#ifdef _WIN32
#include <windows.h>
#include <shellapi.h>
#endif

namespace
{
    constexpr std::string_view AUTHORITY_MARKER = "//";
    constexpr std::string_view PORT_TERMINATORS = "/?#";
    constexpr std::string_view DIGITS = "0123456789";
    constexpr std::size_t MAX_PORT_DIGITS = 5;
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

    bool isSchemeCharacter(const char character)
    {
        return std::isalnum(static_cast<unsigned char>(character)) != 0 || character == '+' || character == '-' || character == '.';
    }

    std::optional<std::size_t> schemeLength(const std::string_view link)
    {
        if (link.empty() || std::isalpha(static_cast<unsigned char>(link.front())) == 0)
            return std::nullopt;

        const auto end = std::ranges::find_if_not(link, isSchemeCharacter);
        if (end == link.end() || *end != ':')
            return std::nullopt;

        return static_cast<std::size_t>(end - link.begin());
    }

    bool startsWithPort(const std::string_view text)
    {
        const auto digits = text.substr(0, text.find_first_not_of(DIGITS));
        if (digits.empty() || digits.size() > MAX_PORT_DIGITS)
            return false;

        const auto following = text.substr(digits.size());
        return following.empty() || PORT_TERMINATORS.find(following.front()) != std::string_view::npos;
    }

    bool isWebScheme(const std::string_view scheme)
    {
        return equalsIgnoreCase(scheme, HTTP_SCHEME) || equalsIgnoreCase(scheme, HTTPS_SCHEME);
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

    const auto length = schemeLength(link);
    if (!length.has_value() || startsWithPort(link.substr(*length + 1)))
        return std::format("{}{}", DEFAULT_SCHEME_PREFIX, link);

    const auto afterScheme = link.substr(*length + 1);
    if (!isWebScheme(link.substr(0, *length)) || !afterScheme.starts_with(AUTHORITY_MARKER) || afterScheme.size() == AUTHORITY_MARKER.size())
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

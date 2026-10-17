#include "UrlTitleFetcher.hpp"

#include <glib.h>
#include <glibmm/main.h>
#include <glibmm/ustring.h>
#include <curl/curl.h>
#include <charconv>
#include <optional>
#include <regex>
#include <string_view>
#include <system_error>
#include <thread>

namespace
{
    constexpr std::size_t MAX_TITLE_LENGTH = 120;
    constexpr std::size_t MAX_ENTITY_LENGTH = 10;
    constexpr auto ELLIPSIS = "…";

    void appendCodePoint(std::string& text, const gunichar codePoint)
    {
        char buffer[6];
        text.append(buffer, static_cast<std::size_t>(g_unichar_to_utf8(codePoint, buffer)));
    }

    std::optional<gunichar> namedEntity(const std::string_view name)
    {
        if (name == "amp")
            return U'&';
        if (name == "lt")
            return U'<';
        if (name == "gt")
            return U'>';
        if (name == "quot")
            return U'"';
        if (name == "apos")
            return U'\'';
        if (name == "nbsp")
            return gunichar{0x00A0};

        return std::nullopt;
    }

    std::optional<gunichar> numericEntity(std::string_view digits)
    {
        int base = 10;
        if (!digits.empty() && (digits.front() == 'x' || digits.front() == 'X'))
        {
            base = 16;
            digits.remove_prefix(1);
        }

        unsigned int value = 0;
        const auto* last = digits.data() + digits.size();
        const auto [end, error] = std::from_chars(digits.data(), last, value, base);
        if (digits.empty() || error != std::errc{} || end != last)
            return std::nullopt;

        return static_cast<gunichar>(value);
    }

    // Decodes the entities that occur in page titles. Everything it does not know is left as it is.
    std::string decodeEntities(const std::string& text)
    {
        std::string result;
        result.reserve(text.size());

        for (std::size_t i = 0; i < text.size(); ++i)
        {
            if (text[i] == '&')
            {
                if (const auto semicolon = text.find(';', i + 1); semicolon != std::string::npos && semicolon - i <= MAX_ENTITY_LENGTH)
                {
                    const std::string_view body(text.data() + i + 1, semicolon - i - 1);
                    const auto decoded = body.starts_with('#') ? numericEntity(body.substr(1)) : namedEntity(body);
                    if (decoded.has_value() && g_unichar_validate(*decoded))
                    {
                        appendCodePoint(result, *decoded);
                        i = semicolon;
                        continue;
                    }
                }
            }

            result.push_back(text[i]);
        }

        return result;
    }

    // Titles often span several lines and carry indentation, which would show up inside the entry.
    std::string collapseWhitespace(const std::string& text)
    {
        std::string result;
        bool pendingSpace = false;

        for (const gunichar character : Glib::ustring(text))
        {
            if (g_unichar_isspace(character) || g_unichar_iscntrl(character))
            {
                pendingSpace = !result.empty();
                continue;
            }

            if (pendingSpace)
                result.push_back(' ');

            pendingSpace = false;
            appendCodePoint(result, character);
        }

        return result;
    }

    // The page decides the encoding and the content of its title, so nothing of it can be trusted: a title that is not
    // valid UTF-8 would reach the labels and the saved calendar as it is, so it is dropped (the entry gets the default name).
    std::string cleanTitle(const std::string& raw)
    {
        if (!Glib::ustring(raw).validate())
            return {};

        const Glib::ustring title(collapseWhitespace(decodeEntities(raw)));
        if (title.size() <= MAX_TITLE_LENGTH)
            return title.raw();

        return title.substr(0, MAX_TITLE_LENGTH).raw() + ELLIPSIS;
    }
}

void UrlTitleFetcher::fetchAsync(const std::string& url, const Callback& onComplete)
{
    std::jthread([url, onComplete]
    {
        const auto title = fetchTitle(url);
        Glib::signal_idle().connect_once([onComplete, title]
        {
            onComplete(title);
        });
    }).detach();
}

std::string UrlTitleFetcher::fetchTitle(const std::string& url)
{
    CURL* curl = curl_easy_init();
    if (curl == nullptr)
        return {};

    std::string html;

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &html);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, kTimeoutSeconds);
    curl_easy_setopt(curl, CURLOPT_MAXFILESIZE, kMaxHtmlSizeBytes);
    curl_easy_setopt(curl, CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1_2);

    curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    return parseTitle(html);
}

std::string UrlTitleFetcher::parseTitle(const std::string &html)
{
    static const std::regex titleRegex(R"(<title[^>]*>([\s\S]*?)<\/title>)", std::regex::icase);
    std::smatch match;
    if (!std::regex_search(html, match, titleRegex))
        return {};
    return cleanTitle(match[1].str());
}

size_t UrlTitleFetcher::writeCallback(const char* ptr, const size_t size, const size_t nmemb, std::string* html)
{
    const size_t totalSize = size * nmemb;

    if (constexpr auto maxSize = static_cast<size_t>(kMaxHtmlSizeBytes); html->size() + totalSize > maxSize)
    {
        const size_t remaining = maxSize - html->size();
        html->append(ptr, remaining);
        return 0;
    }

    html->append(ptr, totalSize);
    return totalSize;
}
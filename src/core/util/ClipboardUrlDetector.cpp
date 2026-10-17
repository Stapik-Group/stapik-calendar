#include "ClipboardUrlDetector.hpp"
#include <regex>
#include <string>

namespace
{
    // Text copied from a page or an address bar often carries a trailing space or line break (CRLF on Windows).
    std::string trimmed(const std::string& text)
    {
        constexpr auto whitespace = " \t\r\n";

        const auto first = text.find_first_not_of(whitespace);
        if (first == std::string::npos)
            return {};

        return text.substr(first, text.find_last_not_of(whitespace) - first + 1);
    }
}

void ClipboardUrlDetector::detectAsync(const Glib::RefPtr<Gdk::Clipboard>& clipboard,
                                       const Callback& onUrlFound,
                                       const Callback& onNoUrl)
{
    clipboard->read_text_async([clipboard, onUrlFound, onNoUrl] (const Glib::RefPtr<Gio::AsyncResult>& result)
    {
        try
        {
            if (const auto url = trimmed(clipboard->read_text_finish(result)); isValidUrl(url))
                onUrlFound(url);
            else
                onNoUrl({});
        }
        catch (const Glib::Error&)
        {
            g_warning("No valid url in clipboard to paste.");
            onNoUrl({});
        }
    });
}

bool ClipboardUrlDetector::isValidUrl(const std::string &text)
{
    static const std::regex urlRegex(R"(^https?://[^\s/$.?#].[^\s]*$)", std::regex_constants::icase);
    return std::regex_match(text, urlRegex);
}
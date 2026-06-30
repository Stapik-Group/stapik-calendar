#include "ClipboardUrlDetector.hpp"
#include <regex>

void ClipboardUrlDetector::detectAsync(const Glib::RefPtr<Gdk::Clipboard>& clipboard,
                                       const Callback& onUrlFound,
                                       const Callback& onNoUrl)
{
    clipboard->read_text_async([clipboard, onUrlFound, onNoUrl] (const Glib::RefPtr<Gio::AsyncResult>& result)
    {
        try
        {
            const auto text = clipboard->read_text_finish(result);
            if (!text.empty() && isValidUrl(text))
                onUrlFound(text);
            else
                onNoUrl({});
        }
        catch (const Glib::Error&)
        {
            onNoUrl({});
        }
    });
}

bool ClipboardUrlDetector::isValidUrl(const std::string &text)
{
    const std::regex urlRegex(R"(^https?://[^\s/$.?#].[^\s]*$)", std::regex_constants::icase);
    return std::regex_match(text, urlRegex);
}

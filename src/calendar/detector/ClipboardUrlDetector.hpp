#pragma once

#include <functional>
#include <string>
#include <gdkmm/clipboard.h>

class ClipboardUrlDetector
{
public:
    using Callback = std::function<void(const std::string& url)>;
    static void detectAsync(const Glib::RefPtr<Gdk::Clipboard>& clipboard, const Callback& onUrlFound, const Callback& onNoUrl);
private:
    static bool isValidUrl(const std::string& text);
};

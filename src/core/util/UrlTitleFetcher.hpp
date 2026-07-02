#pragma once

#include <string>
#include <functional>

class UrlTitleFetcher
{
public:
    using Callback = std::function<void(const std::string& title)>;
    static void fetchAsync(const std::string& url, const Callback& onComplete);
private:
    static constexpr long kTimeoutSeconds = 10L;
    static constexpr long kMaxHtmlSizeBytes = 512000L;

    static std::string fetchTitle(const std::string& url);
    static std::string parseTitle(const std::string& html);
    static size_t writeCallback(const char* ptr, size_t size, size_t nmemb, std::string* html);
};
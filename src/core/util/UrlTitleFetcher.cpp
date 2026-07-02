#include "UrlTitleFetcher.hpp"

#include <curl/curl.h>
#include <glibmm/main.h>
#include <regex>
#include <thread>

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
    return match[1].str();
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
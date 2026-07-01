#include "CloudStorageClient.hpp"

#include <curl/curl.h>
#include <stdexcept>

class CloudStorageException : public std::runtime_error
{
public:
    using std::runtime_error::runtime_error;
};

CloudStorageClient::CloudStorageClient(CloudStorageConfig config): m_config(std::move(config))
{}

nlohmann::json CloudStorageClient::loadJson() const
{
    const auto raw = fetchRawContent();
    if (raw.empty())
        return {};

    try
    {
        const auto outer = nlohmann::json::parse(raw);
        const auto content = outer.at("content").get<std::string>();
        if (content.empty())
            return {};

        return nlohmann::json::parse(content);
    }
    catch (const nlohmann::json::exception&)
    {
        return {};
    }
}

void CloudStorageClient::saveJson(const nlohmann::json& data) const
{
    const auto contentString = data.dump(2);

    const nlohmann::json payload = {
        { "filename", FILENAME },
        { "content",  contentString }
    };

    pushRawContent(payload.dump());
}

std::string CloudStorageClient::fetchRawContent() const
{
    CURL* curl = curl_easy_init();
    if (curl == nullptr)
        return {};

    const auto url = m_config.apiUrl + "/read?filename=" + FILENAME;
    std::string response;

    const auto authHeader = std::string("x-api-key: ") + m_config.apiKey;
    curl_slist* headers = curl_slist_append(nullptr, authHeader.c_str());
    headers = curl_slist_append(headers, "Content-Type: application/json");

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, TIMEOUT_SECONDS);
    curl_easy_setopt(curl, CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1_2);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    curl_easy_perform(curl);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    return response;
}

void CloudStorageClient::pushRawContent(const std::string& content) const
{
    CURL* curl = curl_easy_init();
    if (curl == nullptr)
        throw CloudStorageException("Cannot initialize CURL!");

    const auto url = m_config.apiUrl + "/write";

    const auto authHeader = std::string("x-api-key: ") + m_config.apiKey;
    curl_slist* headers = curl_slist_append(nullptr, authHeader.c_str());
    headers = curl_slist_append(headers, "Content-Type: application/json");

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, content.c_str());
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, TIMEOUT_SECONDS);
    curl_easy_setopt(curl, CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1_2);

    const auto result = curl_easy_perform(curl);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (result != CURLE_OK)
        throw CloudStorageException(std::string("API save failed: ") + curl_easy_strerror(result));
}

size_t CloudStorageClient::writeCallback(const char* ptr, const size_t size, const size_t nmemb, std::string* response)
{
    const size_t totalSize = size * nmemb;
    response->append(ptr, totalSize);
    return totalSize;
}
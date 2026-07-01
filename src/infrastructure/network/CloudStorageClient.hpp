#pragma once

#include "CloudStorageConfig.hpp"
#include <nlohmann/json.hpp>
#include <string>

class CloudStorageClient
{
public:
    explicit CloudStorageClient(CloudStorageConfig config);

    nlohmann::json loadJson() const;
    void saveJson(const nlohmann::json& data) const;
private:
    static constexpr auto FILENAME = "calendar.json";
    static constexpr long TIMEOUT_SECONDS = 8L;

    CloudStorageConfig m_config;

    std::string fetchRawContent() const;
    void pushRawContent(const std::string& content) const;

    static size_t writeCallback(const char* ptr, size_t size, size_t nmemb, std::string* response);
};
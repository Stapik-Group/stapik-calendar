#pragma once

#include "CloudStorageConfig.hpp"
#include <filesystem>
#include <optional>
#include <stdexcept>

class CloudStorageConfigException : public std::runtime_error
{
public:
    using std::runtime_error::runtime_error;
};

class CloudStorageConfigStorage
{
public:
    static void save(const CloudStorageConfig& config);
    static std::optional<CloudStorageConfig> load();
private:
    static std::filesystem::path configPath();
};
#pragma once

#include <chrono>
#include <filesystem>
#include <map>
#include <nlohmann/json.hpp>

#include "../../core/model/CalendarEntry.hpp"

class CalendarStorageException : public std::runtime_error
{
public:
    using std::runtime_error::runtime_error;
};

class CalendarStorage
{
public:
    using Entries = std::map<std::chrono::year_month_day, std::vector<CalendarEntry>>;
    static void save(const Entries& entries);
    static Entries load();
    static nlohmann::json toJson(const Entries& entries);
    static Entries fromJson(const nlohmann::json& json);
private:
    static std::filesystem::path storagePath();
    static std::string serializeDate(std::chrono::year_month_day date);
    static std::chrono::year_month_day deserializeDate(const std::string& str);
};

#pragma once

#include "../../core/model/CalendarEntries.hpp"

#include <chrono>
#include <filesystem>
#include <nlohmann/json.hpp>
#include <stdexcept>

class CalendarStorageException : public std::runtime_error
{
public:
    using std::runtime_error::runtime_error;
};

struct CalendarSnapshot
{
    CalendarEntries entries;
    std::chrono::system_clock::time_point lastUpdate;
};

class CalendarStorage
{
public:
    static void save(const CalendarSnapshot& snapshot);
    [[nodiscard]] static CalendarSnapshot load();
    [[nodiscard]] static nlohmann::json toJson(const CalendarSnapshot& snapshot);
    [[nodiscard]] static CalendarSnapshot fromJson(const nlohmann::json& json);
private:
    static std::filesystem::path storagePath();
    static std::string serializeDate(std::chrono::year_month_day date);
    static std::chrono::year_month_day deserializeDate(const std::string& str);
    static nlohmann::json entriesToJson(const CalendarEntries& entries);
    static CalendarEntries entriesFromJson(const nlohmann::json& json);
};
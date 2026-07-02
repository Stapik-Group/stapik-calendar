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

class CalendarStorage
{
public:
    static void save(const CalendarEntries& entries);
    [[nodiscard]] static CalendarEntries load();
    [[nodiscard]] static nlohmann::json toJson(const CalendarEntries& entries);
    [[nodiscard]] static CalendarEntries fromJson(const nlohmann::json& json);
private:
    static std::filesystem::path storagePath();
    static std::string serializeDate(std::chrono::year_month_day date);
    static std::chrono::year_month_day deserializeDate(const std::string& str);
};
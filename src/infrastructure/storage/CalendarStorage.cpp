#include "CalendarStorage.hpp"

#include "stapik/sync/SyncEnvelope.hpp"

#include <fstream>
#include <nlohmann/json.hpp>

void CalendarStorage::save(const CalendarSnapshot& snapshot)
{
    const auto path = storagePath();
    std::filesystem::create_directories(path.parent_path());

    std::ofstream file(path);
    file << toJson(snapshot).dump(2);
}

CalendarSnapshot CalendarStorage::load()
{
    const auto path = storagePath();
    if (!std::filesystem::exists(path))
        return {};

    std::ifstream file(path);
    if (!file.is_open())
        return {};

    try
    {
        return fromJson(nlohmann::json::parse(file));
    }
    catch (const nlohmann::json::exception &)
    {
        return {};
    }
}

std::filesystem::path CalendarStorage::storagePath()
{
    const auto* home = std::getenv("HOME");
    if (home == nullptr)
        throw CalendarStorageException("No HOME in PATH");

    return std::filesystem::path(home) / ".local" / "share" / "stapikcalendar" / "calendar.json";
}

std::string CalendarStorage::serializeDate(const std::chrono::year_month_day date)
{
    return std::format("{:04}-{:02}-{:02}",
                       static_cast<int>(date.year()),
                       static_cast<unsigned>(date.month()),
                       static_cast<unsigned>(date.day()));
}

std::chrono::year_month_day CalendarStorage::deserializeDate(const std::string &str)
{
    const int year = std::stoi(str.substr(0, 4));
    const auto month = static_cast<unsigned>(std::stoi(str.substr(5, 2)));
    const auto day = static_cast<unsigned>(std::stoi(str.substr(8, 2)));

    return std::chrono::year_month_day{
        std::chrono::year{year},
        std::chrono::month{month},
        std::chrono::day{day}
    };
}

nlohmann::json CalendarStorage::entriesToJson(const CalendarEntries& entries)
{
    nlohmann::json json = nlohmann::json::array();

    for (const auto& [date, dayEntries] : entries)
        for (const auto& [name, link] : dayEntries)
            json.push_back({
                { "date", serializeDate(date) },
                { "name", name },
                { "link", link }
            });

    return json;
}

CalendarEntries CalendarStorage::entriesFromJson(const nlohmann::json& json)
{
    CalendarEntries entries;

    for (const auto& item : json)
    {
        const auto date = deserializeDate(item.at("date").get<std::string>());
        entries[date].emplace_back(
            item.at("name").get<std::string>(),
            item.at("link").get<std::string>()
        );
    }

    return entries;
}

nlohmann::json CalendarStorage::toJson(const CalendarSnapshot& snapshot)
{
    const stapik::sync::SyncEnvelope envelope{ snapshot.lastUpdate, entriesToJson(snapshot.entries) };
    return envelope.toJson();
}

CalendarSnapshot CalendarStorage::fromJson(const nlohmann::json& json)
{
    try
    {
        // Legacy pre-sync files: bare array, no envelope/timestamp.
        if (json.is_array())
            return CalendarSnapshot{ entriesFromJson(json), std::chrono::system_clock::time_point{} };

        const auto envelope = stapik::sync::SyncEnvelope::fromJson(json);
        return CalendarSnapshot{ entriesFromJson(envelope.payload), envelope.lastUpdate };
    }
    catch (const nlohmann::json::exception&)
    {
        return {};
    }
}
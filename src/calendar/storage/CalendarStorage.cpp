#include "CalendarStorage.hpp"

#include <fstream>
#include <nlohmann/json.hpp>
#include <stdexcept>

void CalendarStorage::save(const Entries &entries)
{
    const auto path = storagePath();
    std::filesystem::create_directories(path.parent_path());

    nlohmann::json json = nlohmann::json::array();
    for (const auto &[date, dayEntries]: entries)
    {
        for (const auto &[name, link]: dayEntries)
        {
            json.push_back({
                {"date", serializeDate(date)},
                {"name", name},
                {"link", link}
            });
        }
    }

    std::ofstream file(path);
    file << json.dump(2);
}

CalendarStorage::Entries CalendarStorage::load()
{
    const auto path = storagePath();
    if (!std::filesystem::exists(path))
        return {};

    std::ifstream file(path);
    if (!file.is_open())
        return {};

    Entries entries;
    try
    {
        for (const auto json = nlohmann::json::parse(file); const auto &item: json)
        {
            const auto date = deserializeDate(item.at("date").get<std::string>());
            entries[date].emplace_back(item.at("name").get<std::string>(), item.at("link").get<std::string>());
        }
    } catch (const nlohmann::json::exception &)
    {
        return {};
    }

    return entries;
}

std::filesystem::path CalendarStorage::storagePath()
{
    const auto* home = std::getenv("HOME");
    if (home == nullptr)
        throw CalendarStorageException("Brak zmiennej HOME");

    return std::filesystem::path(home) / ".local" / "share" / "stapikcalendar" / "calendar.json";
}
std::string CalendarStorage::serializeDate(std::chrono::year_month_day date)
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

nlohmann::json CalendarStorage::toJson(const Entries& entries)
{
    nlohmann::json json = nlohmann::json::array();

    for (const auto& [date, dayEntries] : entries)
        for (const auto& [name, link] : dayEntries)
            json.push_back({
                { "date", serializeDate(date) },
                { "name", name                },
                { "link", link                }
            });

    return json;
}

CalendarStorage::Entries CalendarStorage::fromJson(const nlohmann::json& json)
{
    Entries entries;

    try
    {
        for (const auto& item : json)
        {
            const auto date = deserializeDate(item.at("date").get<std::string>());
            entries[date].emplace_back(
                item.at("name").get<std::string>(),
                item.at("link").get<std::string>()
            );
        }
    }
    catch (const nlohmann::json::exception&)
    {
        return {};
    }

    return entries;
}
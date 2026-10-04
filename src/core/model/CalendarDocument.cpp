#include "CalendarDocument.hpp"

#include "stapik/domain/CategoryColor.hpp"
#include "stapik/sync/Timestamp.hpp"

#include <charconv>
#include <utility>
#include <format>
#include <stdexcept>
#include <string>

namespace
{
    constexpr std::size_t DATE_LENGTH = 10;
    constexpr auto DEFAULT_COLOR_ID = "default";

    template<typename Number>
    bool parseNumber(const std::string& text, const std::size_t offset, const std::size_t length, Number& value)
    {
        const char* first = text.data() + offset;
        const char* last = first + length;
        const auto [end, errorCode] = std::from_chars(first, last, value);
        return errorCode == std::errc{} && end == last;
    }

    std::string serializeDate(const std::chrono::year_month_day date)
    {
        return std::format("{:04}-{:02}-{:02}",
                           static_cast<int>(date.year()),
                           static_cast<unsigned>(date.month()),
                           static_cast<unsigned>(date.day()));
    }

    std::chrono::year_month_day deserializeDate(const std::string& text)
    {
        int year = 0;
        unsigned month = 0;
        unsigned day = 0;

        if (text.size() != DATE_LENGTH || text[4] != '-' || text[7] != '-' ||
            !parseNumber(text, 0, 4, year) || !parseNumber(text, 5, 2, month) || !parseNumber(text, 8, 2, day))
        {
            throw std::invalid_argument("Invalid date: " + text);
        }

        const std::chrono::year_month_day date{ std::chrono::year{year}, std::chrono::month{month}, std::chrono::day{day} };
        if (!date.ok())
            throw std::invalid_argument("Invalid date: " + text);

        return date;
    }

    CalendarDocument::TimePoint parseTimestamp(const std::string& text)
    {
        const auto parsed = stapik::sync::parseIso8601(text);
        if (!parsed.has_value())
            throw std::invalid_argument("Invalid timestamp: " + text);

        return *parsed;
    }

    std::string serializeColor(const EntryColor color)
    {
        return color.has_value() ? std::string(stapik::domain::categoryColorId(*color)) : DEFAULT_COLOR_ID;
    }

    nlohmann::json entriesToJson(const CalendarEntries& entries)
    {
        auto json = nlohmann::json::array();

        for (const auto& [date, dayEntries] : entries)
        {
            for (const auto& [name, link, color] : dayEntries)
            {
                json.push_back({
                    { "date", serializeDate(date) },
                    { "name", name },
                    { "link", link },
                    { "color", serializeColor(color) }
                });
            }
        }

        return json;
    }

    CalendarEntries entriesFromJson(const nlohmann::json& json)
    {
        CalendarEntries entries;

        for (const auto& item : json)
        {
            entries[deserializeDate(item.at("date").get<std::string>())].push_back(CalendarEntry{
                item.at("name").get<std::string>(),
                item.at("link").get<std::string>(),
                stapik::domain::categoryColorFromId(item.value("color", std::string{DEFAULT_COLOR_ID}))
            });
        }

        return entries;
    }
}

CalendarDocument::CalendarDocument(CalendarEntries entries, const TimePoint lastUpdate, const std::optional<TimePoint> lastKnownCloudUpdate) :
    m_entries(std::move(entries)),
    m_lastUpdate(lastUpdate),
    m_lastKnownCloudUpdate(lastKnownCloudUpdate) {}

CalendarEntries& CalendarDocument::entries()
{
    return m_entries;
}

const CalendarEntries& CalendarDocument::entries() const
{
    return m_entries;
}

CalendarDocument::TimePoint CalendarDocument::lastUpdate() const
{
    return m_lastUpdate;
}

std::optional<CalendarDocument::TimePoint> CalendarDocument::lastKnownCloudUpdate() const
{
    return m_lastKnownCloudUpdate;
}

void CalendarDocument::markUpdated(const TimePoint when)
{
    m_lastUpdate = when;
}

void CalendarDocument::setLastKnownCloudUpdate(const TimePoint cloudUpdatedAt)
{
    m_lastKnownCloudUpdate = cloudUpdatedAt;
}

CalendarDocument CalendarDocument::withLastKnownCloudUpdate(const TimePoint cloudUpdatedAt) const
{
    CalendarDocument copy = *this;
    copy.setLastKnownCloudUpdate(cloudUpdatedAt);
    return copy;
}

nlohmann::json CalendarDocument::toJson() const
{
    return nlohmann::json{
        { "lastUpdate", stapik::sync::toIso8601(m_lastUpdate) },
        { "entries", entriesToJson(m_entries) }
    };
}

CalendarDocument CalendarDocument::fromJson(const nlohmann::json& json)
{
    // Legacy bare array (pre-sync file or old cloud payload): the timestamp is not part of it.
    if (json.is_array())
        return CalendarDocument(entriesFromJson(json), TimePoint{});

    return CalendarDocument(
        entriesFromJson(json.at("entries")),
        parseTimestamp(json.at("lastUpdate").get<std::string>()));
}

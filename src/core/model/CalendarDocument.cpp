#include "CalendarDocument.hpp"

#include "stapik/domain/CategoryColor.hpp"
#include "stapik/domain/IdGenerator.hpp"
#include "stapik/log/Log.hpp"
#include "stapik/sync/Timestamp.hpp"

#include <algorithm>
#include <array>
#include <charconv>
#include <utility>
#include <format>
#include <set>
#include <stdexcept>
#include <string>
#include <string_view>

namespace
{
    constexpr std::size_t DATE_LENGTH = 10;
    constexpr auto DEFAULT_COLOR_ID = "default";
    constexpr std::array<std::string_view, 7> KNOWN_ENTRY_FIELDS = { "date", "name", "link", "color", "location", "time", "id" };

    struct ParsedEntries
    {
        CalendarEntries entries;
        std::size_t skipped = 0;
    };

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

    EntryId claimUniqueId(EntryId candidate, std::set<EntryId>& usedIds)
    {
        if (candidate.empty() || usedIds.contains(candidate))
            candidate = stapik::domain::generateId();

        usedIds.insert(candidate);
        return candidate;
    }

    nlohmann::json entriesToJson(const CalendarEntries& entries)
    {
        auto json = nlohmann::json::array();

        for (const auto& [date, dayEntries] : entries)
        {
            for (const auto& entry : dayEntries)
            {
                nlohmann::json item{
                    { "date", serializeDate(date) },
                    { "name", entry.name },
                    { "link", entry.link },
                    { "color", serializeColor(entry.color) },
                    { "location", entry.location },
                    { "time", entry.time },
                    { "id", entry.id }
                };

                if (!entry.unknownFields.empty())
                    item.update(entry.unknownFields);

                json.push_back(std::move(item));
            }
        }

        return json;
    }

    nlohmann::json unknownFieldsOf(const nlohmann::json& item)
    {
        auto unknown = nlohmann::json::object();

        for (const auto& [key, value] : item.items())
        {
            if (std::ranges::find(KNOWN_ENTRY_FIELDS, std::string_view(key)) == KNOWN_ENTRY_FIELDS.end())
                unknown[key] = value;
        }

        return unknown;
    }

    CalendarEntry entryFromJson(const nlohmann::json& item, std::set<EntryId>& usedIds)
    {
        return CalendarEntry{
            item.at("name").get<std::string>(),
            item.at("link").get<std::string>(),
            stapik::domain::categoryColorFromId(item.value("color", std::string{DEFAULT_COLOR_ID})),
            item.value("location", std::string{}),
            item.value("time", std::string{}),
            claimUniqueId(item.value("id", EntryId{}), usedIds),
            unknownFieldsOf(item)
        };
    }

    ParsedEntries entriesFromJson(const nlohmann::json& json)
    {
        if (!json.is_array())
            throw std::invalid_argument("The entries are not a list");

        ParsedEntries parsed;
        std::set<EntryId> usedIds;

        for (const auto& item : json)
        {
            try
            {
                const auto date = deserializeDate(item.at("date").get<std::string>());
                auto entry = entryFromJson(item, usedIds);
                parsed.entries[date].push_back(std::move(entry));
            }
            catch (const std::exception& error)
            {
                ++parsed.skipped;
                stapik::log::warning("A calendar entry cannot be read and is left out: {}", error.what());
            }
        }

        return parsed;
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

std::size_t CalendarDocument::skippedEntryCount() const
{
    return m_skippedEntryCount;
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
    const bool isLegacyArray = json.is_array();

    auto parsed = entriesFromJson(isLegacyArray ? json : json.at("entries"));
    const auto lastUpdate = isLegacyArray ? TimePoint{} : parseTimestamp(json.at("lastUpdate").get<std::string>());

    CalendarDocument document(std::move(parsed.entries), lastUpdate);
    document.m_skippedEntryCount = parsed.skipped;
    return document;
}

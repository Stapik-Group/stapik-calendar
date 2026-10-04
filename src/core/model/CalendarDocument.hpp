#pragma once

#include "CalendarEntries.hpp"

#include <nlohmann/json.hpp>

#include <chrono>
#include <optional>

// The whole calendar as one syncable and persistable unit.
// lastKnownCloudUpdate is local sync state: it is not part of toJson().
class CalendarDocument
{
public:
    using TimePoint = std::chrono::system_clock::time_point;

    CalendarDocument() = default;
    CalendarDocument(CalendarEntries entries, TimePoint lastUpdate, std::optional<TimePoint> lastKnownCloudUpdate = std::nullopt);

    [[nodiscard]] CalendarEntries& entries();
    [[nodiscard]] const CalendarEntries& entries() const;
    [[nodiscard]] TimePoint lastUpdate() const;
    [[nodiscard]] std::optional<TimePoint> lastKnownCloudUpdate() const;

    void markUpdated(TimePoint when = std::chrono::system_clock::now());
    [[nodiscard]] CalendarDocument withLastKnownCloudUpdate(TimePoint cloudUpdatedAt) const;

    [[nodiscard]] nlohmann::json toJson() const;
    [[nodiscard]] static CalendarDocument fromJson(const nlohmann::json& json);

private:
    CalendarEntries m_entries;
    TimePoint m_lastUpdate{};
    std::optional<TimePoint> m_lastKnownCloudUpdate;
};

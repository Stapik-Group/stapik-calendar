#pragma once

#include "../../core/model/CalendarDocument.hpp"

#include "stapik/cloud/CloudStorageClient.hpp"

#include <nlohmann/json.hpp>
#include <optional>

class CalendarSyncCoordinator
{
public:
    // Whole-document last-write-wins. No per-entry merge — single-user tool.
    [[nodiscard]] static CalendarDocument resolveOnConnect(const CalendarDocument& local, CloudStorageClient& cloudClient);
    [[nodiscard]] static CalendarDocument pushLocalChange(const CalendarDocument& local, CloudStorageClient& cloudClient);
private:
    [[nodiscard]] static CalendarDocument pushWithConflictResolution(
        const CalendarDocument& local,
        CloudStorageClient& cloudClient,
        std::optional<std::chrono::system_clock::time_point> baseline);

    [[nodiscard]] static std::optional<CalendarDocument> fromCloudDocument(const CloudDocument& document);
    [[nodiscard]] static nlohmann::json toCloudContent(const CalendarDocument& document);
};

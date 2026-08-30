#pragma once

#include "CalendarStorage.hpp"
#include "stapik/cloud/CloudStorageClient.hpp"

#include <optional>

class CalendarSyncCoordinator
{
public:
    // Whole-document last-write-wins. No per-entry merge — single-user tool.
    [[nodiscard]] static CalendarSnapshot resolveOnConnect(const CalendarSnapshot& local, CloudStorageClient& cloudClient);
    [[nodiscard]] static CalendarSnapshot pushLocalChange(const CalendarSnapshot& local, CloudStorageClient& cloudClient);
private:
    [[nodiscard]] static CalendarSnapshot pushWithConflictResolution(
    const CalendarSnapshot& local,
    CloudStorageClient& cloudClient,
    std::optional<std::chrono::system_clock::time_point> baseline);

    [[nodiscard]] static CalendarSnapshot fromCloudDocument(const CloudDocument& document);
};
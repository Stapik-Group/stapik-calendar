#pragma once

#include "CalendarStorage.hpp"
#include "stapik/cloud/CloudStorageClient.hpp"

class CalendarSyncCoordinator
{
public:
    // Whole-document last-write-wins. No per-entry merge — single-user tool,
    [[nodiscard]] static CalendarSnapshot resolveOnConnect(const CalendarSnapshot& local, CloudStorageClient& cloudClient);
};
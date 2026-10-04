#include "CalendarSyncCoordinator.hpp"

#include "stapik/cloud/CloudStorageException.hpp"
#include "stapik/sync/SyncEnvelope.hpp"

#include <glib.h>

namespace
{
    bool isEmptyContent(const nlohmann::json& content)
    {
        return content.is_null() || ((content.is_object() || content.is_array()) && content.empty());
    }
}

nlohmann::json CalendarSyncCoordinator::toCloudContent(const CalendarDocument& document)
{
    return stapik::sync::SyncEnvelope{ document.lastUpdate(), document.toJson() }.toJson();
}

std::optional<CalendarDocument> CalendarSyncCoordinator::fromCloudDocument(const CloudDocument& document)
{
    try
    {
        const auto envelope = stapik::sync::SyncEnvelope::fromJson(document.content);
        auto parsed = CalendarDocument::fromJson(envelope.payload);

        // An old cloud payload is a bare array: its timestamp lives only in the envelope.
        if (envelope.payload.is_array())
            parsed = CalendarDocument(std::move(parsed.entries()), envelope.lastUpdate);

        return parsed.withLastKnownCloudUpdate(document.updatedAt);
    }
    catch (const std::exception& exception)
    {
        g_warning("[Cloud] Cannot read the cloud document, keeping local data: %s", exception.what());
        return std::nullopt;
    }
}

CalendarDocument CalendarSyncCoordinator::pushWithConflictResolution(
    const CalendarDocument& local,
    CloudStorageClient& cloudClient,
    const std::optional<std::chrono::system_clock::time_point> baseline)
{
    CloudWriteResult result;

    try
    {
        result = cloudClient.saveDocument(toCloudContent(local), baseline.value_or(std::chrono::system_clock::time_point{}));
    }
    catch (const CloudStorageException&)
    {
        g_debug("Cannot sync with cloud, will retry on next save.");
        return local;
    }

    if (!result.conflict)
        return local.withLastKnownCloudUpdate(result.document.updatedAt);

    // Server has a newer document than we knew about.
    if (result.document.updatedAt > local.lastUpdate())
        return fromCloudDocument(result.document).value_or(local);

    // We're still newer (rare race) — one retry against the server's current baseline.
    try
    {
        const auto [document, conflict] = cloudClient.saveDocument(toCloudContent(local), result.document.updatedAt);

        if (!conflict)
            return local.withLastKnownCloudUpdate(document.updatedAt);

        // Lost the race twice — accept the server's version to avoid looping.
        return fromCloudDocument(document).value_or(local);
    }
    catch (const CloudStorageException&)
    {
        g_debug("Cannot sync with cloud, will retry on next save.");
        return local;
    }
}

CalendarDocument CalendarSyncCoordinator::resolveOnConnect(const CalendarDocument& local, CloudStorageClient& cloudClient)
{
    std::optional<CloudDocument> remote;

    try
    {
        remote = cloudClient.loadDocument();
    }
    catch (const CloudStorageException&)
    {
        g_debug("Cloud unreachable right now — keep working with local data.");
        return local;
    }

    if (!remote.has_value())
        return pushWithConflictResolution(local, cloudClient, std::nullopt);

    // A document that exists but has no content must never replace local data.
    if (isEmptyContent(remote->content))
        return pushWithConflictResolution(local, cloudClient, remote->updatedAt);

    if (remote->updatedAt > local.lastUpdate())
        return fromCloudDocument(remote.value()).value_or(local);

    return pushWithConflictResolution(local, cloudClient, remote->updatedAt);
}

CalendarDocument CalendarSyncCoordinator::pushLocalChange(const CalendarDocument& local, CloudStorageClient& cloudClient)
{
    return pushWithConflictResolution(local, cloudClient, local.lastKnownCloudUpdate());
}

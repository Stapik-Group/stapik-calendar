#include "CalendarSyncCoordinator.hpp"
#include <glib.h>
#include "stapik/cloud/CloudStorageException.hpp"

CalendarSnapshot CalendarSyncCoordinator::resolveOnConnect(const CalendarSnapshot& local, CloudStorageClient& cloudClient)
{
    CalendarSnapshot cloud;

    try
    {
        const auto json = cloudClient.loadJson();
        if (json.empty())
        {
            cloudClient.saveJson(CalendarStorage::toJson(local));
            return local;
        }
        cloud = CalendarStorage::fromJson(json);
    }
    catch (const CloudStorageException&)
    {
        g_debug("Cloud unreachable right now — keep working with local data.");
        return local;
    }

    if (cloud.lastUpdate > local.lastUpdate)
    {
        CalendarStorage::save(cloud);
        return cloud;
    }

    if (local.lastUpdate > cloud.lastUpdate)
    {
        try { cloudClient.saveJson(CalendarStorage::toJson(local)); }
        catch (const CloudStorageException&)
        {
            g_debug("Cannot sync with cloud, will retry on next save.");
        }
    }

    return local;
}
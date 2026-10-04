#include "CalendarCloudHooks.hpp"

#include "stapik/cloud/CloudStorageException.hpp"
#include "stapik/log/Log.hpp"

#include <memory>
#include <utility>

namespace
{
    constexpr auto CLOUD_SLOT = "calendar.json";
}

stapik::sync::CloudSessionHooks createCalendarCloudHooks()
{
    auto hooks = stapik::sync::defaultCloudSessionHooks(CLOUD_SLOT);

    hooks.createStorage = [create = std::move(hooks.createStorage)](const CloudStorageConfig& config) -> std::unique_ptr<ICloudStorage>
    {
        try
        {
            return create(config);
        }
        catch (const CloudStorageException& exception)
        {
            stapik::log::warning("Cannot create the cloud client: {}", exception.what());
            return nullptr;
        }
    };

    return hooks;
}

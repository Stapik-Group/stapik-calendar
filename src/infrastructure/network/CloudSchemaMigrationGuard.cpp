#include "CloudSchemaMigrationGuard.hpp"
#include "CloudStorageConfigStorage.hpp"

#include <fstream>
#include <glib.h>

void CloudSchemaMigrationGuard::ensureCompatible()
{
    if (readStoredVersion() >= CURRENT_SCHEMA_VERSION)
        return;

    g_message("[Cloud] Incompatible cloud settings detected from a previous version — clearing.");
    CloudStorageConfigStorage::clear();
    writeStoredVersion(CURRENT_SCHEMA_VERSION);
}

int CloudSchemaMigrationGuard::readStoredVersion()
{
    const auto path = versionFilePath();
    if (!std::filesystem::exists(path))
        return 0;

    std::ifstream file(path);
    if (!file.is_open())
        return 0;

    int version = 0;
    file >> version;

    return file.fail() ? 0 : version;
}

void CloudSchemaMigrationGuard::writeStoredVersion(const int version)
{
    const auto path = versionFilePath();
    std::filesystem::create_directories(path.parent_path());

    std::ofstream file(path);
    file << version;
}

std::filesystem::path CloudSchemaMigrationGuard::versionFilePath()
{
    const auto* home = std::getenv("HOME");
    if (home == nullptr)
        return {};

    return std::filesystem::path(home) / ".local" / "share" / "stapikcalendar" / "cloud_schema_version";
}
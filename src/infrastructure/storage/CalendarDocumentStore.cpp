#include "CalendarDocumentStore.hpp"

#include "stapik/app/AppContext.hpp"
#include "stapik/log/Log.hpp"
#include "stapik/storage/AppPaths.hpp"
#include "stapik/storage/AtomicFile.hpp"
#include "stapik/storage/PathText.hpp"
#include "stapik/sync/Timestamp.hpp"

#include <chrono>
#include <format>
#include <fstream>
#include <utility>

namespace
{
    using stapik::document::LoadStatus;
    using TimePoint = CalendarDocument::TimePoint;

    constexpr int SCHEMA_VERSION = 1;
    constexpr int BACKUP_COUNT = 3;
    constexpr auto DOCUMENT_FILE_NAME = "calendar.json";
    constexpr auto BASELINE_FILE_NAME = "cloud-baseline.json";
    constexpr auto BASELINE_KEY = "lastKnownCloudUpdate";

    // Version 0 is every file written before the schema was versioned:
    // either a bare entries array or a sync envelope { lastUpdate, payload, lastKnownCloudUpdate? }.
    void migrateFromUnversioned(nlohmann::json& document)
    {
        if (document.is_array())
        {
            nlohmann::json entries = std::move(document);
            document = nlohmann::json{
                { "lastUpdate", stapik::sync::toIso8601(TimePoint{}) },
                { "entries", std::move(entries) }
            };
            return;
        }

        document = nlohmann::json{
            { "lastUpdate", document.at("lastUpdate") },
            { "entries", document.at("payload") }
        };
    }

    stapik::document::DocumentFile<CalendarDocument> createFile(const std::filesystem::path& path)
    {
        stapik::document::SchemaMigrator migrator(SCHEMA_VERSION);
        migrator.addStep(0, migrateFromUnversioned);

        stapik::document::DocumentFile<CalendarDocument> file(path, std::move(migrator));
        file.setBackupCount(BACKUP_COUNT);
        return file;
    }
}

CalendarDocumentStore::CalendarDocumentStore(const std::filesystem::path& directory) :
    m_file(createFile(directory / DOCUMENT_FILE_NAME)),
    m_baselinePath(directory / BASELINE_FILE_NAME) {}

CalendarDocumentStore CalendarDocumentStore::createDefault()
{
    return CalendarDocumentStore(AppPaths::ensureUserDataDir(stapik::app::AppContext::instance().info().internalName));
}

CalendarDocument CalendarDocumentStore::load() const
{
    auto result = m_file.load();

    if (result.status == LoadStatus::Missing)
        return {};

    if (result.status != LoadStatus::Loaded || !result.document.has_value())
    {
        if (result.status == LoadStatus::NewerVersion || result.status == LoadStatus::MigrationFailed)
            preserveUnreadableFile();

        return {};
    }

    CalendarDocument document = std::move(*result.document);

    if (result.migrated)
    {
        // Saving rotates the backups first, so the pre-migration file stays available as calendar.json.bak.
        stapik::log::info("Migrated {} from schema version {}", m_file.filePath().string(), result.fileVersion);
        if (const auto saved = m_file.save(document); !saved)
            stapik::log::warning("Cannot save the migrated calendar: {}", saved.error().message());
    }

    if (const auto baseline = loadBaseline(); baseline.has_value())
        return document.withLastKnownCloudUpdate(*baseline);

    return document;
}

stapik::storage::FileResult CalendarDocumentStore::save(const CalendarDocument& document) const
{
    const auto documentSaved = m_file.save(document);
    if (!documentSaved)
        stapik::log::warning("Cannot save the calendar: {}", documentSaved.error().message());

    // Both files are always written; when both fail, the calendar itself is the failure worth reporting.
    const auto baselineSaved = saveBaseline(document.lastKnownCloudUpdate());
    return documentSaved ? baselineSaved : documentSaved;
}

std::vector<stapik::document::BackupInfo> CalendarDocumentStore::listBackups() const
{
    return m_file.listBackups();
}

std::optional<CalendarDocument> CalendarDocumentStore::loadBackup(const stapik::document::BackupInfo& backup) const
{
    auto result = m_file.loadBackup(backup);
    if (result.status != LoadStatus::Loaded || !result.document.has_value())
    {
        stapik::log::warning("Cannot restore {}: the backup cannot be used (status {})",
            stapik::storage::pathText(backup.path), static_cast<int>(result.status));
        return std::nullopt;
    }

    return std::move(*result.document);
}

std::optional<TimePoint> CalendarDocumentStore::loadBaseline() const
{
    std::ifstream file(m_baselinePath);
    if (!file.is_open())
        return std::nullopt;

    try
    {
        return stapik::sync::parseIso8601(nlohmann::json::parse(file).at(BASELINE_KEY).get<std::string>());
    }
    catch (const nlohmann::json::exception&)
    {
        return std::nullopt;
    }
}

stapik::storage::FileResult CalendarDocumentStore::saveBaseline(const std::optional<TimePoint>& baseline) const
{
    if (!baseline.has_value())
    {
        std::error_code errorCode;
        std::filesystem::remove(m_baselinePath, errorCode);
        return {};
    }

    const nlohmann::json json = {
        { BASELINE_KEY, stapik::sync::toIso8601(*baseline, stapik::sync::TimestampPrecision::Microseconds) }
    };

    const auto written = stapik::storage::writeFileAtomically(m_baselinePath, json.dump(2) + "\n");
    if (!written)
        stapik::log::warning("Cannot save the cloud baseline: {}", written.error().message());

    return written;
}

void CalendarDocumentStore::preserveUnreadableFile() const
{
    const auto stamp = std::format("{:%Y%m%d-%H%M%S}", std::chrono::floor<std::chrono::seconds>(std::chrono::system_clock::now()));
    const auto target = std::filesystem::path(std::format("{}.unreadable-{}", m_file.filePath().string(), stamp));

    std::error_code errorCode;
    std::filesystem::copy_file(m_file.filePath(), target, std::filesystem::copy_options::skip_existing, errorCode);

    if (errorCode)
        stapik::log::warning("Cannot keep a copy of the unreadable calendar file: {}", errorCode.message());
    else
        stapik::log::warning("The calendar file cannot be read by this version; a copy was kept as {}", target.string());
}

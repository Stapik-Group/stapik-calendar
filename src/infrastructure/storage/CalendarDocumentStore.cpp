#include "CalendarDocumentStore.hpp"

#include "../../core/model/CalendarYears.hpp"

#include "stapik/app/AppContext.hpp"
#include "stapik/log/Log.hpp"
#include "stapik/storage/AppPaths.hpp"
#include "stapik/storage/AtomicFile.hpp"
#include "stapik/storage/PathText.hpp"
#include "stapik/sync/Timestamp.hpp"
#include "stapik/text/JsonText.hpp"

#include <chrono>
#include <format>
#include <fstream>
#include <regex>
#include <string>
#include <utility>

namespace
{
    using stapik::document::LoadStatus;
    using TimePoint = CalendarDocument::TimePoint;

    constexpr int SCHEMA_VERSION = 1;
    constexpr int BACKUP_COUNT = 3;
    constexpr auto LEGACY_FILE_NAME = "calendar.json";
    constexpr auto BASELINE_FILE_NAME = "cloud-baselines.json";

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

    // The calendar has no files in the cloud besides the document itself, so only the settings come along.
    stapik::archive::AppArchiveContext archiveContext()
    {
        const auto& context = stapik::app::AppContext::instance();

        return {
            .appName = context.info().internalName,
            .appVersion = context.info().version,
            .settings = &context.settings(),
            .assets = nullptr
        };
    }
}

CalendarDocumentStore::CalendarDocumentStore(const std::filesystem::path& directory) :
    m_directory(directory),
    m_legacyFile(createFile(directory / LEGACY_FILE_NAME)),
    m_baselinePath(directory / BASELINE_FILE_NAME) {}

CalendarDocumentStore CalendarDocumentStore::createDefault()
{
    return CalendarDocumentStore(AppPaths::ensureUserDataDir(stapik::app::AppContext::instance().info().internalName));
}

std::map<int, CalendarDocument> CalendarDocumentStore::loadYears() const
{
    migrateLegacyFile();

    std::map<int, CalendarDocument> years;

    for (const auto& [year, path] : findYearFiles())
    {
        const auto result = createFile(path).load();

        if (result.status == LoadStatus::Loaded && result.document.has_value())
        {
            if (result.document->skippedEntryCount() > 0)
                setAsideUnreadableEntries(year, path, *result.document);

            years.emplace(year, *result.document);
        }
        else if (result.status == LoadStatus::NewerVersion || result.status == LoadStatus::MigrationFailed)
            preserveUnreadableFile(path);
    }

    return years;
}

stapik::storage::FileResult CalendarDocumentStore::saveYear(const int year, const CalendarDocument& document) const
{
    const auto saved = createFile(yearPath(year)).save(document);
    if (!saved)
        stapik::log::warning("Cannot save the calendar for {}: {}", year, saved.error().message());

    return saved;
}

std::vector<stapik::document::BackupInfo> CalendarDocumentStore::listBackups(const int year) const
{
    return createFile(yearPath(year)).listBackups();
}

std::optional<CalendarDocument> CalendarDocumentStore::loadBackup(const stapik::document::BackupInfo& backup) const
{
    auto result = m_legacyFile.loadBackup(backup);
    if (result.status != LoadStatus::Loaded || !result.document.has_value())
    {
        stapik::log::warning("Cannot restore {}: the backup cannot be used (status {})",
            stapik::storage::pathText(backup.path), static_cast<int>(result.status));
        return std::nullopt;
    }

    return std::move(*result.document);
}

stapik::archive::ArchiveResult<stapik::archive::ArchiveInfo> CalendarDocumentStore::exportArchive(
    const CalendarDocument& document, const std::filesystem::path& target) const
{
    return stapik::archive::exportAppArchive(m_legacyFile, document, archiveContext(), target);
}

stapik::archive::ArchiveResult<stapik::archive::ArchiveInfo> CalendarDocumentStore::importArchive(
    const std::filesystem::path& source,
    const std::function<stapik::storage::FileResult(const CalendarDocument&)>& apply) const
{
    const auto context = archiveContext();

    auto archive = stapik::archive::AppArchive::open(source, context.appName);
    if (!archive)
        return stapik::Failure(std::move(archive).error());

    auto loaded = m_legacyFile.deserialize(archive->documentText(), "the archive " + stapik::storage::pathText(source));
    switch (loaded.status)
    {
        using enum LoadStatus;
        case Loaded:
            break;
        case NewerVersion:
            return stapik::archive::archiveFailure(stapik::archive::ArchiveErrorKind::Unsupported,
                "The archive was made by a newer version of the application");
        case Missing:
        case Corrupted:
        case MigrationFailed:
        default:
            return stapik::archive::archiveFailure(stapik::archive::ArchiveErrorKind::Corrupted,
                "The document in the archive cannot be read");
    }

    if (const auto applied = apply(*loaded.document); !applied)
        return stapik::archive::archiveFailure(stapik::archive::ArchiveErrorKind::Io, applied.error().message());

    // The calendar is already stored here, so a failure to restore the settings is not an error of the import.
    if (context.settings != nullptr)
        static_cast<void>(archive->restoreSettings(*context.settings));

    return archive->info();
}

std::filesystem::path CalendarDocumentStore::yearPath(const int year) const
{
    return m_directory / std::format("calendar-{:04}.json", year);
}

std::map<int, std::filesystem::path> CalendarDocumentStore::findYearFiles() const
{
    static const std::regex pattern(R"(calendar-(\d{4})\.json)");

    std::map<int, std::filesystem::path> files;

    std::error_code errorCode;
    for (std::filesystem::directory_iterator it(m_directory, errorCode), end; !errorCode && it != end; it.increment(errorCode))
    {
        const auto name = it->path().filename().string();

        if (std::smatch match; std::regex_match(name, match, pattern))
            files.emplace(std::stoi(match[1].str()), it->path());
    }

    return files;
}

void CalendarDocumentStore::migrateLegacyFile() const
{
    const auto legacyPath = m_legacyFile.filePath();

    std::error_code errorCode;
    if (!std::filesystem::exists(legacyPath, errorCode))
        return;

    auto result = m_legacyFile.load();

    if (result.status != LoadStatus::Loaded || !result.document.has_value())
    {
        if (result.status == LoadStatus::NewerVersion || result.status == LoadStatus::MigrationFailed)
            preserveUnreadableFile(legacyPath);

        // A corrupted file was already moved aside by the load. A file from a newer version stays where it is.
        return;
    }

    const auto existing = findYearFiles();

    for (const auto& [year, entries] : calendar_years::splitByYear(result.document->entries()))
    {
        // A year that already has a file is newer than the single-file calendar, so it is never overwritten.
        if (existing.contains(year))
            continue;

        if (!saveYear(year, CalendarDocument(entries, result.document->lastUpdate())))
        {
            stapik::log::warning("Cannot split {} into years; it stays as it is and is tried again on the next start", legacyPath.string());
            return;
        }
    }

    const auto migratedPath = std::filesystem::path(legacyPath.string() + ".migrated");
    std::filesystem::rename(legacyPath, migratedPath, errorCode);

    if (errorCode)
        stapik::log::warning("Cannot rename {} after splitting it into years: {}", legacyPath.string(), errorCode.message());
    else
        stapik::log::info("Split {} into one file per year; the old file is kept as {}", legacyPath.string(), migratedPath.string());
}

std::map<int, TimePoint> CalendarDocumentStore::loadBaselines() const
{
    std::map<int, TimePoint> baselines;

    std::ifstream file(m_baselinePath);
    if (!file.is_open())
        return baselines;

    try
    {
        for (const auto& [key, value] : nlohmann::json::parse(file).items())
        {
            if (const auto time = stapik::sync::parseIso8601(value.get<std::string>()))
                baselines.emplace(std::stoi(key), *time);
        }
    }
    catch (const std::exception& exception)
    {
        stapik::log::warning("Cannot read the cloud baselines: {}", exception.what());
        return {};
    }

    return baselines;
}

stapik::storage::FileResult CalendarDocumentStore::saveBaselines(const std::map<int, TimePoint>& baselines) const
{
    if (baselines.empty())
    {
        std::error_code errorCode;
        std::filesystem::remove(m_baselinePath, errorCode);
        return {};
    }

    nlohmann::json json = nlohmann::json::object();
    for (const auto& [year, baseline] : baselines)
        json[std::format("{:04}", year)] = stapik::sync::toIso8601(baseline, stapik::sync::TimestampPrecision::Microseconds);

    const auto written = stapik::storage::writeFileAtomically(m_baselinePath, stapik::text::dumpJson(json, 2) + "\n");
    if (!written)
        stapik::log::warning("Cannot save the cloud baselines: {}", written.error().message());

    return written;
}

void CalendarDocumentStore::setAsideUnreadableEntries(const int year, const std::filesystem::path& path, const CalendarDocument& document) const
{
    stapik::log::warning("{} entries of the calendar for {} cannot be read and are left out", document.skippedEntryCount(), year);

    if (preserveUnreadableFile(path))
        static_cast<void>(saveYear(year, document));
}

bool CalendarDocumentStore::preserveUnreadableFile(const std::filesystem::path& path) const
{
    const auto stamp = std::format("{:%Y%m%d-%H%M%S}", std::chrono::floor<std::chrono::seconds>(std::chrono::system_clock::now()));
    const auto target = std::filesystem::path(std::format("{}.unreadable-{}", path.string(), stamp));

    std::error_code errorCode;
    std::filesystem::copy_file(path, target, std::filesystem::copy_options::skip_existing, errorCode);

    if (errorCode)
    {
        stapik::log::warning("Cannot keep a copy of the unreadable calendar file: {}", errorCode.message());
        return false;
    }

    stapik::log::warning("The calendar file {} cannot be read completely by this version; a copy was kept as {}", path.string(), target.string());
    return true;
}

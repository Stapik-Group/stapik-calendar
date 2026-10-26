#pragma once

#include "../../core/model/CalendarDocument.hpp"

#include "stapik/archive/AppArchive.hpp"
#include "stapik/document/BackupInfo.hpp"
#include "stapik/document/DocumentFile.hpp"
#include "stapik/storage/FileError.hpp"

#include <filesystem>
#include <map>
#include <optional>
#include <vector>

// Persists the calendar as one versioned, atomic file per year (calendar-YYYY.json, each with its own backups)
// and, separately, the local cloud baselines, so that the baseline never ends up in the cloud payload.
class CalendarDocumentStore
{
public:
    explicit CalendarDocumentStore(const std::filesystem::path& directory);

    [[nodiscard]] static CalendarDocumentStore createDefault();

    // Every year that has a file, keyed by year. The single-file calendar of older versions (calendar.json)
    // is split into year files first, once, and kept aside as calendar.json.migrated.
    [[nodiscard]] std::map<int, CalendarDocument> loadYears() const;
    [[nodiscard]] stapik::storage::FileResult saveYear(int year, const CalendarDocument& document) const;
    [[nodiscard]] std::vector<stapik::document::BackupInfo> listBackups(int year) const;
    [[nodiscard]] std::optional<CalendarDocument> loadBackup(const stapik::document::BackupInfo& backup) const;

    // The cloud timestamp each year was last synchronized with (local sync state, never uploaded).
    [[nodiscard]] std::map<int, CalendarDocument::TimePoint> loadBaselines() const;
    [[nodiscard]] stapik::storage::FileResult saveBaselines(const std::map<int, CalendarDocument::TimePoint>& baselines) const;

    // Writes the whole calendar (all years as one document) and the application settings into one archive file.
    [[nodiscard]] stapik::archive::ArchiveResult<stapik::archive::ArchiveInfo> exportArchive(
        const CalendarDocument& document, const std::filesystem::path& target) const;
    // Reads an archive made by exportArchive. `apply` stores the calendar from the archive; only when it succeeds
    // the settings are replaced in the settings store. Nothing is changed when reading the archive fails.
    template<std::invocable<const CalendarDocument&> ApplyDocument>
    [[nodiscard]] stapik::archive::ArchiveResult<stapik::archive::ArchiveInfo> importArchive(
        const std::filesystem::path& source,
        const ApplyDocument& apply) const
    {
        auto imported = openArchiveForImport(source);
        if (!imported)
            return stapik::Failure(std::move(imported).error());

        if (const auto applied = apply(imported->document); !applied)
            return stapik::archive::archiveFailure(stapik::archive::ArchiveErrorKind::Io, applied.error().message());

        restoreArchivedSettings(imported->archive);

        return imported->archive.info();
    }

private:
    struct OpenedArchive
    {
        stapik::archive::AppArchive archive;
        CalendarDocument document;
    };

    [[nodiscard]] std::filesystem::path yearPath(int year) const;
    [[nodiscard]] std::map<int, std::filesystem::path> findYearFiles() const;
    void migrateLegacyFile() const;
    static bool preserveUnreadableFile(const std::filesystem::path& path) ;
    void setAsideUnreadableEntries(int year, const std::filesystem::path& path, const CalendarDocument& document) const;
    [[nodiscard]] stapik::archive::ArchiveResult<OpenedArchive> openArchiveForImport(const std::filesystem::path& source) const;
    static void restoreArchivedSettings(const stapik::archive::AppArchive& archive) ;

    std::filesystem::path m_directory;
    // Only used to read and write the single-document formats: the legacy file, backups and archives.
    stapik::document::DocumentFile<CalendarDocument> m_legacyFile;
    std::filesystem::path m_baselinePath;
};

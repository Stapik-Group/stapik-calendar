#pragma once

#include "../../core/model/CalendarDocument.hpp"

#include "stapik/archive/AppArchive.hpp"
#include "stapik/document/BackupInfo.hpp"
#include "stapik/document/DocumentFile.hpp"
#include "stapik/storage/FileError.hpp"

#include <filesystem>
#include <functional>
#include <map>
#include <optional>
#include <vector>

// Persists the calendar as one versioned, atomic file per year (calendar-YYYY.json, each with its own backups)
// and, separately, the local cloud baseline, so that the baseline never ends up in the cloud payload.
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

    [[nodiscard]] std::optional<CalendarDocument::TimePoint> loadBaseline() const;
    [[nodiscard]] stapik::storage::FileResult saveBaseline(const std::optional<CalendarDocument::TimePoint>& baseline) const;

    // Writes the whole calendar (all years as one document) and the application settings into one archive file.
    [[nodiscard]] stapik::archive::ArchiveResult<stapik::archive::ArchiveInfo> exportArchive(
        const CalendarDocument& document, const std::filesystem::path& target) const;
    // Reads an archive made by exportArchive. `apply` stores the calendar from the archive; only when it succeeds
    // the settings are replaced in the settings store. Nothing is changed when reading the archive fails.
    [[nodiscard]] stapik::archive::ArchiveResult<stapik::archive::ArchiveInfo> importArchive(
        const std::filesystem::path& source,
        const std::function<stapik::storage::FileResult(const CalendarDocument&)>& apply) const;

private:
    [[nodiscard]] std::filesystem::path yearPath(int year) const;
    [[nodiscard]] std::map<int, std::filesystem::path> findYearFiles() const;
    void migrateLegacyFile() const;
    void preserveUnreadableFile(const std::filesystem::path& path) const;

    std::filesystem::path m_directory;
    // Only used to read and write the single-document formats: the legacy file, backups and archives.
    stapik::document::DocumentFile<CalendarDocument> m_legacyFile;
    std::filesystem::path m_baselinePath;
};

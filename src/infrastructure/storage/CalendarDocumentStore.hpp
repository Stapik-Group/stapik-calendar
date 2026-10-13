#pragma once

#include "../../core/model/CalendarDocument.hpp"

#include "stapik/archive/AppArchive.hpp"
#include "stapik/document/BackupInfo.hpp"
#include "stapik/document/DocumentFile.hpp"
#include "stapik/storage/FileError.hpp"

#include <filesystem>
#include <optional>
#include <vector>

// Persists the calendar document (versioned, atomic, with backups) and, separately,
// the local cloud baseline, so that the baseline never ends up in the cloud payload.
class CalendarDocumentStore
{
public:
    explicit CalendarDocumentStore(const std::filesystem::path& directory);

    [[nodiscard]] static CalendarDocumentStore createDefault();

    [[nodiscard]] CalendarDocument load() const;
    [[nodiscard]] stapik::storage::FileResult save(const CalendarDocument& document) const;
    [[nodiscard]] std::vector<stapik::document::BackupInfo> listBackups() const;
    [[nodiscard]] std::optional<CalendarDocument> loadBackup(const stapik::document::BackupInfo& backup) const;

    // Writes the calendar and the application settings into one archive file at `target`.
    [[nodiscard]] stapik::archive::ArchiveResult<stapik::archive::ArchiveInfo> exportArchive(
        const CalendarDocument& document, const std::filesystem::path& target) const;
    // Reads an archive made by exportArchive. On success the calendar is already saved to disk and the settings
    // are replaced in the settings store; nothing is changed when it fails.
    [[nodiscard]] stapik::archive::ArchiveResult<stapik::archive::RestoredApp<CalendarDocument>> importArchive(
        const std::filesystem::path& source) const;

private:
    [[nodiscard]] std::optional<CalendarDocument::TimePoint> loadBaseline() const;
    [[nodiscard]] stapik::storage::FileResult saveBaseline(const std::optional<CalendarDocument::TimePoint>& baseline) const;
    void preserveUnreadableFile() const;

    stapik::document::DocumentFile<CalendarDocument> m_file;
    std::filesystem::path m_baselinePath;
};

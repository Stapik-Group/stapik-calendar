#pragma once

#include "../../core/model/CalendarDocument.hpp"

#include "stapik/document/DocumentFile.hpp"

#include <filesystem>
#include <optional>

// Persists the calendar document (versioned, atomic, with backups) and, separately,
// the local cloud baseline, so that the baseline never ends up in the cloud payload.
class CalendarDocumentStore
{
public:
    explicit CalendarDocumentStore(const std::filesystem::path& directory);

    [[nodiscard]] static CalendarDocumentStore createDefault();

    [[nodiscard]] CalendarDocument load() const;
    [[nodiscard]] bool save(const CalendarDocument& document) const;

private:
    [[nodiscard]] std::optional<CalendarDocument::TimePoint> loadBaseline() const;
    [[nodiscard]] bool saveBaseline(const std::optional<CalendarDocument::TimePoint>& baseline) const;
    void preserveUnreadableFile() const;

    stapik::document::DocumentFile<CalendarDocument> m_file;
    std::filesystem::path m_baselinePath;
};

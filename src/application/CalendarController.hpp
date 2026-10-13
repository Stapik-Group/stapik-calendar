#pragma once

#include "../core/model/CalendarDocument.hpp"
#include "../core/model/CalendarYears.hpp"
#include "../infrastructure/storage/CalendarDocumentStore.hpp"

#include "stapik/archive/AppArchive.hpp"
#include "stapik/cloud/CloudStorageConfig.hpp"
#include "stapik/command/UndoStack.hpp"
#include "stapik/document/BackupInfo.hpp"
#include "stapik/sync/CloudSession.hpp"

#include <sigc++/connection.h>
#include <sigc++/signal.h>

#include <chrono>
#include <cstddef>
#include <filesystem>
#include <functional>
#include <map>
#include <optional>
#include <string>
#include <vector>

struct ConnectionResult
{
    bool connected = false;
    std::string message;
};

// Owns the calendar document, its undo history, local persistence and the cloud session.
// It knows nothing about widgets: views read from it and report user intents to it.
class CalendarController
{
public:
    using Date = std::chrono::year_month_day;

    CalendarController(CalendarDocumentStore store, stapik::sync::CloudSessionHooks hooks);

    CalendarController(const CalendarController&) = delete;
    CalendarController& operator=(const CalendarController&) = delete;

    [[nodiscard]] const std::vector<CalendarEntry>* entriesOn(Date date) const;
    [[nodiscard]] const CalendarEntry* findEntry(Date date, std::size_t index) const;
    [[nodiscard]] stapik::command::UndoStack& undoStack();

    void addEntry(Date date, CalendarEntry entry);
    void editEntry(Date date, std::size_t index, CalendarEntry entry);
    void deleteEntry(Date date, std::size_t index);
    void moveEntry(Date sourceDate, std::size_t index, Date destDate);
    void copyEntry(Date sourceDate, std::size_t index, Date destDate);
    void changeEntryColor(Date date, std::size_t index, EntryColor color);

    // The year the user is looking at. Backups are kept and restored per year, for this year.
    void setDisplayedYear(int year);

    // Backups of the displayed year.
    [[nodiscard]] std::vector<stapik::document::BackupInfo> listBackups() const;
    // Replaces the displayed year with a backup. Returns false, and changes nothing, when the backup cannot be used.
    [[nodiscard]] bool restoreBackup(const stapik::document::BackupInfo& backup);
    [[nodiscard]] stapik::archive::ArchiveResult<stapik::archive::ArchiveInfo> exportArchive(const std::filesystem::path& target) const;
    [[nodiscard]] stapik::archive::ArchiveResult<stapik::archive::ArchiveInfo> importArchive(const std::filesystem::path& source);

    void connect(const CloudStorageConfig& config);
    void syncNow();
    void flush();
    [[nodiscard]] bool isConnected() const;
    [[nodiscard]] std::optional<CloudStorageConfig> savedCloudConfig() const;
    [[nodiscard]] stapik::sync::SyncStatus syncStatus() const;

    sigc::signal<void()>& signalDocumentChanged();
    sigc::signal<void(stapik::sync::SyncStatus)>& signalSyncStatusChanged();
    sigc::signal<void(const ConnectionResult&)>& signalConnectionResult();
    // Emitted with the reason when saving to disk starts to fail; not repeated until a save succeeds again.
    sigc::signal<void(const std::string&)>& signalSaveFailed();
    // Emitted with the expiry time when the cloud reports that the API key is about to expire; once per reported time.
    sigc::signal<void(std::chrono::system_clock::time_point)>& signalApiKeyExpiring();

private:
    using Session = stapik::sync::CloudSession<CalendarDocument>;

    void onHistoryChanged();
    void onDocumentReplaced(const CalendarDocument& replacement);
    void onBaselineChanged(CalendarDocument::TimePoint baseline);
    void onSyncOutcome(const stapik::sync::SyncOutcome<CalendarDocument>& outcome);
    void clearHistorySilently();
    CalendarDocument loadFromStore();
    // Writes the years whose entries differ from what was last written, stamping them with `stamp`, and the baseline.
    void persist(CalendarDocument::TimePoint stamp);

    // What is on disk for one year: the entries as last written and the time they were written.
    struct YearState
    {
        CalendarDocument::TimePoint lastUpdate{};
        CalendarEntries saved;
    };

    CalendarDocumentStore m_store;
    std::map<int, YearState> m_years;    // filled by loadFromStore(), so it has to be declared before m_document
    CalendarDocument m_document;
    int m_displayedYear = static_cast<int>(std::chrono::year_month_day{ std::chrono::floor<std::chrono::days>(std::chrono::system_clock::now()) }.year());
    std::function<std::optional<CloudStorageConfig>()> m_loadCloudConfig;
    stapik::command::UndoStack m_history;
    sigc::connection m_historyConnection;
    Session m_session;
    bool m_awaitingConnectionResult = false;
    bool m_saveFailed = false;

    sigc::signal<void()> m_signalDocumentChanged;
    sigc::signal<void(const ConnectionResult&)> m_signalConnectionResult;
    sigc::signal<void(const std::string&)> m_signalSaveFailed;
};

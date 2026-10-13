#pragma once

#include "../core/model/CalendarDocument.hpp"
#include "../core/model/CalendarYears.hpp"
#include "../infrastructure/storage/CalendarDocumentStore.hpp"

#include "stapik/archive/AppArchive.hpp"
#include "stapik/cloud/CloudStorageConfig.hpp"
#include "stapik/cloud/ICloudStorage.hpp"
#include "stapik/command/UndoStack.hpp"
#include "stapik/document/BackupInfo.hpp"
#include "stapik/settings/ObservableSetting.hpp"
#include "stapik/sync/CloudSession.hpp"
#include "stapik/task/BackgroundTaskRunner.hpp"

#include <sigc++/connection.h>
#include <sigc++/signal.h>

#include <chrono>
#include <cstddef>
#include <filesystem>
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <set>
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

    // Each year is a separate cloud partition. Connecting checks the connection and, once per cloud, brings in the
    // single calendar document of older versions (which stays untouched). Years are loaded from the cloud when
    // they are shown: the displayed year at once, the others as the user moves to them.
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
    using TimePoint = CalendarDocument::TimePoint;

    // What is known about one year: what is on disk and what the cloud last confirmed.
    struct YearState
    {
        TimePoint lastUpdate{};
        CalendarEntries saved;
        std::optional<TimePoint> baseline;
    };

    struct LegacyRead
    {
        bool ok = false;
        std::optional<CloudDocument> document;
        std::string message;
    };

    void onHistoryChanged();
    void onPartitionReplaced(const std::string& partition, const CalendarDocument& replacement);
    void onPartitionBaselineChanged(const std::string& partition, TimePoint baseline);
    void onPartitionStatusChanged(const std::string& partition, stapik::sync::SyncStatus status);
    void onAvailableYears(const std::optional<std::vector<int>>& years);
    void onLegacyRead(const std::optional<LegacyRead>& result, bool migrate);
    void clearHistorySilently();
    CalendarDocument loadFromStore();

    // Writes the years whose entries differ from what was last written, stamping them with `stamp`.
    // Returns the years that changed.
    std::vector<int> persist(TimePoint stamp);
    void persistBaselines();
    void reportSaveResult(const std::optional<stapik::storage::FileError>& calendarFailure);

    void startCloud();
    void importLegacy(const std::optional<CloudDocument>& legacy);
    void ensureYearLoaded(int year);
    void ensureYearsLoaded();
    void pushYears(const std::vector<int>& years);
    void setStatus(const std::string& source, std::optional<stapik::sync::SyncStatus> status);
    [[nodiscard]] CalendarDocument yearDocument(int year) const;

    CalendarDocumentStore m_store;
    std::map<int, YearState> m_years;    // filled by loadFromStore(), so it has to be declared before m_document
    CalendarDocument m_document;
    int m_displayedYear = static_cast<int>(std::chrono::year_month_day{ std::chrono::floor<std::chrono::days>(std::chrono::system_clock::now()) }.year());
    std::set<int> m_visitedYears;
    std::function<std::optional<CloudStorageConfig>()> m_loadCloudConfig;
    std::function<std::unique_ptr<ICloudStorage>(const CloudStorageConfig&)> m_createStorage;
    stapik::settings::ObservableSetting<std::string> m_legacyMigratedFor;
    stapik::command::UndoStack m_history;
    sigc::connection m_historyConnection;
    Session m_session;
    std::unique_ptr<stapik::task::BackgroundTaskRunner> m_legacyRunner;
    std::set<int> m_availableYears;
    std::map<std::string, stapik::sync::SyncStatus> m_statuses;
    stapik::sync::SyncStatus m_combinedStatus = stapik::sync::SyncStatus::Idle;
    bool m_cloudReady = false;
    bool m_awaitingConnectionResult = false;
    bool m_saveFailed = false;

    sigc::signal<void(stapik::sync::SyncStatus)> m_signalSyncStatusChanged;
    sigc::signal<void()> m_signalDocumentChanged;
    sigc::signal<void(const ConnectionResult&)> m_signalConnectionResult;
    sigc::signal<void(const std::string&)> m_signalSaveFailed;
};

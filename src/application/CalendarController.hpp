#pragma once

#include "../core/model/CalendarDocument.hpp"
#include "../infrastructure/storage/CalendarDocumentStore.hpp"

#include "stapik/cloud/CloudStorageConfig.hpp"
#include "stapik/command/UndoStack.hpp"
#include "stapik/sync/CloudSession.hpp"

#include <sigc++/connection.h>
#include <sigc++/signal.h>

#include <chrono>
#include <cstddef>
#include <functional>
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

    void connect(const CloudStorageConfig& config);
    void syncNow();
    void flush();
    [[nodiscard]] bool isConnected() const;
    [[nodiscard]] std::optional<CloudStorageConfig> savedCloudConfig() const;
    [[nodiscard]] stapik::sync::SyncStatus syncStatus() const;

    sigc::signal<void()>& signalDocumentChanged();
    sigc::signal<void(stapik::sync::SyncStatus)>& signalSyncStatusChanged();
    sigc::signal<void(const ConnectionResult&)>& signalConnectionResult();

private:
    using Session = stapik::sync::CloudSession<CalendarDocument>;

    void onHistoryChanged();
    void onDocumentReplaced(const CalendarDocument& replacement);
    void onBaselineChanged(CalendarDocument::TimePoint baseline);
    void onSyncOutcome(const stapik::sync::SyncOutcome<CalendarDocument>& outcome);
    void clearHistorySilently();
    void persist() const;

    CalendarDocumentStore m_store;
    CalendarDocument m_document;
    std::function<std::optional<CloudStorageConfig>()> m_loadCloudConfig;
    stapik::command::UndoStack m_history;
    sigc::connection m_historyConnection;
    Session m_session;
    bool m_awaitingConnectionResult = false;

    sigc::signal<void()> m_signalDocumentChanged;
    sigc::signal<void(const ConnectionResult&)> m_signalConnectionResult;
};

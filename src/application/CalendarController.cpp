#include "CalendarController.hpp"

#include "../core/command/AddEntryCommand.hpp"
#include "../core/command/DeleteEntryCommand.hpp"
#include "../core/command/EditEntryCommand.hpp"
#include "../core/command/MoveEntryCommand.hpp"

#include "stapik/log/Log.hpp"

#include <memory>
#include <utility>

static_assert(stapik::sync::SyncableDocument<CalendarDocument>);

CalendarController::CalendarController(CalendarDocumentStore store, stapik::sync::CloudSessionHooks hooks) :
    m_store(std::move(store)),
    m_document(m_store.load()),
    m_loadCloudConfig(hooks.loadConfig),
    m_session(std::move(hooks))
{
    m_historyConnection = m_history.signalChanged().connect(sigc::mem_fun(*this, &CalendarController::onHistoryChanged));

    m_session.signalDocumentReplaced().connect(sigc::mem_fun(*this, &CalendarController::onDocumentReplaced));
    m_session.signalBaselineChanged().connect(sigc::mem_fun(*this, &CalendarController::onBaselineChanged));
    m_session.signalOutcome().connect(sigc::mem_fun(*this, &CalendarController::onSyncOutcome));

    if (m_session.isConnected())
        m_session.syncNow(m_document);
}

const std::vector<CalendarEntry>* CalendarController::entriesOn(const Date date) const
{
    const auto& entries = m_document.entries();
    const auto day = entries.find(date);
    return day == entries.end() ? nullptr : &day->second;
}

const CalendarEntry* CalendarController::findEntry(const Date date, const std::size_t index) const
{
    const auto* day = entriesOn(date);
    if (day == nullptr || index >= day->size())
        return nullptr;

    return &(*day)[index];
}

stapik::command::UndoStack& CalendarController::undoStack()
{
    return m_history;
}

void CalendarController::addEntry(const Date date, CalendarEntry entry)
{
    m_history.execute(std::make_unique<AddEntryCommand>(m_document.entries(), date, std::move(entry)));
}

void CalendarController::editEntry(const Date date, const std::size_t index, CalendarEntry entry)
{
    if (findEntry(date, index) == nullptr)
        return;

    m_history.execute(std::make_unique<EditEntryCommand>(m_document.entries(), date, index, std::move(entry)));
}

void CalendarController::deleteEntry(const Date date, const std::size_t index)
{
    if (findEntry(date, index) == nullptr)
        return;

    m_history.execute(std::make_unique<DeleteEntryCommand>(m_document.entries(), date, index));
}

void CalendarController::moveEntry(const Date sourceDate, const std::size_t index, const Date destDate)
{
    if (sourceDate == destDate || findEntry(sourceDate, index) == nullptr)
        return;

    m_history.execute(std::make_unique<MoveEntryCommand>(m_document.entries(), sourceDate, index, destDate));
}

void CalendarController::copyEntry(const Date sourceDate, const std::size_t index, const Date destDate)
{
    const auto* entry = findEntry(sourceDate, index);
    if (entry == nullptr)
        return;

    addEntry(destDate, *entry);
}

void CalendarController::changeEntryColor(const Date date, const std::size_t index, const EntryColor color)
{
    const auto* entry = findEntry(date, index);
    if (entry == nullptr)
        return;

    auto updated = *entry;
    updated.color = color;
    editEntry(date, index, std::move(updated));
}

std::vector<stapik::document::BackupInfo> CalendarController::listBackups() const
{
    return m_store.listBackups();
}

bool CalendarController::restoreBackup(const stapik::document::BackupInfo& backup)
{
    auto restored = m_store.loadBackup(backup);
    if (!restored)
        return false;

    // Commands hold indices into the old entries, so the history cannot outlive them.
    clearHistorySilently();

    if (const auto baseline = m_document.lastKnownCloudUpdate())
        restored->setLastKnownCloudUpdate(*baseline);

    m_document = std::move(*restored);

    // From here the restored calendar is a change like any other: stamped as the newest, saved, sent and shown.
    onHistoryChanged();
    return true;
}

void CalendarController::connect(const CloudStorageConfig& config)
{
    m_awaitingConnectionResult = true;

    if (m_session.connectWith(config, m_document))
        return;

    m_awaitingConnectionResult = false;
    m_signalConnectionResult.emit(ConnectionResult{ false, {} });
}

void CalendarController::syncNow()
{
    m_session.syncNow(m_document);
}

void CalendarController::flush()
{
    m_session.flush();
}

bool CalendarController::isConnected() const
{
    return m_session.isConnected();
}

std::optional<CloudStorageConfig> CalendarController::savedCloudConfig() const
{
    return m_loadCloudConfig ? m_loadCloudConfig() : std::nullopt;
}

stapik::sync::SyncStatus CalendarController::syncStatus() const
{
    return m_session.status();
}

sigc::signal<void()>& CalendarController::signalDocumentChanged()
{
    return m_signalDocumentChanged;
}

sigc::signal<void(stapik::sync::SyncStatus)>& CalendarController::signalSyncStatusChanged()
{
    return m_session.signalStatusChanged();
}

sigc::signal<void(const ConnectionResult&)>& CalendarController::signalConnectionResult()
{
    return m_signalConnectionResult;
}

sigc::signal<void(const std::string&)>& CalendarController::signalSaveFailed()
{
    return m_signalSaveFailed;
}

sigc::signal<void(std::chrono::system_clock::time_point)>& CalendarController::signalApiKeyExpiring()
{
    return m_session.signalApiKeyExpiring();
}

void CalendarController::onHistoryChanged()
{
    m_document.markUpdated();
    persist();
    m_session.pushChange(m_document);
    m_signalDocumentChanged.emit();
}

void CalendarController::onDocumentReplaced(const CalendarDocument& replacement)
{
    // Commands hold indices into the old entries, so they must not outlive the replacement.
    clearHistorySilently();
    m_document = replacement;
    persist();
    m_signalDocumentChanged.emit();
}

void CalendarController::onBaselineChanged(const CalendarDocument::TimePoint baseline)
{
    m_document.setLastKnownCloudUpdate(baseline);
    persist();
}

void CalendarController::onSyncOutcome(const stapik::sync::SyncOutcome<CalendarDocument>& outcome)
{
    if (!m_awaitingConnectionResult)
        return;

    m_awaitingConnectionResult = false;

    using enum stapik::sync::SyncState;
    const bool connected = outcome.state == Synchronized || outcome.state == ServerWon || outcome.state == LostRaceAcceptedServer;
    m_signalConnectionResult.emit(ConnectionResult{ connected, connected ? std::string{} : outcome.message });
}

void CalendarController::clearHistorySilently()
{
    m_historyConnection.block();
    m_history.clear();
    m_historyConnection.unblock();
}

void CalendarController::persist()
{
    const auto saved = m_store.save(m_document);
    if (saved)
    {
        m_saveFailed = false;
        return;
    }

    // The store has logged the reason. The user hears about it once per streak of failures, not after every edit.
    if (!std::exchange(m_saveFailed, true))
        m_signalSaveFailed.emit(saved.error().message());
}

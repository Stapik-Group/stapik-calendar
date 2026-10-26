#include "CalendarController.hpp"

#include "../core/command/AddEntryCommand.hpp"
#include "../core/command/DeleteEntryCommand.hpp"
#include "../core/command/EditEntryCommand.hpp"
#include "../core/command/MoveEntryCommand.hpp"

#include "stapik/app/AppContext.hpp"
#include "stapik/cloud/CloudStorageException.hpp"
#include "stapik/log/Log.hpp"
#include "stapik/storage/CloudStorageConfigStorage.hpp"
#include "stapik/sync/PartitionKey.hpp"
#include "stapik/sync/SyncEnvelope.hpp"

#include <algorithm>
#include <memory>
#include <set>
#include <utility>

static_assert(stapik::sync::SyncableDocument<CalendarDocument>);

namespace
{
    constexpr auto LEGACY_MIGRATED_KEY = "cloud.legacyMigratedFor";
    constexpr auto LEGACY_STATUS_SOURCE = "legacy";

    // Which status wins when the partitions report different ones: the worst news first.
    int statusSeverity(const stapik::sync::SyncStatus status)
    {
        using enum stapik::sync::SyncStatus;
        switch (status)
        {
            case Error: return 4;
            case Conflict: return 3;
            case Offline: return 2;
            case Syncing: return 1;
            case Idle: return 0;
        }

        return 0;
    }

    std::optional<SyncProblem> permanentProblemOf(const stapik::sync::SyncState state)
    {
        using enum stapik::sync::SyncState;
        switch (state)
        {
            case AuthRejected: return SyncProblem::AuthRejected;
            case DecryptionFailed: return SyncProblem::DecryptionFailed;
            case DocumentTooLarge: return SyncProblem::DocumentTooLarge;
            case Synchronized:
            case OfflineKeptLocal:
            case ServerWon:
            case LostRaceAcceptedServer:
            case RemoteUnreadable:
            case LostRaceKeptLocal:
            default: return std::nullopt;
        }
    }

    // Adds the entries of `source` that `target` does not have yet. Returns whether anything was added.
    bool addMissingEntries(CalendarEntries& target, const CalendarEntries& source)
    {
        bool added = false;

        for (const auto& [date, dayEntries] : source)
        {
            auto& existing = target[date];

            for (const auto& entry : dayEntries)
            {
                if (std::ranges::find(existing, entry) == existing.end())
                {
                    existing.push_back(entry);
                    added = true;
                }
            }

            if (existing.empty())
                target.erase(date);
        }

        return added;
    }
}

CalendarController::CalendarController(CalendarDocumentStore store, stapik::sync::CloudSessionHooks hooks) :
    m_store(std::move(store)),
    m_document(loadFromStore()),
    m_loadCloudConfig(hooks.loadConfig),
    m_createStorage(hooks.createStorage),
    m_legacyMigratedFor(stapik::app::AppContext::instance().settings(), LEGACY_MIGRATED_KEY, std::string{}),
    m_session(std::move(hooks))
{
    m_visitedYears.insert(m_displayedYear);
    m_historyConnection = m_history.signalChanged().connect(sigc::mem_fun(*this, &CalendarController::onHistoryChanged));

    m_session.signalPartitionReplaced().connect(sigc::mem_fun(*this, &CalendarController::onPartitionReplaced));
    m_session.signalPartitionBaselineChanged().connect(sigc::mem_fun(*this, &CalendarController::onPartitionBaselineChanged));
    m_session.signalPartitionStatusChanged().connect(sigc::mem_fun(*this, &CalendarController::onPartitionStatusChanged));
    m_session.signalAvailableYears().connect(sigc::mem_fun(*this, &CalendarController::onAvailableYears));
    m_session.signalPartitionOutcome().connect(sigc::mem_fun(*this, &CalendarController::onPartitionOutcome));

    if (m_session.isConnected())
        startCloud();
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

void CalendarController::setDisplayedYear(const int year)
{
    m_displayedYear = year;
    m_visitedYears.insert(year);
    ensureYearLoaded(year);
}

std::vector<stapik::document::BackupInfo> CalendarController::listBackups() const
{
    return m_store.listBackups(m_displayedYear);
}

bool CalendarController::restoreBackup(const stapik::document::BackupInfo& backup)
{
    auto restored = m_store.loadBackup(backup);
    if (!restored)
        return false;

    // Commands hold indices into the old entries, so the history cannot outlive them.
    clearHistorySilently();

    calendar_years::replaceYear(m_document.entries(), m_displayedYear, restored->entries());

    // From here the restored year is a change like any other: stamped as the newest, saved, sent and shown.
    onHistoryChanged();
    return true;
}

stapik::archive::ArchiveResult<stapik::archive::ArchiveInfo> CalendarController::exportArchive(const std::filesystem::path& target) const
{
    return m_store.exportArchive(m_document, target);
}

stapik::archive::ArchiveResult<stapik::archive::ArchiveInfo> CalendarController::importArchive(const std::filesystem::path& source)
{
    return m_store.importArchive(source, [this](const CalendarDocument& imported)
    {
        // Commands hold indices into the old entries, so the history cannot outlive them.
        clearHistorySilently();

        CalendarDocument restored = imported;
        if (const auto baseline = m_document.lastKnownCloudUpdate())
            restored.setLastKnownCloudUpdate(*baseline);

        m_document = std::move(restored);

        // The imported calendar is a change like any other. Years missing from the archive end up empty.
        // A failure to write is reported the usual way, through signalSaveFailed.
        onHistoryChanged();
        return stapik::storage::FileResult{};
    });
}

void CalendarController::connect(const CloudStorageConfig& config)
{
    m_awaitingConnectionResult = true;
    m_reportedProblem.reset();
    m_statuses.clear();
    discardBaselinesOfPreviousCloud();

    if (m_session.connectPartitionsOnly(config))
    {
        startCloud();
        m_signalConnectionChanged.emit();
        return;
    }

    m_awaitingConnectionResult = false;
    m_signalConnectionResult.emit(ConnectionResult{ false, {} });
    m_signalConnectionChanged.emit();
}

void CalendarController::disconnect()
{
    m_legacyRunner.reset();
    m_awaitingConnectionResult = false;
    m_reportedProblem.reset();
    m_session.disconnect();

    m_cloudReady = false;
    m_availableYears.clear();
    discardBaselinesOfPreviousCloud();
    clearStatuses();
    m_signalConnectionChanged.emit();
}

void CalendarController::syncNow()
{
    if (!m_session.isConnected())
        return;

    // Load everything that is in use again, so that changes made elsewhere are picked up.
    for (const int year : m_visitedYears)
        m_session.forgetPartition(stapik::sync::partitionKeyForYear(year));

    m_cloudReady = false;
    m_session.fetchAvailableYears();
}

void CalendarController::flushAndWait()
{
    m_session.flushAndWait();
}

bool CalendarController::isConnected() const
{
    return m_session.isConnected();
}

std::optional<CloudStorageConfig> CalendarController::savedCloudConfig() const
{
    return m_loadCloudConfig ? m_loadCloudConfig() : std::nullopt;
}

bool CalendarController::cloudSecretsUnreadable() const
{
    const auto config = savedCloudConfig();
    return config && config->secretsUnreadable;
}

bool CalendarController::secretsKeptInPlainText() const
{
    return CloudStorageConfigStorage::secretsKeptInConfigFile(stapik::app::AppContext::instance().info().internalName).any();
}

stapik::sync::SyncStatus CalendarController::syncStatus() const
{
    return m_combinedStatus;
}

sigc::signal<void()>& CalendarController::signalDocumentChanged()
{
    return m_signalDocumentChanged;
}

sigc::signal<void(stapik::sync::SyncStatus)>& CalendarController::signalSyncStatusChanged()
{
    return m_signalSyncStatusChanged;
}

sigc::signal<void(const ConnectionResult&)>& CalendarController::signalConnectionResult()
{
    return m_signalConnectionResult;
}

sigc::signal<void()>& CalendarController::signalConnectionChanged()
{
    return m_signalConnectionChanged;
}

sigc::signal<void(const SyncProblemReport&)>& CalendarController::signalSyncProblem()
{
    return m_signalSyncProblem;
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
    const auto now = std::chrono::system_clock::now();
    m_document.markUpdated(now);
    pushYears(persist(now));
    m_signalDocumentChanged.emit();
}

void CalendarController::onPartitionReplaced(const std::string& partition, const CalendarDocument& replacement)
{
    const auto year = stapik::sync::yearFromPartitionKey(partition);
    if (!year)
        return;

    const auto entries = calendar_years::entriesOfYear(replacement.entries(), *year);

    auto& state = m_years[*year];
    state.baseline = replacement.lastKnownCloudUpdate();

    if (calendar_years::entriesOfYear(m_document.entries(), *year) != entries)
    {
        // Commands hold indices into the old entries, so they must not outlive the replacement.
        clearHistorySilently();
        calendar_years::replaceYear(m_document.entries(), *year, entries);
        persist(replacement.lastUpdate());
        m_signalDocumentChanged.emit();
    }

    // Whatever the entries were, the year now carries the time of the version that came from the cloud.
    state.lastUpdate = replacement.lastUpdate();
    persistBaselines();
}

void CalendarController::onPartitionBaselineChanged(const std::string& partition, const TimePoint baseline)
{
    if (const auto year = stapik::sync::yearFromPartitionKey(partition))
    {
        m_years[*year].baseline = baseline;
        persistBaselines();
    }
}

void CalendarController::onPartitionStatusChanged(const std::string& partition, const stapik::sync::SyncStatus status)
{
    setStatus(partition, status);
}

void CalendarController::onAvailableYears(const std::optional<std::vector<int>>& years)
{
    m_cloudReady = true;

    if (years)
    {
        m_availableYears = std::set<int>(years->begin(), years->end());
        setStatus(LEGACY_STATUS_SOURCE, std::nullopt);
    }

    ensureYearsLoaded();
}

void CalendarController::onPartitionOutcome(const std::string& partition, const stapik::sync::SyncOutcome<CalendarDocument>& outcome)
{
    const auto year = stapik::sync::yearFromPartitionKey(partition);
    if (!year)
        return;

    if (outcome.discardedLocalChange)
        m_signalSyncProblem.emit(SyncProblemReport{ SyncProblem::LocalChangeDiscarded, *year });

    if (const auto problem = permanentProblemOf(outcome.state))
    {
        if (std::exchange(m_reportedProblem, problem) != problem)
            m_signalSyncProblem.emit(SyncProblemReport{ *problem, *year });

        return;
    }

    if (outcome.state == stapik::sync::SyncState::Synchronized)
        m_reportedProblem.reset();
}

void CalendarController::onLegacyRead(const std::optional<LegacyRead>& result, const bool migrate)
{
    const bool ok = result && result->ok;

    if (m_awaitingConnectionResult)
    {
        m_awaitingConnectionResult = false;
        m_signalConnectionResult.emit(ConnectionResult{ ok, ok ? std::string{} : (result ? result->message : std::string{}) });
    }

    if (ok)
    {
        setStatus(LEGACY_STATUS_SOURCE, std::nullopt);

        if (migrate)
            importLegacy(result->document);
    }
    else
    {
        setStatus(LEGACY_STATUS_SOURCE, stapik::sync::SyncStatus::Offline);
    }

    // The years follow in any case; when the cloud is out of reach they are sent as soon as it is back.
    m_session.fetchAvailableYears();
}

void CalendarController::clearHistorySilently()
{
    m_historyConnection.block();
    m_history.clear();
    m_historyConnection.unblock();
}

CalendarDocument CalendarController::loadFromStore()
{
    CalendarEntries entries;
    TimePoint lastUpdate{};

    for (const auto& [year, document] : m_store.loadYears())
    {
        // A year file holds only its own year; anything else in it would be written back to the wrong file.
        auto yearEntries = calendar_years::entriesOfYear(document.entries(), year);

        entries.insert(yearEntries.begin(), yearEntries.end());
        lastUpdate = std::max(lastUpdate, document.lastUpdate());
        m_years[year] = YearState{ document.lastUpdate(), std::move(yearEntries), std::nullopt };
    }

    for (const auto& [year, baseline] : m_store.loadBaselines())
    {
        if (const auto state = m_years.find(year); state != m_years.end())
            state->second.baseline = baseline;
    }

    return CalendarDocument(std::move(entries), lastUpdate);
}

std::vector<int> CalendarController::persist(const TimePoint stamp)
{
    const auto current = calendar_years::splitByYear(m_document.entries());

    std::set<int> years;
    for (const auto& [year, entries] : current)
        years.insert(year);
    for (const auto& [year, state] : m_years)
        years.insert(year);

    const CalendarEntries none;
    std::vector<int> changed;
    std::optional<stapik::storage::FileError> failure;

    for (const int year : years)
    {
        const auto currentIt = current.find(year);
        const auto& entries = currentIt == current.end() ? none : currentIt->second;

        auto stateIt = m_years.find(year);
        if (stateIt == m_years.end())
            stateIt = m_years.emplace(year, YearState{}).first;
        else if (stateIt->second.saved == entries)
            continue;

        changed.push_back(year);
        stateIt->second.lastUpdate = stamp;

        if (const auto saved = m_store.saveYear(year, CalendarDocument(entries, stamp)); saved)
            stateIt->second.saved = entries;
        else if (!failure.has_value())
            failure = saved.error();
    }

    reportSaveResult(failure);
    return changed;
}

void CalendarController::persistBaselines()
{
    std::map<int, TimePoint> baselines;
    for (const auto& [year, state] : m_years)
    {
        if (state.baseline)
            baselines.emplace(year, *state.baseline);
    }

    const auto saved = m_store.saveBaselines(baselines);
    reportSaveResult(saved ? std::nullopt : std::optional<stapik::storage::FileError>(saved.error()));
}

void CalendarController::discardBaselinesOfPreviousCloud()
{
    for (auto& [year, state] : m_years)
        state.baseline.reset();

    persistBaselines();
}

void CalendarController::reportSaveResult(const std::optional<stapik::storage::FileError>& calendarFailure)
{
    if (!calendarFailure.has_value())
    {
        m_saveFailed = false;
        return;
    }

    // The store has logged the reason. The user hears about it once per streak of failures, not after every edit.
    if (!std::exchange(m_saveFailed, true))
        m_signalSaveFailed.emit(calendarFailure->message());
}

void CalendarController::startCloud()
{
    const auto config = m_session.config();
    if (!config || !m_createStorage)
        return;

    m_cloudReady = false;
    m_availableYears.clear();
    setStatus(LEGACY_STATUS_SOURCE, stapik::sync::SyncStatus::Syncing);

    const bool migrate = m_legacyMigratedFor.get() != config->normalizedApiUrl();

    if (!m_legacyRunner)
        m_legacyRunner = std::make_unique<stapik::task::BackgroundTaskRunner>();

    // Reading the old single document is also what shows whether the address and the key work.
    // It is only read, never written: the old document stays as it is for older versions of the application.
    m_legacyRunner->submit(
        [create = m_createStorage, config = *config]() -> LegacyRead
        {
            try
            {
                const auto storage = create(config);
                if (!storage)
                    return LegacyRead{ .ok = false, .document = std::nullopt, .message = "Cannot create the cloud client" };

                return LegacyRead{ .ok = true, .document = storage->loadDocument(), .message = {} };
            }
            catch (const CloudStorageException& exception)
            {
                return LegacyRead{ .ok = false, .document = std::nullopt, .message = exception.what() };
            }
        },
        [this, migrate](const std::optional<LegacyRead> result) { onLegacyRead(result, migrate); });
}

void CalendarController::importLegacy(const std::optional<CloudDocument>& legacy)
{
    const auto config = m_session.config();
    const std::string cloudId = config ? config->normalizedApiUrl() : std::string{};

    if (!legacy || legacy->content.is_null() || legacy->content.empty())
    {
        m_legacyMigratedFor.set(cloudId);
        return;
    }

    std::optional<CalendarDocument> document;
    TimePoint stamp{};

    try
    {
        const auto envelope = stapik::sync::SyncEnvelope::fromJson(legacy->content);
        document = CalendarDocument::fromJson(envelope.payload);
        stamp = envelope.lastUpdate;
    }
    catch (const std::exception& exception)
    {
        stapik::log::warning("Cannot read the calendar document of older versions from the cloud: {}", exception.what());
        return;
    }

    bool imported = false;
    for (const auto& [year, entries] : calendar_years::splitByYear(document->entries()))
    {
        auto yearEntries = calendar_years::entriesOfYear(m_document.entries(), year);

        // What is here stays; what the old document has beyond it is added.
        if (addMissingEntries(yearEntries, entries))
        {
            calendar_years::replaceYear(m_document.entries(), year, yearEntries);
            imported = true;
        }
    }

    if (imported)
    {
        // Commands hold indices into the old entries, so they must not outlive the import.
        clearHistorySilently();
        persist(stamp);
        m_signalDocumentChanged.emit();
        stapik::log::info("Imported the calendar document of older versions from the cloud");
    }

    m_legacyMigratedFor.set(cloudId);
}

CalendarDocument CalendarController::yearDocument(const int year) const
{
    const auto state = m_years.find(year);

    return CalendarDocument(
        calendar_years::entriesOfYear(m_document.entries(), year),
        state == m_years.end() ? TimePoint{} : state->second.lastUpdate,
        state == m_years.end() ? std::nullopt : state->second.baseline);
}

void CalendarController::ensureYearLoaded(const int year)
{
    if (!m_cloudReady || !m_session.isConnected())
        return;

    const auto partition = stapik::sync::partitionKeyForYear(year);
    if (m_session.isPartitionLoaded(partition))
        return;

    const auto state = m_years.find(year);
    const bool hasLocalEntries = state != m_years.end() && !state->second.saved.empty();

    // A year nobody has written anywhere is not created in the cloud just because it was looked at.
    if (hasLocalEntries || m_availableYears.contains(year))
        m_session.loadPartition(partition, yearDocument(year));
}

void CalendarController::ensureYearsLoaded()
{
    std::set<int> years = m_visitedYears;
    for (const auto& [year, state] : m_years)
    {
        if (!state.saved.empty())
            years.insert(year);
    }

    for (const int year : years)
        ensureYearLoaded(year);
}

void CalendarController::pushYears(const std::vector<int>& years)
{
    if (!m_session.isConnected())
        return;

    for (const int year : years)
    {
        const auto partition = stapik::sync::partitionKeyForYear(year);

        const auto state = m_years.find(year);
        const bool synchronizedBefore = state != m_years.end() && state->second.baseline.has_value();

        // A year this device never synchronized may exist in the cloud with entries it has not seen yet,
        // so it is resolved against the cloud first instead of being written over it.
        if (m_session.isPartitionLoaded(partition) || synchronizedBefore)
            m_session.pushPartitionChange(partition, yearDocument(year));
        else if (m_cloudReady)
            m_session.loadPartition(partition, yearDocument(year));
    }
}

void CalendarController::setStatus(const std::string& source, const std::optional<stapik::sync::SyncStatus> status)
{
    if (status)
        m_statuses[source] = *status;
    else
        m_statuses.erase(source);

    updateCombinedStatus();
}

void CalendarController::clearStatuses()
{
    m_statuses.clear();
    updateCombinedStatus();
}

void CalendarController::updateCombinedStatus()
{
    auto combined = stapik::sync::SyncStatus::Idle;
    for (const auto& [name, value] : m_statuses)
    {
        if (statusSeverity(value) > statusSeverity(combined))
            combined = value;
    }

    if (std::exchange(m_combinedStatus, combined) != combined)
        m_signalSyncStatusChanged.emit(combined);
}

#include "MenuActionHandler.hpp"

#include "stapik/app/AppContext.hpp"
#include "stapik/locale/DateFormatter.hpp"
#include "stapik/locale/LocaleManager.hpp"
#include "stapik/storage/PathText.hpp"
#include "stapik/theme/FontManager.hpp"
#include "stapik/theme/ThemeManager.hpp"
#include "stapik/ui/dialog/ConfirmDialog.hpp"
#include "stapik/ui/dialog/ConnectDialog.hpp"
#include "stapik/ui/dialog/DialogUtils.hpp"
#include "stapik/ui/dialog/FileDialogs.hpp"
#include "stapik/ui/dialog/RestoreBackupDialog.hpp"

#include <chrono>
#include <format>
#include <utility>

namespace
{
    constexpr auto ARCHIVE_DIRECTORY_KEY = "archive.lastDirectory";
    constexpr auto ARCHIVE_EXTENSION = ".zip";

    std::string defaultArchiveName()
    {
        const auto today = stapik::locale::toLocalTime(std::chrono::system_clock::now());
        return std::format("{}-{:%Y-%m-%d}{}", stapik::app::AppContext::instance().info().internalName, today, ARCHIVE_EXTENSION);
    }

    std::string importFailureText(const stapik::archive::ArchiveError& error)
    {
        const auto& loc = LocaleManager::instance();

        switch (error.kind)
        {
            using enum stapik::archive::ArchiveErrorKind;
            case NotAnArchive: return loc.translate("archive.import.failed.notAnArchive");
            case WrongApplication: return loc.translate("archive.import.failed.wrongApplication");
            case Unsupported: return loc.translate("archive.import.failed.unsupported");
            case Corrupted: return loc.translate("archive.import.failed.corrupted");
            case Io:
            case Cloud:
            default: return loc.translate("archive.import.failed.io", { { "reason", error.detail } });
        }
    }

    // What the settings windows and the application already read from the settings store is not updated by replacing it.
    void reloadSharedSettings()
    {
        ThemeManager::instance().themeSetting().reload();
        FontManager::instance().fontSetting().reload();
        LocaleManager::instance().languageSetting().reload();
    }
}

MenuActionHandler::MenuActionHandler(Gtk::ApplicationWindow& window, CalendarController& controller, std::function<void()> onSettingsRestored):
    m_window(window),
    m_controller(controller),
    m_onSettingsRestored(std::move(onSettingsRestored))
{
    m_connectionResult = m_controller.signalConnectionResult().connect(
        sigc::mem_fun(*this, &MenuActionHandler::onConnectionResult));
}

MenuActionHandler::~MenuActionHandler()
{
    m_connectionResult.disconnect();
    m_connectionChanged.disconnect();
}

void MenuActionHandler::registerActions()
{
    m_window.add_action("connect", sigc::mem_fun(*this, &MenuActionHandler::onActionConnect));
    m_disconnectAction = m_window.add_action("disconnect", sigc::mem_fun(*this, &MenuActionHandler::onActionDisconnect));
    m_window.add_action("quit", sigc::mem_fun(*this, &MenuActionHandler::onActionQuit));
    m_window.add_action("sync", sigc::mem_fun(*this, &MenuActionHandler::onActionSync));
    m_window.add_action("restoreBackup", sigc::mem_fun(*this, &MenuActionHandler::onActionRestoreBackup));
    m_window.add_action("exportArchive", sigc::mem_fun(*this, &MenuActionHandler::onActionExportArchive));
    m_window.add_action("importArchive", sigc::mem_fun(*this, &MenuActionHandler::onActionImportArchive));

    m_connectionChanged = m_controller.signalConnectionChanged().connect(
        sigc::mem_fun(*this, &MenuActionHandler::updateDisconnectAvailability));
    updateDisconnectAvailability();
}

void MenuActionHandler::onActionConnect() const
{
    const auto saved = m_controller.savedCloudConfig();
    const bool secretsUnreadable = saved && saved->secretsUnreadable;

    showConnectDialog(m_window, saved, [this, secretsUnreadable](const CloudStorageConfig& chosen)
    {
        auto config = chosen;
        config.secretsUnreadable = secretsUnreadable;
        m_controller.connect(config);
    });
}

void MenuActionHandler::onActionDisconnect() const
{
    const auto& loc = LocaleManager::instance();

    const ConfirmDialogOptions confirmation{
        .title = loc.translate("dialog.disconnect.title"),
        .message = loc.translate("dialog.disconnect.message"),
        .confirmLabel = loc.translate("dialog.disconnect.button"),
        .destructive = true };

    showConfirmDialog(m_window, confirmation, [this] { m_controller.disconnect(); });
}

void MenuActionHandler::onActionQuit() const
{
    // Closing the window (instead of quitting the application) lets MainWindow flush pending changes first.
    m_window.close();
}

void MenuActionHandler::onActionSync() const
{
    m_controller.syncNow();
}

void MenuActionHandler::onActionRestoreBackup() const
{
    showRestoreBackupDialog(m_window, m_controller.listBackups(), [this](const stapik::document::BackupInfo& backup)
    {
        if (m_controller.restoreBackup(backup))
            return;

        const auto& loc = LocaleManager::instance();
        showMessageDialog(m_window, loc.translate("backup.restore.failed.title"), loc.translate("backup.restore.failed.text"));
    });
}

void MenuActionHandler::onActionExportArchive() const
{
    const auto& loc = LocaleManager::instance();

    const FileDialogOptions options{
        .title = loc.translate("archive.export.title"),
        .initialName = defaultArchiveName(),
        .lastDirectoryKey = ARCHIVE_DIRECTORY_KEY };

    showSaveFileDialog(m_window, options, [this](const std::filesystem::path& target) { exportArchiveTo(target); });
}

void MenuActionHandler::onActionImportArchive() const
{
    const auto& loc = LocaleManager::instance();

    const FileDialogOptions options{
        .title = loc.translate("archive.import.title"),
        .initialName = {},
        .lastDirectoryKey = ARCHIVE_DIRECTORY_KEY };

    showOpenFileDialog(m_window, options, [this](const std::filesystem::path& source)
    {
        const auto& texts = LocaleManager::instance();

        const ConfirmDialogOptions confirmation{
            .title = texts.translate("dialog.archive.import.title"),
            .message = texts.translate("archive.import.confirm"),
            .confirmLabel = texts.translate("dialog.archive.import.button"),
            .destructive = true };

        showConfirmDialog(m_window, confirmation, [this, source] { importArchiveFrom(source); });
    });
}

void MenuActionHandler::exportArchiveTo(const std::filesystem::path& target) const
{
    const auto& loc = LocaleManager::instance();

    if (const auto exported = m_controller.exportArchive(target); !exported)
    {
        showMessageDialog(m_window, loc.translate("archive.export.failed.title"),
            loc.translate("archive.export.failed.text", { { "reason", exported.error().message() } }));
        return;
    }

    showMessageDialog(m_window, loc.translate("archive.export.done.title"),
        loc.translate("archive.export.done.text", { { "path", stapik::storage::pathText(target) } }));
}

void MenuActionHandler::importArchiveFrom(const std::filesystem::path& source) const
{
    if (const auto imported = m_controller.importArchive(source); !imported)
    {
        const auto& loc = LocaleManager::instance();
        showMessageDialog(m_window, loc.translate("archive.import.failed.title"), importFailureText(imported.error()));
        return;
    }

    reloadSharedSettings();
    if (m_onSettingsRestored)
        m_onSettingsRestored();

    // Read after the reload, so that the message is in the restored language.
    const auto& loc = LocaleManager::instance();
    showMessageDialog(m_window, loc.translate("archive.import.done.title"), loc.translate("archive.import.done.text"));
}

void MenuActionHandler::updateDisconnectAvailability() const
{
    m_disconnectAction->set_enabled(m_controller.isConnected());
}

void MenuActionHandler::onConnectionResult(const ConnectionResult& result) const
{
    const auto& loc = LocaleManager::instance();

    if (!result.connected)
    {
        showMessageDialog(m_window, loc.translate("cloud.failed.header"), result.message);
        return;
    }

    std::string text = loc.translate("cloud.connected.secondary");
    if (CalendarController::secretsKeptInPlainText())
        text += "\n\n" + loc.translate("cloud.secretsInConfigFile");

    showMessageDialog(m_window, loc.translate("cloud.connected"), text);
}

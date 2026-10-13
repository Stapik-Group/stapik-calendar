#include "MenuActionHandler.hpp"

#include "../../infrastructure/help/UserGuide.hpp"

#include "stapik/locale/LocaleManager.hpp"
#include "stapik/ui/dialog/ConnectDialog.hpp"
#include "stapik/ui/dialog/DialogUtils.hpp"
#include "stapik/ui/dialog/RestoreBackupDialog.hpp"

MenuActionHandler::MenuActionHandler(Gtk::ApplicationWindow& window, CalendarController& controller):
    m_window(window),
    m_controller(controller)
{
    m_connectionResult = m_controller.signalConnectionResult().connect(
        sigc::mem_fun(*this, &MenuActionHandler::onConnectionResult));
}

MenuActionHandler::~MenuActionHandler()
{
    m_connectionResult.disconnect();
}

void MenuActionHandler::registerActions()
{
    m_window.add_action("connect", sigc::mem_fun(*this, &MenuActionHandler::onActionConnect));
    m_window.add_action("quit", sigc::mem_fun(*this, &MenuActionHandler::onActionQuit));
    m_window.add_action("sync", sigc::mem_fun(*this, &MenuActionHandler::onActionSync));
    m_window.add_action("guide", sigc::mem_fun(*this, &MenuActionHandler::onActionGuide));
    m_window.add_action("restoreBackup", sigc::mem_fun(*this, &MenuActionHandler::onActionRestoreBackup));
}

void MenuActionHandler::onActionConnect() const
{
    showConnectDialog(
        m_window,
        m_controller.savedCloudConfig(),
        [this](const CloudStorageConfig& config) { m_controller.connect(config); });
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

void MenuActionHandler::onActionGuide() const
{
    const auto& loc = LocaleManager::instance();
    if (UserGuide::open(loc.languageCode()))
        return;

    showMessageDialog(m_window, loc.translate("guide.error.title"), loc.translate("guide.error.text"));
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

void MenuActionHandler::onConnectionResult(const ConnectionResult& result) const
{
    const auto& loc = LocaleManager::instance();

    if (result.connected)
        showMessageDialog(m_window, loc.translate("cloud.connected"), loc.translate("cloud.connected.secondary"));
    else
        showMessageDialog(m_window, loc.translate("cloud.failed.header"), result.message);
}

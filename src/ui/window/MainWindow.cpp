#include "MainWindow.hpp"

#include "../../application/CalendarCloudHooks.hpp"

#include "stapik/app/AppContext.hpp"
#include "stapik/locale/DateFormatter.hpp"
#include "stapik/locale/LocaleManager.hpp"
#include "stapik/storage/AppPaths.hpp"
#include "stapik/ui/dialog/DialogUtils.hpp"
#include "stapik/update/UpdateChecker.hpp"

MainWindow::MainWindow(const stapik::theme::ThemeRegistry& themes) :
    m_mainBox(Gtk::Orientation::VERTICAL, 0),
    m_controller(CalendarDocumentStore::createDefault(), createCalendarCloudHooks()),
    m_calendarView(m_controller),
    m_statusBar(Gtk::Orientation::HORIZONTAL, 0),
    m_actionHandler(*this, m_controller),
    m_checkForUpdates(stapik::app::AppContext::instance().settings(), stapik::update::CHECK_ENABLED_SETTING, true),
    m_menu(*this, StandardMenuOptions{ .tutorialDirectory = AppPaths::resourcesDir() / TUTORIAL_DIRECTORY, .themes = &themes, .undoStack = &m_controller.undoStack() })
{
    m_actionHandler.registerActions();
    initFileMenu();
    initSettings();
    initSaveErrors();
    initApiKeyWarning();
    init();
    initLayout();
    initSyncStatus();
}

MainWindow::~MainWindow()
{
    m_statusConnection.disconnect();
    m_saveFailedConnection.disconnect();
    m_apiKeyExpiringConnection.disconnect();
}

void MainWindow::init()
{
    set_title(WINDOW_TITLE);
    set_default_size(DEFAULT_WIDTH, DEFAULT_HEIGHT);
    set_child(m_mainBox);

    signal_close_request().connect(sigc::mem_fun(*this, &MainWindow::onCloseRequest), false);
}

void MainWindow::initLayout()
{
    m_statusBar.set_margin_start(STATUS_BAR_MARGIN);
    m_statusBar.set_margin_end(STATUS_BAR_MARGIN);
    m_statusBar.append(m_statusIndicator);

    m_mainBox.append(m_menu.menuBar());
    m_mainBox.append(m_calendarView);
    m_mainBox.append(m_statusBar);
}

void MainWindow::initFileMenu()
{
    m_menu.addToMenu(StandardMenu::Target::File, [](Gio::Menu& menu)
    {
        const auto backupSection = Gio::Menu::create();
        backupSection->append(LocaleManager::instance().translate("menu.file.restoreBackup"), "win.restoreBackup");
        menu.append_section(backupSection);
    });
}

void MainWindow::initSettings()
{
    // The check runs once, at startup, so a change takes effect on the next start.
    m_menu.settings().addBool("menu.settings.checkForUpdates", m_checkForUpdates);
}

void MainWindow::initSaveErrors()
{
    m_saveFailedConnection = m_controller.signalSaveFailed().connect([this](const std::string& reason)
    {
        const auto& loc = LocaleManager::instance();
        showMessageDialog(*this, loc.translate("save.failed.title"), loc.translate("save.failed.text", { { "reason", reason } }));
    });
}

void MainWindow::initApiKeyWarning()
{
    m_apiKeyExpiringConnection = m_controller.signalApiKeyExpiring().connect([this](const std::chrono::system_clock::time_point expiresAt)
    {
        const auto& loc = LocaleManager::instance();
        const auto date = stapik::locale::formatDate(stapik::locale::toLocalTime(expiresAt), loc.languageCode());
        showMessageDialog(*this, loc.translate("cloud.apiKeyExpiring.title"), loc.translate("cloud.apiKeyExpiring", { { "date", date } }));
    });
}

void MainWindow::initSyncStatus()
{
    m_statusIndicator.setStatus(m_controller.syncStatus());
    m_statusConnection = m_controller.signalSyncStatusChanged().connect([this](const stapik::sync::SyncStatus status)
    {
        m_statusIndicator.setStatus(status);
        refreshStatusBar();
    });

    refreshStatusBar();
}

void MainWindow::refreshStatusBar()
{
    m_statusBar.set_visible(m_controller.isConnected());
}

bool MainWindow::onCloseRequest()
{
    m_controller.flush();
    return false;
}

#include "MainWindow.hpp"

#include "../../application/CalendarCloudHooks.hpp"

#include "stapik/app/AppContext.hpp"
#include "stapik/locale/DateFormatter.hpp"
#include "stapik/locale/LocaleManager.hpp"
#include "stapik/storage/AppPaths.hpp"
#include "stapik/ui/dialog/DialogUtils.hpp"
#include "stapik/update/UpdateChecker.hpp"

#include <gdk/gdk.h>
#include <gtkmm/button.h>
#include <gtkmm/editable.h>

#include <string>

namespace
{
    struct ProblemTexts
    {
        const char* title;
        const char* text;
    };

    ProblemTexts problemTexts(const SyncProblem problem)
    {
        switch (problem)
        {
            case SyncProblem::AuthRejected: return { "sync.problem.title", "sync.problem.authRejected" };
            case SyncProblem::DecryptionFailed: return { "sync.problem.title", "sync.problem.decryptionFailed" };
            case SyncProblem::DocumentTooLarge: return { "sync.problem.title", "sync.problem.documentTooLarge" };
            case SyncProblem::LocalChangeDiscarded:
            default: return { "sync.discarded.title", "sync.discarded.text" };
        }
    }
}

MainWindow::MainWindow(const stapik::theme::ThemeRegistry& themes) :
    m_mainBox(Gtk::Orientation::VERTICAL, 0),
    m_controller(CalendarDocumentStore::createDefault(), createCalendarCloudHooks()),
    m_showWeekNumbers(stapik::app::AppContext::instance().settings(), SHOW_WEEK_NUMBERS_SETTING, true),
    m_calendarView(m_controller, m_showWeekNumbers),
    m_statusBar(Gtk::Orientation::HORIZONTAL, 0),
    m_actionHandler(*this, m_controller, [this] { m_checkForUpdates.reload(); }),
    m_checkForUpdates(stapik::app::AppContext::instance().settings(), stapik::update::CHECK_ENABLED_SETTING, true),
    m_menu(*this, StandardMenuOptions{ .tutorialDirectory = AppPaths::resourcesDir() / TUTORIAL_DIRECTORY, .themes = &themes, .undoStack = &m_controller.undoStack() })
{
    m_actionHandler.registerActions();
    initFileMenu();
    initSettings();
    initSaveErrors();
    initApiKeyWarning();
    initSyncProblems();
    initKeyringWarning();
    init();
    initLayout();
    initSyncStatus();
}

MainWindow::~MainWindow()
{
    m_statusConnection.disconnect();
    m_connectionChangedConnection.disconnect();
    m_saveFailedConnection.disconnect();
    m_apiKeyExpiringConnection.disconnect();
    m_syncProblemConnection.disconnect();
    m_mapConnection.disconnect();
}

void MainWindow::init()
{
    set_title(WINDOW_TITLE);
    set_default_size(DEFAULT_WIDTH, DEFAULT_HEIGHT);
    set_child(m_mainBox);

    signal_close_request().connect(sigc::mem_fun(*this, &MainWindow::onCloseRequest), false);

    auto keyController = Gtk::EventControllerKey::create();
    keyController->set_propagation_phase(Gtk::PropagationPhase::CAPTURE);
    keyController->signal_key_pressed().connect(
        [this](const guint keyval, const guint, const Gdk::ModifierType state) { return onKeyPressed(keyval, state); }, false);
    add_controller(keyController);
}

bool MainWindow::onKeyPressed(const guint keyval, const Gdk::ModifierType state)
{
    const auto* focused = get_focus();

    if (dynamic_cast<const Gtk::Editable*>(focused) != nullptr)
        return false;

    const bool activationKey = keyval == GDK_KEY_Return || keyval == GDK_KEY_KP_Enter;
    if (activationKey && dynamic_cast<const Gtk::Button*>(focused) != nullptr)
        return false;

    return m_calendarView.handleKeyPress(keyval, state);
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
        const auto disconnectSection = Gio::Menu::create();
        disconnectSection->append(LocaleManager::instance().translate("menu.file.disconnect"), "win.disconnect");
        menu.append_section(disconnectSection);

        const auto backupSection = Gio::Menu::create();
        backupSection->append(LocaleManager::instance().translate("menu.file.restoreBackup"), "win.restoreBackup");
        menu.append_section(backupSection);

        const auto archiveSection = Gio::Menu::create();
        archiveSection->append(LocaleManager::instance().translate("menu.file.exportArchive"), "win.exportArchive");
        archiveSection->append(LocaleManager::instance().translate("menu.file.importArchive"), "win.importArchive");
        menu.append_section(archiveSection);
    });
}

void MainWindow::initSettings()
{
    // The check runs once, at startup, so a change takes effect on the next start.
    m_menu.settings().addBool("menu.settings.checkForUpdates", m_checkForUpdates);
    m_menu.settings().addBool("menu.settings.showWeekNumbers", m_showWeekNumbers);
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

void MainWindow::initSyncProblems()
{
    m_syncProblemConnection = m_controller.signalSyncProblem().connect([this](const SyncProblemReport& report)
    {
        const auto& loc = LocaleManager::instance();
        const auto texts = problemTexts(report.problem);
        showMessageDialog(*this, loc.translate(texts.title), loc.translate(texts.text, { { "year", std::to_string(report.year) } }));
    });
}

void MainWindow::initKeyringWarning()
{
    m_mapConnection = signal_map().connect([this]
    {
        m_mapConnection.disconnect();

        if (!m_controller.cloudSecretsUnreadable())
            return;

        const auto& loc = LocaleManager::instance();
        showMessageDialog(*this, loc.translate("cloud.keyring.title"), loc.translate("cloud.keyring.text"));
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
    m_connectionChangedConnection = m_controller.signalConnectionChanged().connect(sigc::mem_fun(*this, &MainWindow::refreshStatusBar));

    refreshStatusBar();
}

void MainWindow::refreshStatusBar()
{
    m_statusBar.set_visible(m_controller.isConnected());
}

bool MainWindow::onCloseRequest()
{
    m_controller.flushAndWait();
    return false;
}

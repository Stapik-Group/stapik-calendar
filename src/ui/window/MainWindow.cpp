#include "MainWindow.hpp"

#include "../../application/CalendarCloudHooks.hpp"

MainWindow::MainWindow(const stapik::theme::ThemeRegistry& themes) :
    m_mainBox(Gtk::Orientation::VERTICAL, 0),
    m_controller(CalendarDocumentStore::createDefault(), createCalendarCloudHooks()),
    m_calendarView(m_controller),
    m_statusBar(Gtk::Orientation::HORIZONTAL, 0),
    m_actionHandler(*this, m_controller),
    m_menu(*this, StandardMenuOptions{ .themes = &themes, .undoStack = &m_controller.undoStack() })
{
    m_actionHandler.registerActions();
    init();
    initLayout();
    initSyncStatus();
}

MainWindow::~MainWindow()
{
    m_statusConnection.disconnect();
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

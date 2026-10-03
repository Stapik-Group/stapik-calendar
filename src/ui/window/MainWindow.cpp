#include "MainWindow.hpp"

#include "stapik/app/AppContext.hpp"
#include "stapik/storage/CloudStorageConfigStorage.hpp"

MainWindow::MainWindow(const stapik::theme::ThemeRegistry& themes) :
    m_mainBox(Gtk::Orientation::VERTICAL, 0),
    m_actionHandler(*this, m_calendarView.getCalendarGrid()),
    m_menu(*this, StandardMenuOptions{ .themes = &themes, .undoStack = &m_calendarView.getCalendarGrid().undoStack() })
{
    m_actionHandler.registerActions();
    init();
    initLayout();
    initCloud();
}

void MainWindow::init()
{
    set_title(WINDOW_TITLE);
    set_default_size(DEFAULT_WIDTH, DEFAULT_HEIGHT);
    set_child(m_mainBox);
}

void MainWindow::initLayout()
{
    m_mainBox.append(m_menu.menuBar());
    m_mainBox.append(m_calendarView);
}

void MainWindow::initCloud()
{
    const auto config = CloudStorageConfigStorage::load(stapik::app::AppContext::instance().info().internalName);
    if (!config.has_value())
        return;

    m_calendarView.getCalendarGrid().setCloudClient(
        std::make_unique<CloudStorageClient>(config.value(), "calendar.json"));
}

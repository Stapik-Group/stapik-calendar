#include "MainWindow.hpp"

#include "../../infrastructure/network/CloudStorageConfigStorage.hpp"

MainWindow::MainWindow() :
    m_mainBox(Gtk::Orientation::VERTICAL, 0),
    m_mainMenu(*this, m_calendarView.getCalendarGrid())
{
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
    m_mainBox.append(m_mainMenu.getMenuBar());
    m_mainBox.append(m_calendarView);
}

void MainWindow::initCloud()
{
    const auto config = CloudStorageConfigStorage::load();
    if (!config.has_value())
        return;

    m_calendarView.getCalendarGrid().setCloudClient(
        std::make_unique<CloudStorageClient>(config.value()));
}

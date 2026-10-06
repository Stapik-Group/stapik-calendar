#pragma once

#include <gtkmm/applicationwindow.h>
#include <gtkmm/box.h>
#include <sigc++/connection.h>

#include "stapik/theme/ThemeRegistry.hpp"
#include "stapik/ui/menu/StandardMenu.hpp"
#include "stapik/ui/widget/StatusIndicator.hpp"

#include "../../application/CalendarController.hpp"
#include "../action/MenuActionHandler.hpp"
#include "../view/CalendarView.hpp"

class MainWindow : public Gtk::ApplicationWindow
{
public:
    explicit MainWindow(const stapik::theme::ThemeRegistry& themes);
    ~MainWindow() override;
private:
    static constexpr int DEFAULT_WIDTH = 1280;
    static constexpr int DEFAULT_HEIGHT = 800;
    static constexpr int STATUS_BAR_MARGIN = 6;
    static constexpr auto WINDOW_TITLE = "Stapik Calendar";

    Gtk::Box m_mainBox;
    CalendarController m_controller;
    CalendarView m_calendarView;
    Gtk::Box m_statusBar;
    StatusIndicator m_statusIndicator;
    MenuActionHandler m_actionHandler;
    StandardMenu m_menu;
    sigc::connection m_statusConnection;

    void init();
    void initLayout();
    void initHelpMenu();
    void initSyncStatus();
    void refreshStatusBar();
    bool onCloseRequest();
};

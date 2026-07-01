#pragma once

#include <gtkmm/applicationwindow.h>
#include <gtkmm/box.h>

#include "../widget/MainMenu.hpp"
#include "../view/CalendarView.hpp"

class MainWindow : public Gtk::ApplicationWindow
{
public:
    explicit MainWindow();
    ~MainWindow() override = default;
    void initCloud();
private:
    static constexpr int DEFAULT_WIDTH = 1280;
    static constexpr int DEFAULT_HEIGHT = 800;
    static constexpr auto WINDOW_TITLE = "Stapik Calendar";

    Gtk::Box m_mainBox;
    CalendarView m_calendarView;
    MainMenu m_mainMenu;

    void init();
    void initLayout();
};

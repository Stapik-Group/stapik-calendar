#pragma once

#include <gtkmm/applicationwindow.h>
#include <gtkmm/popovermenubar.h>
#include <giomm/menu.h>
#include "MenuActionHandler.hpp"

class MainMenu
{
public:
    explicit MainMenu(Gtk::ApplicationWindow& window, CalendarGrid& calendarGrid);
    ~MainMenu() = default;
    Gtk::PopoverMenuBar& getMenuBar();

private:
    Gtk::ApplicationWindow& m_window;
    MenuActionHandler m_actionHandler;
    Glib::RefPtr<Gio::Menu> m_menuModel;
    Gtk::PopoverMenuBar m_menuBar;

    void buildModel();
    void initLanguageAction() const;
};
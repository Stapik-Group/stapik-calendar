#pragma once
#include <gtkmm/applicationwindow.h>

#include "../view/CalendarGrid.hpp"

class MenuActionHandler
{
public:
    explicit MenuActionHandler(Gtk::ApplicationWindow& window, CalendarGrid& calendarGrid);
    ~MenuActionHandler() = default;

    void registerActions();
private:
    Gtk::ApplicationWindow& m_window;
    CalendarGrid& m_calendarGrid;

    void onActionConnect() const;
    void onActionQuit() const;
    void onActionUndo() const;
    void onActionRedo() const;
    void onActionAbout() const;
};

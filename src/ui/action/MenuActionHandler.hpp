#pragma once
#include <gtkmm/applicationwindow.h>
#include <sigc++/connection.h>

#include "../../application/CalendarController.hpp"

class MenuActionHandler
{
public:
    explicit MenuActionHandler(Gtk::ApplicationWindow& window, CalendarController& controller);
    ~MenuActionHandler();

    MenuActionHandler(const MenuActionHandler&) = delete;
    MenuActionHandler& operator=(const MenuActionHandler&) = delete;

    void registerActions();
private:
    Gtk::ApplicationWindow& m_window;
    CalendarController& m_controller;
    sigc::connection m_connectionResult;

    void onActionConnect() const;
    void onActionQuit() const;
    void onActionSync() const;
    void onConnectionResult(const ConnectionResult& result) const;
};

#pragma once
#include <giomm/simpleaction.h>
#include <gtkmm/applicationwindow.h>
#include <sigc++/connection.h>

#include <filesystem>
#include <functional>

#include "../../application/CalendarController.hpp"

class MenuActionHandler
{
public:
    // `onSettingsRestored` is called after the settings were replaced by the ones from an archive,
    // so that the window can reload the settings it keeps by itself.
    MenuActionHandler(Gtk::ApplicationWindow& window, CalendarController& controller, std::function<void()> onSettingsRestored);
    ~MenuActionHandler();

    MenuActionHandler(const MenuActionHandler&) = delete;
    MenuActionHandler& operator=(const MenuActionHandler&) = delete;

    void registerActions();
private:
    Gtk::ApplicationWindow& m_window;
    CalendarController& m_controller;
    std::function<void()> m_onSettingsRestored;
    sigc::connection m_connectionResult;
    sigc::connection m_connectionChanged;
    Glib::RefPtr<Gio::SimpleAction> m_disconnectAction;

    void onActionConnect() const;
    void onActionDisconnect() const;
    void onActionQuit() const;
    void onActionSync() const;
    void onActionRestoreBackup() const;
    void onActionExportArchive() const;
    void onActionImportArchive() const;
    void exportArchiveTo(const std::filesystem::path& target) const;
    void importArchiveFrom(const std::filesystem::path& source) const;
    void onConnectionResult(const ConnectionResult& result) const;
    void updateDisconnectAvailability() const;
};

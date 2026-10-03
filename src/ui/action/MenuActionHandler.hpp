#pragma once
#include <gtkmm/applicationwindow.h>

#include <string>

#include "../view/CalendarGrid.hpp"

class MenuActionHandler
{
public:
    explicit MenuActionHandler(Gtk::ApplicationWindow& window, CalendarGrid& calendarGrid);
    ~MenuActionHandler() = default;

    void registerActions();
private:
    static constexpr auto CALENDAR_FILENAME = "calendar.json";

    Gtk::ApplicationWindow& m_window;
    CalendarGrid& m_calendarGrid;

    void onActionConnect() const;
    void onActionQuit() const;
    void onActionSync() const;

    [[nodiscard]] static std::string appName();
    void handleConnectResult(const CloudStorageConfig& config) const;
    void applyCloudConfig(const CloudStorageConfig& config) const;
};
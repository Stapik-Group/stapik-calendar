#pragma once

#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <sigc++/connection.h>
#include <sigc++/signal.h>
#include <chrono>
#include "CalendarDatePicker.hpp"

class CalendarNavBar : public Gtk::Box
{
public:
    explicit CalendarNavBar();
    ~CalendarNavBar() override;
    void updateDisplay(std::chrono::year_month_day date);
    sigc::signal<void()>& signalPrevYear();
    sigc::signal<void()>& signalPrevMonth();
    sigc::signal<void()>& signalNextMonth();
    sigc::signal<void()>& signalNextYear();
    sigc::signal<void()>& signalToday();
    // The user picked a month in the calendar popup.
    sigc::signal<void(std::chrono::year_month)>& signalMonthSelected();
    sigc::signal<void(std::chrono::year_month_day)>& signalDaySelected();
private:
    static constexpr int BOX_SPACING = 4;
    static constexpr int BOX_MARGIN = 4;
    static constexpr int TODAY_BUTTON_MARGIN = 12;

    Gtk::Button m_btnPrevYear;
    Gtk::Button m_btnPrevMonth;
    Gtk::Button m_btnNextMonth;
    Gtk::Button m_btnNextYear;
    Gtk::Button m_btnToday;
    CalendarDatePicker m_datePicker;

    sigc::signal<void()> m_signalPrevYear;
    sigc::signal<void()> m_signalPrevMonth;
    sigc::signal<void()> m_signalNextMonth;
    sigc::signal<void()> m_signalNextYear;
    sigc::signal<void()> m_signalToday;
    sigc::connection m_localeConnection;
    sigc::signal<void(std::chrono::year_month)> m_signalMonthSelected;
    sigc::signal<void(std::chrono::year_month_day)> m_signalDaySelected;

    void initLayout();
    void initButtons();
    void refreshTodayLabel();
};
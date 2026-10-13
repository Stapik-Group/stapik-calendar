#pragma once

#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <sigc++/signal.h>
#include <chrono>
#include "stapik/ui/widget/MonthPicker.hpp"

class CalendarNavBar : public Gtk::Box
{
public:
    explicit CalendarNavBar();
    void updateDisplay(std::chrono::year_month yearMonth);
    sigc::signal<void()>& signalPrevYear();
    sigc::signal<void()>& signalPrevMonth();
    sigc::signal<void()>& signalNextMonth();
    sigc::signal<void()>& signalNextYear();
    // The user picked a month in the calendar popup.
    sigc::signal<void(std::chrono::year_month)>& signalMonthSelected();
private:
    static constexpr int BOX_SPACING = 4;
    static constexpr int BOX_MARGIN = 4;

    Gtk::Button m_btnPrevYear;
    Gtk::Button m_btnPrevMonth;
    Gtk::Button m_btnNextMonth;
    Gtk::Button m_btnNextYear;
    MonthPicker m_monthPicker;
    bool m_updating = false;

    sigc::signal<void()> m_signalPrevYear;
    sigc::signal<void()> m_signalPrevMonth;
    sigc::signal<void()> m_signalNextMonth;
    sigc::signal<void()> m_signalNextYear;
    sigc::signal<void(std::chrono::year_month)> m_signalMonthSelected;

    void initLayout();
    void initButtons();
    void onMonthPicked();
};
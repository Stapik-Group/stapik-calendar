#pragma once

#include <gtkmm/box.h>
#include <chrono>

#include "CalendarNavBar.hpp"
#include "CalendarHeader.hpp"
#include "CalendarGrid.hpp"
#include "../../core/util/WeekRules.hpp"

class CalendarView : public Gtk::Box
{
public:
    explicit CalendarView(CalendarController& controller);
private:
    CalendarController& m_controller;
    std::chrono::year_month m_currentYearMonth;
    WeekRules m_weekRules;

    CalendarNavBar m_navBar;
    CalendarHeader m_header;
    CalendarGrid m_grid;

    void initLayout();
    void initNavigation();

    void navigatePrevYear();
    void navigatePrevMonth();
    void navigateNextMonth();
    void navigateNextYear();
    void navigateTo(std::chrono::year_month yearMonth);

    void refreshView();
};
#pragma once

#include <gtkmm/box.h>
#include <chrono>

#include "CalendarNavBar.hpp"
#include "CalendarHeader.hpp"
#include "CalendarGrid.hpp"

class CalendarView : public Gtk::Box
{
public:
    explicit CalendarView();
    CalendarGrid& getCalendarGrid();
private:
    std::chrono::year_month m_currentYearMonth;

    CalendarNavBar m_navBar;
    CalendarHeader m_header;
    CalendarGrid m_grid;

    void initLayout();
    void initNavigation();

    void navigatePrevYear();
    void navigatePrevMonth();
    void navigateNextMonth();
    void navigateNextYear();

    void refreshView();

    static std::chrono::year_month currentYearMonth();
};
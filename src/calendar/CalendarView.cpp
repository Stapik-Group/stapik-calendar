#include "CalendarView.hpp"

CalendarView::CalendarView() : Box(Gtk::Orientation::VERTICAL, 0), m_currentYearMonth(currentYearMonth())
{
    initLayout();
    initNavigation();
    refreshView();
}

void CalendarView::initLayout()
{
    set_hexpand(true);
    set_vexpand(true);

    append(m_navBar);
    append(m_header);
    append(m_grid);
}

void CalendarView::initNavigation()
{
    m_navBar.signalPrevYear().connect(sigc::mem_fun(*this, &CalendarView::navigatePrevYear));
    m_navBar.signalPrevMonth().connect(sigc::mem_fun(*this, &CalendarView::navigatePrevMonth));
    m_navBar.signalNextMonth().connect(sigc::mem_fun(*this, &CalendarView::navigateNextMonth));
    m_navBar.signalNextYear().connect( sigc::mem_fun(*this, &CalendarView::navigateNextYear));
}

void CalendarView::navigatePrevYear()
{
    m_currentYearMonth = (m_currentYearMonth.year() - std::chrono::years{1}) / m_currentYearMonth.month();
    refreshView();
}

void CalendarView::navigatePrevMonth()
{
    m_currentYearMonth = m_currentYearMonth - std::chrono::months{1};
    refreshView();
}

void CalendarView::navigateNextMonth()
{
    m_currentYearMonth = m_currentYearMonth + std::chrono::months{1};
    refreshView();
}

void CalendarView::navigateNextYear()
{
    m_currentYearMonth = (m_currentYearMonth.year() + std::chrono::years{1}) / m_currentYearMonth.month();
    refreshView();
}

void CalendarView::refreshView()
{
    m_navBar.updateDisplay(m_currentYearMonth);
    m_grid.displayMonth(m_currentYearMonth);
}

std::chrono::year_month CalendarView::currentYearMonth()
{
    using namespace std::chrono;
    const auto today = year_month_day{floor<days>(system_clock::now())};
    return year_month{today.year(), today.month()};
}

CalendarGrid& CalendarView::getCalendarGrid()
{
    return m_grid;
}
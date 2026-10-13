#include "CalendarView.hpp"

#include "../../core/util/DateUtils.hpp"

CalendarView::CalendarView(CalendarController& controller) :
    Box(Gtk::Orientation::VERTICAL, 0),
    m_controller(controller),
    m_currentYearMonth(DateUtils::todayYearMonth()),
    m_grid(controller)
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
    m_navBar.signalMonthSelected().connect(sigc::mem_fun(*this, &CalendarView::navigateTo));
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

void CalendarView::navigateTo(const std::chrono::year_month yearMonth)
{
    m_currentYearMonth = yearMonth;
    refreshView();
}

void CalendarView::refreshView()
{
    m_controller.setDisplayedYear(static_cast<int>(m_currentYearMonth.year()));
    m_navBar.updateDisplay(m_currentYearMonth);
    m_grid.displayMonth(m_currentYearMonth);
}

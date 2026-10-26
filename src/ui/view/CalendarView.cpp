#include "CalendarView.hpp"

#include "../../core/util/DateUtils.hpp"
#include "../../infrastructure/system/RegionalSettings.hpp"

#include "stapik/locale/LocaleManager.hpp"

CalendarView::CalendarView(CalendarController& controller, stapik::settings::ObservableSetting<bool>& showWeekNumbers) :
    Box(Gtk::Orientation::VERTICAL, 0),
    m_controller(controller),
    m_currentYearMonth(DateUtils::todayYearMonth()),
    m_weekRules(RegionalSettings::firstDayOfWeek()),
    m_showWeekNumbers(showWeekNumbers),
    m_headerRow(Gtk::Orientation::HORIZONTAL, 0),
    m_header(m_weekRules),
    m_bodyRow(Gtk::Orientation::HORIZONTAL, 0),
    m_grid(controller, m_weekRules)
{
    initLayout();
    initNavigation();
    initWeekNumbers();
    refreshView();
}

CalendarView::~CalendarView()
{
    m_showWeekNumbersConnection.disconnect();
    m_localeConnection.disconnect();
}

void CalendarView::initLayout()
{
    set_hexpand(true);
    set_vexpand(true);

    m_weekNumberTitle.set_size_request(CalendarWeekNumbers::COLUMN_WIDTH, -1);
    m_weekNumberTitle.add_css_class("calendar-week-number");

    m_headerRow.append(m_weekNumberTitle);
    m_headerRow.append(m_header);
    m_bodyRow.append(m_weekNumbers);
    m_bodyRow.append(m_grid);

    append(m_navBar);
    append(m_headerRow);
    append(m_bodyRow);
}

void CalendarView::initWeekNumbers()
{
    refreshWeekNumberTitle();
    applyWeekNumberVisibility();

    m_localeConnection = LocaleManager::instance().signalLocaleChanged().connect(sigc::mem_fun(*this, &CalendarView::refreshWeekNumberTitle));
    m_showWeekNumbersConnection = m_showWeekNumbers.signalChanged().connect([this](const bool) { applyWeekNumberVisibility(); });
}

void CalendarView::refreshWeekNumberTitle()
{
    m_weekNumberTitle.set_text(LocaleManager::instance().translate("calendar.weekNumber.title"));
}

void CalendarView::applyWeekNumberVisibility()
{
    const bool visible = m_showWeekNumbers.get();
    m_weekNumberTitle.set_visible(visible);
    m_weekNumbers.set_visible(visible);
}

std::vector<int> CalendarView::weekNumbersOfMonth(const std::chrono::year_month yearMonth) const
{
    using namespace std::chrono;

    const sys_days firstWeekStart{m_weekRules.startOfWeek(yearMonth / 1)};

    std::vector<int> numbers;
    for (int week = 0; week < CalendarWeekNumbers::MAX_WEEKS; ++week)
        numbers.push_back(m_weekRules.weekNumber(year_month_day{firstWeekStart + days{week * WeekRules::DAYS_IN_WEEK}}));

    return numbers;
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
    m_weekNumbers.setWeekNumbers(weekNumbersOfMonth(m_currentYearMonth));
    m_grid.displayMonth(m_currentYearMonth);
}

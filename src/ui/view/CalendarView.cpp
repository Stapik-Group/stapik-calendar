#include "CalendarView.hpp"

#include "../../core/util/DateUtils.hpp"
#include "../../infrastructure/system/RegionalSettings.hpp"

#include "stapik/locale/LocaleManager.hpp"

#include <gdk/gdk.h>

CalendarView::CalendarView(CalendarController& controller, stapik::settings::ObservableSetting<bool>& showWeekNumbers) :
    Box(Gtk::Orientation::VERTICAL, 0),
    m_controller(controller),
    m_selectedDate(DateUtils::today()),
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
    const bool visible = m_showWeekNumbers.get() && m_viewMode != ViewMode::Day;
    m_weekNumberTitle.set_visible(visible);
    m_weekNumbers.set_visible(visible);
}

std::vector<int> CalendarView::weekNumbersOfView() const
{
    using namespace std::chrono;

    switch (m_viewMode)
    {
        using enum ViewMode;
        case Month: return weekNumbersOfMonth(year_month{m_selectedDate.year(), m_selectedDate.month()});
        case Week:  return { m_weekRules.weekNumber(m_weekRules.startOfWeek(m_selectedDate)) };
        case Day:   return {};
    }

    return {};
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
    m_navBar.signalPrevMonth().connect(sigc::mem_fun(*this, &CalendarView::navigatePrevPeriod));
    m_navBar.signalNextMonth().connect(sigc::mem_fun(*this, &CalendarView::navigateNextPeriod));
    m_navBar.signalNextYear().connect( sigc::mem_fun(*this, &CalendarView::navigateNextYear));
    m_navBar.signalToday().connect(sigc::mem_fun(*this, &CalendarView::navigateToToday));
    m_navBar.signalMonthSelected().connect(sigc::mem_fun(*this, &CalendarView::navigateTo));
    m_navBar.signalDaySelected().connect(sigc::mem_fun(*this, &CalendarView::onDaySelected));
    m_grid.signalDaySelected().connect(sigc::mem_fun(*this, &CalendarView::onDaySelected));
}

bool CalendarView::handleKeyPress(const guint keyval, const Gdk::ModifierType state)
{
    if ((state & Gdk::ModifierType::CONTROL_MASK) == Gdk::ModifierType::CONTROL_MASK)
    {
        switch (gdk_keyval_to_lower(keyval))
        {
            using enum ViewMode;
            case GDK_KEY_Page_Up:
            case GDK_KEY_KP_Page_Up:   markDate(DateUtils::addYears(m_selectedDate, -1)); return true;
            case GDK_KEY_Page_Down:
            case GDK_KEY_KP_Page_Down: markDate(DateUtils::addYears(m_selectedDate, 1));  return true;
            case GDK_KEY_t:         markDate(DateUtils::today());                      return true;
            case GDK_KEY_1:         setViewMode(Month);                      return true;
            case GDK_KEY_2:         setViewMode(Week);                       return true;
            case GDK_KEY_3:         setViewMode(Day);                        return true;
            case GDK_KEY_n:         m_grid.addEntryOn(m_selectedDate);                 return true;
            default:                return false;
        }
    }

    switch (keyval)
    {
    case GDK_KEY_Left:
    case GDK_KEY_KP_Left:   markDate(DateUtils::addDays(m_selectedDate, -1));                         return true;
    case GDK_KEY_Right:
    case GDK_KEY_KP_Right:  markDate(DateUtils::addDays(m_selectedDate, 1));                          return true;
    case GDK_KEY_Up:
    case GDK_KEY_KP_Up:     markDate(DateUtils::addDays(m_selectedDate, -WeekRules::DAYS_IN_WEEK));   return true;
    case GDK_KEY_Down:
    case GDK_KEY_KP_Down:   markDate(DateUtils::addDays(m_selectedDate, WeekRules::DAYS_IN_WEEK));    return true;
    case GDK_KEY_Page_Up:
    case GDK_KEY_KP_Page_Up:   markDate(stepped(-1));                                                    return true;
    case GDK_KEY_Page_Down:
    case GDK_KEY_KP_Page_Down: markDate(stepped(1));                                                     return true;
    case GDK_KEY_Home:
    case GDK_KEY_KP_Home:   markDate(DateUtils::today());                                             return true;
    case GDK_KEY_Return:
    case GDK_KEY_KP_Enter:  m_grid.addEntryOn(m_selectedDate);                                        return true;
    default:                return false;
    }
}

void CalendarView::navigatePrevYear()
{
    selectDate(DateUtils::addYears(m_selectedDate, -1));
}

void CalendarView::navigatePrevPeriod()
{
    selectDate(stepped(-1));
}

void CalendarView::navigateNextPeriod()
{
    selectDate(stepped(1));
}

std::chrono::year_month_day CalendarView::stepped(const int direction) const
{
    switch (m_viewMode)
    {
        using enum ViewMode;
        case Week: return DateUtils::addDays(m_selectedDate, direction * WeekRules::DAYS_IN_WEEK);
        case Day:  return DateUtils::addDays(m_selectedDate, direction);
        case Month:
            break;
    }

    return DateUtils::addMonths(m_selectedDate, direction);
}

void CalendarView::setViewMode(const ViewMode viewMode)
{
    if (viewMode == m_viewMode)
        return;

    m_viewMode = viewMode;
    applyWeekNumberVisibility();
    refreshView();
    m_signalViewModeChanged.emit(viewMode);
}

void CalendarView::showToday()
{
    navigateToToday();
}

sigc::signal<void(ViewMode)>& CalendarView::signalViewModeChanged()
{
    return m_signalViewModeChanged;
}

void CalendarView::navigateNextYear()
{
    selectDate(DateUtils::addYears(m_selectedDate, 1));
}

void CalendarView::navigateTo(const std::chrono::year_month yearMonth)
{
    selectDate(DateUtils::inMonth(m_selectedDate, yearMonth));
}

void CalendarView::navigateToToday()
{
    selectDate(DateUtils::today());
}

void CalendarView::selectDate(const std::chrono::year_month_day date)
{
    m_selectedDate = date;
    m_hasMarkedDay = false;
    refreshView();
}

void CalendarView::markDate(const std::chrono::year_month_day date)
{
    m_selectedDate = date;
    m_hasMarkedDay = true;
    refreshView();
}

void CalendarView::onDaySelected(const std::chrono::year_month_day date)
{
    markDate(date);
}

void CalendarView::refreshView()
{
    m_controller.setDisplayedYear(static_cast<int>(m_selectedDate.year()));
    m_navBar.updateDisplay(m_selectedDate);
    m_weekNumbers.setWeekNumbers(weekNumbersOfView());
    m_header.display(m_viewMode, m_selectedDate);
    m_grid.display(m_viewMode, m_selectedDate, m_hasMarkedDay ? std::optional{m_selectedDate} : std::nullopt);
}

#include "CalendarDatePicker.hpp"

#include "stapik/domain/YearMonth.hpp"
#include "stapik/locale/LocaleManager.hpp"

namespace
{
    bool isShownYear(const std::chrono::year_month_day date)
    {
        const int year = static_cast<int>(date.year());
        return year >= stapik::domain::MIN_YEAR_MONTH_YEAR && year <= stapik::domain::MAX_YEAR_MONTH_YEAR;
    }
}

CalendarDatePicker::CalendarDatePicker(const std::chrono::year_month_day date) :
    m_date(date)
{
    showDate(m_date);
    CalendarDatePicker::refreshLabel();
}

void CalendarDatePicker::setDate(const std::chrono::year_month_day date)
{
    if (!isShownYear(date))
        return;

    m_date = date;
    showDate(m_date);
    refreshLabel();
}

sigc::signal<void(std::chrono::year_month)>& CalendarDatePicker::signalMonthChanged()
{
    return m_signalMonthChanged;
}

sigc::signal<void(std::chrono::year_month_day)>& CalendarDatePicker::signalDayPicked()
{
    return m_signalDayPicked;
}

void CalendarDatePicker::onDaySelected()
{
    const auto picked = selectedDate();
    const bool sameMonth = picked.year() == m_date.year() && picked.month() == m_date.month();

    m_date = picked;
    refreshLabel();

    if (!sameMonth)
    {
        m_signalMonthChanged.emit(std::chrono::year_month{picked.year(), picked.month()});
        return;
    }

    closePopover();
    m_signalDayPicked.emit(picked);
}

void CalendarDatePicker::refreshLabel()
{
    set_label(stapik::domain::YearMonth::fromDate(m_date).label(LocaleManager::instance()));
}

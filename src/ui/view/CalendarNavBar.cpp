#include "CalendarNavBar.hpp"
#include <array>

namespace
{
    constexpr std::array<const char*, 12> MONTH_KEYS = {
        "month.january", "month.february", "month.march", "month.april",
        "month.may", "month.june", "month.july", "month.august",
        "month.september", "month.october", "month.november", "month.december"
    };
}

CalendarNavBar::CalendarNavBar() : Box(Gtk::Orientation::HORIZONTAL, 4)
{
    initButtons();
    initLayout();

    LocaleManager::instance().signalLocaleChanged().connect([this] { updateDisplay(m_lastDisplayedMonth); });
}

void CalendarNavBar::initButtons()
{
    m_btnPrevYear.set_label("◀◀");
    m_btnPrevMonth.set_label("◀");
    m_btnNextMonth.set_label("▶");
    m_btnNextYear.set_label("▶▶");

    m_btnPrevYear.signal_clicked().connect([this] { m_signalPrevYear.emit(); });
    m_btnPrevMonth.signal_clicked().connect([this] { m_signalPrevMonth.emit(); });
    m_btnNextMonth.signal_clicked().connect([this] { m_signalNextMonth.emit(); });
    m_btnNextYear.signal_clicked().connect([this] { m_signalNextYear.emit(); });
}

void CalendarNavBar::initLayout()
{
    add_css_class("calendar-navbar");
    set_halign(Gtk::Align::CENTER);
    set_margin(4);

    append(m_btnPrevYear);
    append(m_btnPrevMonth);
    append(m_currentLabel);
    append(m_btnNextMonth);
    append(m_btnNextYear);

    m_currentLabel.set_width_chars(20);
    m_currentLabel.set_halign(Gtk::Align::CENTER);
}

void CalendarNavBar::updateDisplay(const std::chrono::year_month yearMonth)
{
    m_lastDisplayedMonth = yearMonth;
    m_currentLabel.set_text(formatYearMonth(yearMonth));
}

std::string CalendarNavBar::formatYearMonth(const std::chrono::year_month yearMonth)
{
    const auto monthIndex = static_cast<unsigned>(yearMonth.month()) - 1;
    const auto year = static_cast<int>(yearMonth.year());

    return LocaleManager::instance().translate(MONTH_KEYS[monthIndex]) + " " + std::to_string(year);
}

sigc::signal<void()>& CalendarNavBar::signalPrevYear()  { return m_signalPrevYear;  }
sigc::signal<void()>& CalendarNavBar::signalPrevMonth() { return m_signalPrevMonth; }
sigc::signal<void()>& CalendarNavBar::signalNextMonth() { return m_signalNextMonth; }
sigc::signal<void()>& CalendarNavBar::signalNextYear()  { return m_signalNextYear;  }
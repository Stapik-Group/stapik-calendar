#include "CalendarNavBar.hpp"
#include "../../core/util/DateUtils.hpp"

#include "stapik/locale/LocaleManager.hpp"

CalendarNavBar::CalendarNavBar() :
    Box(Gtk::Orientation::HORIZONTAL, BOX_SPACING),
    m_datePicker(DateUtils::today())
{
    initButtons();
    initLayout();
}

CalendarNavBar::~CalendarNavBar()
{
    m_localeConnection.disconnect();
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
    m_btnToday.signal_clicked().connect([this] { m_signalToday.emit(); });

    refreshTodayLabel();
    m_localeConnection = LocaleManager::instance().signalLocaleChanged().connect(sigc::mem_fun(*this, &CalendarNavBar::refreshTodayLabel));

    m_datePicker.signalMonthChanged().connect([this](const std::chrono::year_month yearMonth) { m_signalMonthSelected.emit(yearMonth); });
    m_datePicker.signalDayPicked().connect([this](const std::chrono::year_month_day date) { m_signalDaySelected.emit(date); });
}

void CalendarNavBar::initLayout()
{
    add_css_class("calendar-navbar");
    set_halign(Gtk::Align::CENTER);
    set_margin(BOX_MARGIN);

    append(m_btnPrevYear);
    append(m_btnPrevMonth);
    append(m_datePicker);
    append(m_btnNextMonth);
    append(m_btnNextYear);

    m_btnToday.set_margin_start(TODAY_BUTTON_MARGIN);
    append(m_btnToday);
}

void CalendarNavBar::refreshTodayLabel()
{
    m_btnToday.set_label(LocaleManager::instance().translate("nav.today"));
}

void CalendarNavBar::updateDisplay(const std::chrono::year_month_day date)
{
    m_datePicker.setDate(date);
}

sigc::signal<void()>& CalendarNavBar::signalPrevYear()  { return m_signalPrevYear;  }
sigc::signal<void()>& CalendarNavBar::signalPrevMonth() { return m_signalPrevMonth; }
sigc::signal<void()>& CalendarNavBar::signalNextMonth() { return m_signalNextMonth; }
sigc::signal<void()>& CalendarNavBar::signalNextYear()  { return m_signalNextYear;  }
sigc::signal<void()>& CalendarNavBar::signalToday()     { return m_signalToday;     }
sigc::signal<void(std::chrono::year_month)>& CalendarNavBar::signalMonthSelected() { return m_signalMonthSelected; }
sigc::signal<void(std::chrono::year_month_day)>& CalendarNavBar::signalDaySelected() { return m_signalDaySelected; }

#pragma once

#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/label.h>
#include <sigc++/signal.h>
#include <chrono>

class CalendarNavBar : public Gtk::Box
{
public:
    explicit CalendarNavBar();
    void updateDisplay(std::chrono::year_month yearMonth);
    sigc::signal<void()>& signalPrevYear();
    sigc::signal<void()>& signalPrevMonth();
    sigc::signal<void()>& signalNextMonth();
    sigc::signal<void()>& signalNextYear();
private:
    static constexpr int BOX_SPACING = 4;
    static constexpr int BOX_MARGIN = 4;
    static constexpr int LABEL_WIDTH_CHARS = 20;

    Gtk::Button m_btnPrevYear;
    Gtk::Button m_btnPrevMonth;
    Gtk::Button m_btnNextMonth;
    Gtk::Button m_btnNextYear;
    Gtk::Label  m_currentLabel;

    std::chrono::year_month m_lastDisplayedMonth{};

    sigc::signal<void()> m_signalPrevYear;
    sigc::signal<void()> m_signalPrevMonth;
    sigc::signal<void()> m_signalNextMonth;
    sigc::signal<void()> m_signalNextYear;

    void initLayout();
    void initButtons();

    static std::string formatYearMonth(std::chrono::year_month yearMonth);
};
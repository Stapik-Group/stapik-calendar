#pragma once

#include <gdkmm/enums.h>
#include <gtkmm/box.h>
#include <gtkmm/label.h>
#include <sigc++/connection.h>

#include <chrono>
#include <vector>

#include "stapik/settings/ObservableSetting.hpp"

#include "CalendarNavBar.hpp"
#include "CalendarHeader.hpp"
#include "CalendarGrid.hpp"
#include "CalendarWeekNumbers.hpp"
#include "ViewMode.hpp"
#include "../../core/util/WeekRules.hpp"

class CalendarView : public Gtk::Box
{
public:
    CalendarView(CalendarController& controller, stapik::settings::ObservableSetting<bool>& showWeekNumbers);
    ~CalendarView() override;

    bool handleKeyPress(guint keyval, Gdk::ModifierType state);
    void setViewMode(ViewMode viewMode);
    void showToday();
    sigc::signal<void(ViewMode)>& signalViewModeChanged();
private:
    CalendarController& m_controller;
    std::chrono::year_month_day m_selectedDate;
    bool m_hasMarkedDay = false;
    ViewMode m_viewMode = ViewMode::Month;
    WeekRules m_weekRules;

    stapik::settings::ObservableSetting<bool>& m_showWeekNumbers;

    CalendarNavBar m_navBar;
    Box m_headerRow;
    Gtk::Label m_weekNumberTitle;
    CalendarHeader m_header;
    Box m_bodyRow;
    CalendarWeekNumbers m_weekNumbers;
    CalendarGrid m_grid;
    sigc::connection m_showWeekNumbersConnection;
    sigc::connection m_localeConnection;
    sigc::signal<void(ViewMode)> m_signalViewModeChanged;

    void initLayout();
    void initNavigation();
    void initWeekNumbers();
    void refreshWeekNumberTitle();
    void applyWeekNumberVisibility();
    [[nodiscard]] std::vector<int> weekNumbersOfView() const;
    [[nodiscard]] std::vector<int> weekNumbersOfMonth(std::chrono::year_month yearMonth) const;

    void navigatePrevYear();
    void navigatePrevPeriod();
    void navigateNextPeriod();
    void navigateNextYear();
    void navigateTo(std::chrono::year_month yearMonth);
    void navigateToToday();
    [[nodiscard]] std::chrono::year_month_day stepped(int direction) const;
    void selectDate(std::chrono::year_month_day date);
    void markDate(std::chrono::year_month_day date);
    void onDaySelected(std::chrono::year_month_day date);

    void refreshView();
};
#pragma once

#include <gtkmm/grid.h>
#include <gtkmm/label.h>
#include <gtkmm/box.h>
#include <array>
#include <chrono>

#include "ViewMode.hpp"
#include "../../core/util/WeekRules.hpp"

class CalendarHeader : public Gtk::Grid
{
public:
    explicit CalendarHeader(const WeekRules& weekRules);

    void display(ViewMode viewMode, std::chrono::year_month_day selectedDate);

private:
    static constexpr int COLUMNS = 7;
    static constexpr int LABEL_MARGIN = 8;
    static constexpr std::array<const char*, COLUMNS> DAY_KEYS = {
        "day.sun", "day.mon", "day.tue", "day.wed", "day.thu", "day.fri", "day.sat"
    };

    const WeekRules& m_weekRules;
    ViewMode m_viewMode = ViewMode::Month;
    std::chrono::year_month_day m_selectedDate {};
    std::array<Gtk::Label, COLUMNS> m_labels;
    std::array<Gtk::Box, COLUMNS>   m_cells;

    void initLayout();
    void layoutCells();
    void refreshLabels();
};
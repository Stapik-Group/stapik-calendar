#pragma once

#include <gtkmm/grid.h>
#include <gtkmm/label.h>
#include <array>
#include <gtkmm/box.h>
#include "../locale/LocaleManager.hpp"

class CalendarHeader : public Gtk::Grid
{
public:
    explicit CalendarHeader();

private:
    static constexpr int COLUMNS = 7;
    static constexpr std::array<const char*, COLUMNS> DAY_KEYS = {
        "day.mon", "day.tue", "day.wed", "day.thu", "day.fri", "day.sat", "day.sun"
    };

    std::array<Gtk::Label, COLUMNS> m_labels;
    std::array<Gtk::Box, COLUMNS>   m_cells;

    void initLayout();
    void refreshLabels();
};
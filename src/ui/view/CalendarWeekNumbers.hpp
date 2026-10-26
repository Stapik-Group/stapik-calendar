#pragma once

#include <gtkmm/box.h>
#include <gtkmm/label.h>

#include <array>
#include <vector>

class CalendarWeekNumbers : public Gtk::Box
{
public:
    static constexpr int MAX_WEEKS = 6;
    static constexpr int COLUMN_WIDTH = 44;

    CalendarWeekNumbers();

    void setWeekNumbers(const std::vector<int>& numbers);

private:
    std::array<Gtk::Label, MAX_WEEKS> m_labels;
};

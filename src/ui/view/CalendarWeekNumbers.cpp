#include "CalendarWeekNumbers.hpp"

#include <string>

CalendarWeekNumbers::CalendarWeekNumbers() :
    Box(Gtk::Orientation::VERTICAL, 0)
{
    set_homogeneous(true);
    set_vexpand(true);
    set_size_request(COLUMN_WIDTH, -1);

    for (auto& label : m_labels)
    {
        label.set_halign(Gtk::Align::CENTER);
        label.set_valign(Gtk::Align::CENTER);
        label.set_vexpand(true);
        label.add_css_class("calendar-week-number");
        append(label);
    }
}

void CalendarWeekNumbers::setWeekNumbers(const std::vector<int>& numbers)
{
    for (std::size_t i = 0; i < m_labels.size(); ++i)
    {
        const bool shown = i < numbers.size();
        m_labels[i].set_visible(shown);

        if (shown)
            m_labels[i].set_text(std::to_string(numbers[i]));
    }
}

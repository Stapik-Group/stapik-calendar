#pragma once
#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/label.h>

#include "../data/CalendarEntry.hpp"

class CalendarEntryWidget : public Gtk::Box
{
public:
    explicit CalendarEntryWidget(const CalendarEntry& entry);
    sigc::signal<void()>& signalEditRequested();
    sigc::signal<void()>& signalDeleteRequested();
private:
    Gtk::Label m_nameLabel;
    Gtk::Button m_deleteButton;

    sigc::signal<void()> m_signalEditRequested;
    sigc::signal<void()> m_signalDeleteRequested;

    void initLayout(const CalendarEntry& entry);
    void initGesture();
};

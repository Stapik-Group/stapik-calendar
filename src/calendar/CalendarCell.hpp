#pragma once

#include <gtkmm/frame.h>
#include <gtkmm/label.h>
#include <gtkmm/box.h>
#include <gtkmm/gestureclick.h>
#include <gtkmm/scrolledwindow.h>

#include "data/CalendarEntry.hpp"

class CalendarCell : public Gtk::Frame {
public:
    explicit CalendarCell();
    void setDay(int day);
    void clearDay();
    void markAsToday(bool isToday);
    void setEntries(const std::vector<CalendarEntry>& entries);

    sigc::signal<void()>& signalDoubleClicked();
    sigc::signal<void()>& signalRightClicked();
    sigc::signal<void(int)>& signalEditRequested();
    sigc::signal<void(int)>& signalDeleteRequested();
private:
    Gtk::Box m_outerBox;
    Gtk::Label m_dayLabel;
    Gtk::ScrolledWindow m_scrolledWindow;
    Gtk::Box m_entriesBox;

    Glib::RefPtr<Gtk::GestureClick> m_gestureClick;
    Glib::RefPtr<Gtk::GestureClick> m_gestureRightClick;
    sigc::signal<void()> m_signalDoubleClicked;
    sigc::signal<void()> m_signalRightClicked;
    sigc::signal<void(int)> m_signalEditRequested;
    sigc::signal<void(int)> m_signalDeleteRequested;

    void initLayout();
    void initGesture();
    void refreshEntries(const std::vector<CalendarEntry>& entries);
};

#include "CalendarEntryWidget.hpp"

#include <gtkmm/gestureclick.h>

CalendarEntryWidget::CalendarEntryWidget(const CalendarEntry &entry) :
    Box(Gtk::Orientation::HORIZONTAL, 4)
{
    initLayout(entry);
    initGesture();
}

void CalendarEntryWidget::initLayout(const CalendarEntry &entry)
{
    m_nameLabel.set_text(entry.name);
    m_nameLabel.set_halign(Gtk::Align::START);
    m_nameLabel.set_ellipsize(Pango::EllipsizeMode::END);
    m_nameLabel.set_hexpand(true);
    m_nameLabel.add_css_class("calendar-entry-label");

    m_deleteButton.set_label("✕");
    m_deleteButton.set_has_frame(false);
    m_deleteButton.add_css_class("calendar-entry-delete");
    m_deleteButton.signal_clicked().connect([this] { m_signalDeleteRequested.emit(); });

    append(m_nameLabel);
    append(m_deleteButton);
}

void CalendarEntryWidget::initGesture()
{
    const auto gesture = Gtk::GestureClick::create();
    gesture->set_button(1);
    gesture->signal_pressed().connect(
        [this](const int nPress, double, double)
        {
            if (nPress == 1)
                m_signalEditRequested.emit();
        });

    m_nameLabel.add_controller(gesture);
}

sigc::signal<void()>& CalendarEntryWidget::signalEditRequested()
{
    return m_signalEditRequested;
}

sigc::signal<void()>& CalendarEntryWidget::signalDeleteRequested()
{
    return m_signalDeleteRequested;
}

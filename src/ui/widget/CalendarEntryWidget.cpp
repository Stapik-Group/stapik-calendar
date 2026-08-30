#include "CalendarEntryWidget.hpp"

#include <gtkmm/gestureclick.h>

#include "../../core/util/EntryColorUtils.hpp"

CalendarEntryWidget::CalendarEntryWidget(const CalendarEntry &entry) :
    Box(Gtk::Orientation::HORIZONTAL, 4)
{
    initLayout(entry);
    initGesture();
    initColorPopover(entry.color);
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

    if (const auto cssClass = EntryColorUtils::cssClass(entry.color); !cssClass.empty())
        add_css_class(cssClass);

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

void CalendarEntryWidget::initColorPopover(const EntryColor currentColor)
{
    m_colorPicker.setSelectedColor(currentColor);
    m_colorPicker.signalColorSelected().connect([this](const EntryColor color)
    {
        m_colorPopover.popdown();
        m_signalColorChangeRequested.emit(color);
    });
    m_colorPopover.set_child(m_colorPicker);
    m_colorPopover.set_parent(*this);
    m_colorPopover.set_has_arrow(true);

    const auto rightClickGesture = Gtk::GestureClick::create();
    rightClickGesture->set_button(3);
    rightClickGesture->set_propagation_phase(Gtk::PropagationPhase::CAPTURE);
    rightClickGesture->signal_pressed().connect(
        [this, rightClickGesture](int, double, double)
        {
            rightClickGesture->set_state(Gtk::EventSequenceState::CLAIMED);
            m_colorPopover.popup();
        });
    add_controller(rightClickGesture);
}

sigc::signal<void()>& CalendarEntryWidget::signalEditRequested()
{
    return m_signalEditRequested;
}

sigc::signal<void()>& CalendarEntryWidget::signalDeleteRequested()
{
    return m_signalDeleteRequested;
}

sigc::signal<void(EntryColor)>& CalendarEntryWidget::signalColorChangeRequested()
{
    return m_signalColorChangeRequested;
}
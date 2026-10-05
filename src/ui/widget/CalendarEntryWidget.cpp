#include "CalendarEntryWidget.hpp"

#include <gtkmm/gestureclick.h>
#include <gtkmm/settings.h>
#include <gdkmm/contentprovider.h>
#include <glibmm/main.h>

#include "EntryColorStyle.hpp"
#include "../../core/util/EntryDragPayload.hpp"

namespace
{
    constexpr unsigned FALLBACK_DOUBLE_CLICK_MS = 400;

    unsigned doubleClickTimeMs()
    {
        const auto settings = Gtk::Settings::get_default();
        if (!settings)
            return FALLBACK_DOUBLE_CLICK_MS;

        return static_cast<unsigned>(settings->property_gtk_double_click_time().get_value());
    }
}

CalendarEntryWidget::CalendarEntryWidget(const CalendarEntry &entry) :
    Box(Gtk::Orientation::HORIZONTAL, 4)
{
    initLayout(entry);
    initGesture();
    initColorPopover(entry.color);
    initDragSource();
}

CalendarEntryWidget::~CalendarEntryWidget()
{
    m_pendingClick.disconnect();
}

void CalendarEntryWidget::initLayout(const CalendarEntry &entry)
{
    m_nameLabel.set_text(entry.name);
    m_nameLabel.set_halign(Gtk::Align::START);
    m_nameLabel.set_ellipsize(Pango::EllipsizeMode::END);
    m_nameLabel.set_hexpand(true);
    m_nameLabel.add_css_class("calendar-entry-label");
    if (!entry.link.empty())
        m_nameLabel.set_cursor("pointer");

    m_deleteButton.set_label("✕");
    m_deleteButton.set_has_frame(false);
    m_deleteButton.add_css_class("calendar-entry-delete");
    m_deleteButton.signal_clicked().connect([this] { m_signalDeleteRequested.emit(); });

    add_css_class("calendar-entry");
    add_css_class(entryColorCssClass(entry.color));

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
            if (nPress >= DOUBLE_CLICK_COUNT)
                m_pendingClick.disconnect();
        });
    gesture->signal_released().connect(
        [this](const int nPress, double, double)
        {
            if (nPress == SINGLE_CLICK_COUNT)
                scheduleSingleClick();
            else if (nPress == DOUBLE_CLICK_COUNT)
                m_signalEditRequested.emit();
        });

    m_nameLabel.add_controller(gesture);
}

void CalendarEntryWidget::scheduleSingleClick()
{
    m_pendingClick.disconnect();
    m_pendingClick = Glib::signal_timeout().connect(
        [this]
        {
            m_signalClicked.emit();
            return false;
        },
        doubleClickTimeMs());
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

void CalendarEntryWidget::initDragSource()
{
    m_dragSource = Gtk::DragSource::create();
    m_dragSource->set_actions(Gdk::DragAction::MOVE | Gdk::DragAction::COPY);
    m_dragSource->signal_prepare().connect(
        [this](double, double) -> Glib::RefPtr<Gdk::ContentProvider>
        {
            const bool isCopy = (m_dragSource->get_current_event_state() & Gdk::ModifierType::CONTROL_MASK) == Gdk::ModifierType::CONTROL_MASK;
            Glib::Value<Glib::ustring> value;
            value.init(Glib::Value<Glib::ustring>::value_type());
            value.set(EntryDragPayload::serialize(m_cellIndex, m_entryIndex, isCopy));
            return Gdk::ContentProvider::create(value);
        }, false);
    add_controller(m_dragSource);
}

void CalendarEntryWidget::setSourceLocator(const int cellIndex, const int entryIndex)
{
    m_cellIndex = cellIndex;
    m_entryIndex = entryIndex;
}

sigc::signal<void()>& CalendarEntryWidget::signalClicked()
{
    return m_signalClicked;
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
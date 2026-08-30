#include "CalendarCell.hpp"

#include "../../core/util/EntryDragPayload.hpp"
#include "../widget/CalendarEntryWidget.hpp"

CalendarCell::CalendarCell() :
    m_outerBox(Gtk::Orientation::VERTICAL, 0),
    m_entriesBox(Gtk::Orientation::VERTICAL, ENTRIES_SPACING)
{
    initLayout();
    initGesture();
    initDropTarget();
}

void CalendarCell::initLayout()
{
    m_dayLabel.set_halign(Gtk::Align::END);
    m_dayLabel.set_valign(Gtk::Align::START);
    m_dayLabel.set_margin(CELL_MARGIN);

    m_scrolledWindow.set_child(m_entriesBox);
    m_scrolledWindow.set_policy(Gtk::PolicyType::NEVER, Gtk::PolicyType::AUTOMATIC);
    m_scrolledWindow.set_vexpand(true);
    m_scrolledWindow.set_overlay_scrolling(false);

    m_entriesBox.set_margin_start(CELL_MARGIN);
    m_entriesBox.set_margin_end(CELL_MARGIN);
    m_entriesBox.set_margin_bottom(CELL_MARGIN);

    m_outerBox.append(m_dayLabel);
    m_outerBox.append(m_scrolledWindow);

    set_child(m_outerBox);
}

void CalendarCell::initGesture()
{
    m_gestureClick = Gtk::GestureClick::create();
    m_gestureClick->set_button(LEFT_MOUSE_BUTTON);
    m_gestureClick->signal_pressed().connect(
        [this](const int nPress, double, double) {
            if (nPress == DOUBLE_CLICK_COUNT) m_signalDoubleClicked.emit();
        });

    add_controller(m_gestureClick);

    m_gestureRightClick = Gtk::GestureClick::create();
    m_gestureRightClick->set_button(RIGHT_MOUSE_BUTTON);
    m_gestureRightClick->signal_pressed().connect(
        [this](int, double, double)
        {
            m_signalRightClicked.emit();
        });

    add_controller(m_gestureRightClick);
}

void CalendarCell::initDropTarget()
{
    m_dropTarget = Gtk::DropTarget::create(Glib::Value<Glib::ustring>::value_type(), Gdk::DragAction::MOVE | Gdk::DragAction::COPY);
    m_dropTarget->signal_drop().connect(sigc::mem_fun(*this, &CalendarCell::onDrop), false);
    m_dropTarget->signal_enter().connect(
        [this](double, double) { add_css_class("drag-over"); return Gdk::DragAction::MOVE; }, false);
    m_dropTarget->signal_leave().connect([this] { remove_css_class("drag-over"); });
    add_controller(m_dropTarget);
}

void CalendarCell::setDay(const int day)
{
    m_dayLabel.set_text(std::to_string(day));
}

void CalendarCell::setCellIndex(const int cellIndex)
{
    m_cellIndex = cellIndex;
}

void CalendarCell::clearDay()
{
    m_dayLabel.set_text("");
    refreshEntries({});
}

void CalendarCell::markAsToday(const bool isToday)
{
    if (isToday)
        add_css_class("today");
    else
        remove_css_class("today");
}

void CalendarCell::setEntries(const std::vector<CalendarEntry>& entries)
{
    refreshEntries(entries);
}

void CalendarCell::refreshEntries(const std::vector<CalendarEntry>& entries)
{
    while (auto* child = m_entriesBox.get_first_child())
        m_entriesBox.remove(*child);

    for (int i = 0; i < static_cast<int>(entries.size()); ++i)
    {
        auto* widget = Gtk::make_managed<CalendarEntryWidget>(entries[i]);

        widget->signalEditRequested().connect([this, i] { m_signalEditRequested.emit(i); });
        widget->signalDeleteRequested().connect([this, i] { m_signalDeleteRequested.emit(i); });
        widget->signalColorChangeRequested().connect([this, i](const EntryColor color) { m_signalColorChangeRequested.emit(i, color); });
        widget->setSourceLocator(m_cellIndex, i);

        m_entriesBox.append(*widget);
    }
}

bool CalendarCell::onDrop(const Glib::ValueBase& value, double, double)
{
    remove_css_class("drag-over");

    if (G_VALUE_TYPE(value.gobj()) != G_TYPE_STRING)
        return false;

    const auto& stringValue = static_cast<const Glib::Value<Glib::ustring>&>(value);
    const auto parsed = EntryDragPayload::deserialize(stringValue.get());
    if (!parsed.has_value())
        return false;

    const auto [sourceCellIndex, sourceEntryIndex, isCopy] = parsed.value();
    m_signalEntryMoveRequested.emit(sourceCellIndex, sourceEntryIndex, isCopy);
    return true;
}

sigc::signal<void()>& CalendarCell::signalDoubleClicked()
{
    return m_signalDoubleClicked;
}

sigc::signal<void(int)>& CalendarCell::signalEditRequested()
{
    return m_signalEditRequested;
}

sigc::signal<void(int)>& CalendarCell::signalDeleteRequested()
{
    return m_signalDeleteRequested;
}

sigc::signal<void()> &CalendarCell::signalRightClicked()
{
    return m_signalRightClicked;
}

sigc::signal<void(int, EntryColor)>& CalendarCell::signalColorChangeRequested()
{
    return m_signalColorChangeRequested;
}

sigc::signal<void(int, int, bool)>& CalendarCell::signalEntryMoveRequested()
{
    return m_signalEntryMoveRequested;
}
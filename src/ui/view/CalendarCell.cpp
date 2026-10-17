#include "CalendarCell.hpp"

#include "../../core/util/EntryDragPayload.hpp"
#include "../widget/CalendarEntryWidget.hpp"

#include <glibmm/main.h>
#include <gtkmm/adjustment.h>

#include <algorithm>
#include <numeric>
#include <string>

namespace
{
    void showOverflowCount(Gtk::Label& indicator, const std::string& arrow, const int count)
    {
        indicator.set_visible(count > 0);

        if (count > 0)
            indicator.set_text(arrow + " " + std::to_string(count));
    }
}

CalendarCell::CalendarCell() :
    m_outerBox(Gtk::Orientation::VERTICAL, 0),
    m_entriesBox(Gtk::Orientation::VERTICAL, ENTRIES_SPACING)
{
    initLayout();
    initGesture();
    initDropTarget();
}

CalendarCell::~CalendarCell()
{
    m_overflowUpdate.disconnect();
}

void CalendarCell::initLayout()
{
    m_dayLabel.set_halign(Gtk::Align::END);
    m_dayLabel.set_valign(Gtk::Align::START);
    m_dayLabel.set_margin(CELL_MARGIN);
    m_dayLabel.add_css_class("calendar-day-number");

    m_scrolledWindow.set_child(m_entriesBox);
    // EXTERNAL keeps scrolling (wheel, touchpad, touch) but never shows a scrollbar, in any theme.
    m_scrolledWindow.set_policy(Gtk::PolicyType::NEVER, Gtk::PolicyType::EXTERNAL);
    m_scrolledWindow.set_vexpand(true);

    m_entriesBox.set_margin_start(CELL_MARGIN);
    m_entriesBox.set_margin_end(CELL_MARGIN);
    m_entriesBox.set_margin_bottom(CELL_MARGIN);

    initOverflowIndicators();

    m_outerBox.append(m_dayLabel);
    m_outerBox.append(m_overlay);

    set_child(m_outerBox);
}

void CalendarCell::initOverflowIndicators()
{
    // The indicators float over the entries instead of taking space from them, so showing or hiding one never
    // changes the width or the position of an entry. They ignore the mouse, so clicks and drops reach the entries.
    m_overlay.set_vexpand(true);
    m_overlay.set_child(m_scrolledWindow);

    for (auto* indicator : { &m_moreAbove, &m_moreBelow })
    {
        indicator->set_halign(Gtk::Align::END);
        indicator->set_margin(OVERFLOW_INDICATOR_MARGIN);
        indicator->set_can_target(false);
        indicator->set_visible(false);
        indicator->add_css_class("calendar-overflow-indicator");
        m_overlay.add_overlay(*indicator);
    }

    m_moreAbove.set_valign(Gtk::Align::START);
    m_moreBelow.set_valign(Gtk::Align::END);

    const auto adjustment = m_scrolledWindow.get_vadjustment();
    adjustment->signal_value_changed().connect(sigc::mem_fun(*this, &CalendarCell::updateOverflowIndicators));
    adjustment->signal_changed().connect(sigc::mem_fun(*this, &CalendarCell::scheduleOverflowUpdate));
}

void CalendarCell::scheduleOverflowUpdate()
{
    // The size of the entries is known only after the layout, which comes after the change of the adjustment.
    if (m_overflowUpdate.connected())
        return;

    m_overflowUpdate = Glib::signal_idle().connect([this]
    {
        updateOverflowIndicators();
        return false;
    });
}

void CalendarCell::updateOverflowIndicators()
{
    const auto adjustment = m_scrolledWindow.get_vadjustment();
    const double visibleTop = adjustment->get_value();
    const double visibleBottom = visibleTop + adjustment->get_page_size();

    int hiddenAbove = 0;
    int hiddenBelow = 0;
    double entryTop = 0.0;

    for (auto* entry = m_entriesBox.get_first_child(); entry != nullptr; entry = entry->get_next_sibling())
    {
        const double entryBottom = entryTop + entry->get_height();

        if (entryTop < visibleTop - OVERFLOW_TOLERANCE)
            ++hiddenAbove;

        if (entryBottom > visibleBottom + OVERFLOW_TOLERANCE)
            ++hiddenBelow;

        entryTop = entryBottom + ENTRIES_SPACING;
    }

    showOverflowCount(m_moreAbove, "▲", hiddenAbove);
    showOverflowCount(m_moreBelow, "▼", hiddenBelow);
}

void CalendarCell::initGesture()
{
    m_gestureClick = Gtk::GestureClick::create();
    m_gestureClick->set_button(LEFT_MOUSE_BUTTON);
    m_gestureClick->signal_pressed().connect(
        [this](const int nPress, const double x, const double y) {
            if (nPress == DOUBLE_CLICK_COUNT && !isOverEntry(x, y)) m_signalDoubleClicked.emit();
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
    remove_css_class("other-month");
}

void CalendarCell::setAdjacentMonthDay(const int day)
{
    m_dayLabel.set_text(std::to_string(day));
    add_css_class("other-month");
    remove_css_class("today");
    refreshEntries({});
}

void CalendarCell::setWeekend(const bool isWeekend)
{
    if (isWeekend)
        add_css_class("weekend");
    else
        remove_css_class("weekend");
}

void CalendarCell::setCellIndex(const int cellIndex)
{
    m_cellIndex = cellIndex;
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

bool CalendarCell::isOverEntry(const double x, const double y)
{
    for (auto* widget = pick(x, y); widget != nullptr && widget != this; widget = widget->get_parent())
    {
        if (dynamic_cast<CalendarEntryWidget*>(widget) != nullptr)
            return true;
    }

    return false;
}

void CalendarCell::refreshEntries(const std::vector<CalendarEntry>& entries)
{
    while (auto* child = m_entriesBox.get_first_child())
        m_entriesBox.remove(*child);

    // Entries with a time come first, in the order of the clock; the others follow in the order of adding.
    // Only the display order changes, the entry keeps its real index in the day (for editing, deleting, dragging).
    std::vector<int> displayOrder(entries.size());
    std::iota(displayOrder.begin(), displayOrder.end(), 0);
    std::ranges::stable_sort(displayOrder, [&entries](const int left, const int right)
    {
        const auto& leftTime = entries[left].time;
        const auto& rightTime = entries[right].time;

        if (leftTime.empty() != rightTime.empty())
            return !leftTime.empty();

        return leftTime < rightTime;
    });

    for (const int i : displayOrder)
    {
        auto* widget = Gtk::make_managed<CalendarEntryWidget>(entries[i]);

        widget->signalLinkClicked().connect([this, i] { m_signalEntryLinkClicked.emit(i); });
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

sigc::signal<void(int)>& CalendarCell::signalEntryLinkClicked()
{
    return m_signalEntryLinkClicked;
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
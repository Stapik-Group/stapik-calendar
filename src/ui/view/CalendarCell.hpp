#pragma once

#include <gtkmm/frame.h>
#include <gtkmm/label.h>
#include <gtkmm/box.h>
#include <gtkmm/gestureclick.h>
#include <gtkmm/overlay.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/droptarget.h>
#include <glibmm/value.h>
#include <sigc++/connection.h>

#include "../../core/model/CalendarEntry.hpp"
#include "../../core/model/EntryColor.hpp"

class CalendarCell : public Gtk::Frame {
public:
    explicit CalendarCell();
    ~CalendarCell() override;
    void setDay(int day);
    void setAdjacentMonthDay(int day);
    void setWeekend(bool isWeekend);
    void markAsToday(bool isToday);
    void markAsSelected(bool isSelected);
    void setEntries(const std::vector<CalendarEntry>& entries);
    void setCellIndex(int cellIndex);

    sigc::signal<void()>& signalClicked();
    sigc::signal<void()>& signalDoubleClicked();
    sigc::signal<void()>& signalRightClicked();
    sigc::signal<void(const EntryId&)>& signalEntryLinkClicked();
    sigc::signal<void(const EntryId&)>& signalEditRequested();
    sigc::signal<void(const EntryId&)>& signalDeleteRequested();
    sigc::signal<void(const EntryId&, EntryColor)>& signalColorChangeRequested();
    sigc::signal<void(int, const EntryId&, bool)>& signalEntryMoveRequested();
private:
    static constexpr int ENTRIES_SPACING = 2;
    static constexpr int CELL_MARGIN = 4;
    static constexpr guint LEFT_MOUSE_BUTTON = 1;
    static constexpr guint RIGHT_MOUSE_BUTTON = 3;
    static constexpr int SINGLE_CLICK_COUNT = 1;
    static constexpr int DOUBLE_CLICK_COUNT = 2;
    static constexpr int OVERFLOW_INDICATOR_MARGIN = 2;
    // Entries cut off by less than this (in pixels) still count as fully visible.
    static constexpr double OVERFLOW_TOLERANCE = 1.0;

    int m_cellIndex = -1;

    Gtk::Box m_outerBox;
    Gtk::Label m_dayLabel;
    Gtk::Overlay m_overlay;
    Gtk::ScrolledWindow m_scrolledWindow;
    Gtk::Box m_entriesBox;
    Gtk::Label m_moreAbove;
    Gtk::Label m_moreBelow;
    sigc::connection m_overflowUpdate;

    Glib::RefPtr<Gtk::GestureClick> m_gestureClick;
    Glib::RefPtr<Gtk::GestureClick> m_gestureRightClick;
    sigc::signal<void()> m_signalClicked;
    sigc::signal<void()> m_signalDoubleClicked;
    sigc::signal<void()> m_signalRightClicked;
    sigc::signal<void(const EntryId&)> m_signalEntryLinkClicked;
    sigc::signal<void(const EntryId&)> m_signalEditRequested;
    sigc::signal<void(const EntryId&)> m_signalDeleteRequested;
    sigc::signal<void(const EntryId&, EntryColor)> m_signalColorChangeRequested;
    Glib::RefPtr<Gtk::DropTarget> m_dropTarget;
    sigc::signal<void(int, const EntryId&, bool)> m_signalEntryMoveRequested;

    void initLayout();
    void initOverflowIndicators();
    void scheduleOverflowUpdate();
    void updateOverflowIndicators();
    void initGesture();
    [[nodiscard]] bool isOverEntry(double x, double y);
    void refreshEntries(const std::vector<CalendarEntry>& entries);
    void initDropTarget();
    bool onDrop(const Glib::ValueBase& value, double x, double y);
};

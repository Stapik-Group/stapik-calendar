#pragma once
#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/label.h>
#include <gtkmm/popover.h>
#include <gtkmm/dragsource.h>

#include <string>

#include "EntryColorPickerWidget.hpp"
#include "../../core/model/CalendarEntry.hpp"
#include "../../core/model/EntryColor.hpp"

class CalendarEntryWidget : public Gtk::Box
{
public:
    explicit CalendarEntryWidget(const CalendarEntry& entry);
    ~CalendarEntryWidget() override;

    void setCellIndex(int cellIndex);
    sigc::signal<void()>& signalLinkClicked();
    sigc::signal<void()>& signalEditRequested();
    sigc::signal<void()>& signalDeleteRequested();
    sigc::signal<void(EntryColor)>& signalColorChangeRequested();
private:
    static constexpr int SINGLE_CLICK_COUNT = 1;
    // A plain text character on purpose: an emoji would be drawn with a separate color font and ignore the theme colors.
    static constexpr auto LINK_SYMBOL = "↗";

    int m_cellIndex = -1;
    EntryId m_entryId;

    Box m_textBox;
    Box m_titleRow;
    Gtk::Label m_timeLabel;
    Gtk::Label m_nameLabel;
    Gtk::Label m_locationLabel;
    Gtk::Label m_linkLabel;
    Gtk::Button m_deleteButton;
    Gtk::Popover m_colorPopover;
    EntryColorPickerWidget m_colorPicker;
    Glib::RefPtr<Gtk::DragSource> m_dragSource;

    sigc::signal<void()> m_signalLinkClicked;
    sigc::signal<void()> m_signalEditRequested;
    sigc::signal<void()> m_signalDeleteRequested;
    sigc::signal<void(EntryColor)> m_signalColorChangeRequested;

    void initLayout(const CalendarEntry& entry);
    void initGesture();
    void initLinkSymbol(const std::string& link);
    void initColorPopover(EntryColor currentColor);
    void initDragSource();
};

#pragma once
#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/label.h>
#include <gtkmm/popover.h>

#include "EntryColorPickerWidget.hpp"
#include "../../core/model/CalendarEntry.hpp"
#include "../../core/model/EntryColor.hpp"

class CalendarEntryWidget : public Gtk::Box
{
public:
    explicit CalendarEntryWidget(const CalendarEntry& entry);
    sigc::signal<void()>& signalEditRequested();
    sigc::signal<void()>& signalDeleteRequested();
    sigc::signal<void(EntryColor)>& signalColorChangeRequested();
private:
    Gtk::Label m_nameLabel;
    Gtk::Button m_deleteButton;
    Gtk::Popover m_colorPopover;
    EntryColorPickerWidget m_colorPicker;

    sigc::signal<void()> m_signalEditRequested;
    sigc::signal<void()> m_signalDeleteRequested;
    sigc::signal<void(EntryColor)> m_signalColorChangeRequested;

    void initLayout(const CalendarEntry& entry);
    void initGesture();
    void initColorPopover(EntryColor currentColor);
};

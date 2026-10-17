#pragma once

#include <optional>
#include <string>
#include <gtkmm/box.h>
#include <gtkmm/dialog.h>
#include <gtkmm/entry.h>
#include <gtkmm/label.h>
#include <gtkmm/sizegroup.h>

#include "../../core/model/CalendarEntry.hpp"
#include "../widget/EntryColorPickerWidget.hpp"

class CalendarEntryDialog : public Gtk::Dialog
{
public:
    explicit CalendarEntryDialog(Window& parent);
    explicit CalendarEntryDialog(Window& parent, const CalendarEntry& existing);
    void setLink(const std::string& link);
    // Fills the name only while it is still empty, so what the user has typed in the meantime is never replaced.
    void suggestName(const std::string& name);
    [[nodiscard]] std::optional<CalendarEntry> getResult() const;
private:
    static constexpr int CONTENT_SPACING = 8;
    static constexpr int CONTENT_MARGIN = 16;
    static constexpr int DEFAULT_WIDTH = 400;
    static constexpr int TIME_MAX_LENGTH = 5;

    Gtk::Box m_contentBox;
    Gtk::Label m_nameLabel;
    Gtk::Entry m_nameEntry;
    Gtk::Label m_linkLabel;
    Gtk::Entry m_linkEntry;
    Gtk::Label m_timeLabel;
    Gtk::Entry m_timeEntry;
    Gtk::Label m_locationLabel;
    Gtk::Entry m_locationEntry;
    Gtk::Label m_colorLabel;
    EntryColorPickerWidget m_colorPicker;
    Glib::RefPtr<Gtk::SizeGroup> m_buttonSizeGroup;

    void initLayout();
    void onTimeChanged();
};

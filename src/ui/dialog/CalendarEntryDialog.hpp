#pragma once

#include <optional>
#include <gtkmm/box.h>
#include <gtkmm/dialog.h>
#include <gtkmm/entry.h>
#include <gtkmm/label.h>

#include "../../core/model/CalendarEntry.hpp"

class CalendarEntryDialog : public Gtk::Dialog
{
public:
    explicit CalendarEntryDialog(Window& parent);
    explicit CalendarEntryDialog(Window& parent, const CalendarEntry& existing);
    [[nodiscard]] std::optional<CalendarEntry> getResult() const;
private:
    static constexpr int CONTENT_SPACING = 8;
    static constexpr int CONTENT_MARGIN = 16;
    static constexpr int DEFAULT_WIDTH = 400;

    Gtk::Box m_contentBox;
    Gtk::Label m_nameLabel;
    Gtk::Entry m_nameEntry;
    Gtk::Label m_linkLabel;
    Gtk::Entry m_linkEntry;

    void initLayout();
};
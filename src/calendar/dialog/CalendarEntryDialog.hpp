#pragma once

#include <optional>
#include <gtkmm/dialog.h>
#include <gtkmm/entry.h>
#include <gtkmm/label.h>

#include "../data/CalendarEntry.hpp"
#include "../../locale/LocaleManager.hpp"

class CalendarEntryDialog : public Gtk::Dialog
{
public:
    explicit CalendarEntryDialog(Window& parent);
    explicit CalendarEntryDialog(Gtk::Window& parent, const CalendarEntry& existing);
    std::optional<CalendarEntry> getResult() const;

private:
    Gtk::Box   m_contentBox;
    Gtk::Label m_nameLabel;
    Gtk::Entry m_nameEntry;
    Gtk::Label m_linkLabel;
    Gtk::Entry m_linkEntry;

    void initLayout();
};